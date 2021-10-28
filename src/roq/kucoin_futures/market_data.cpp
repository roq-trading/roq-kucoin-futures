/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/market_data.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/tools/exception.h"

#include "roq/core/metrics/factory.h"

#include "roq/kucoin_futures/flags.h"

#include "roq/kucoin_futures/json/utils.h"

#include "roq/kucoin_futures/tools/splitter.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {

namespace {
static const auto NAME = "md"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

MarketData::MarketData(
    Handler &handler,
    core::io::Context &context,
    uint32_t stream_id,
    Shared &shared,
    const std::string_view &uri,
    const std::string_view &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      ping_frequency_(ping_frequency), connection_(
                                           *this,
                                           context,
                                           core::URI{uri},
                                           query,
                                           Flags::ws_ping_freq(),
                                           Flags::decode_buffer_size(),
                                           Flags::encode_buffer_size(),
                                           []() { return std::string(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      request_id_(static_cast<uint64_t>(stream_id_) * 1000000),  // scale (debugging)
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .welcome = create_metrics(name_, "welcome"_sv),
          .error = create_metrics(name_, "error"_sv),
          .pong = create_metrics(name_, "pong"_sv),
          .ack = create_metrics(name_, "ack"_sv),
          .ticker_v2 = create_metrics(name_, "ticker_v2"_sv),
          .ticker = create_metrics(name_, "ticker"_sv),
          .match = create_metrics(name_, "match"_sv),
          .mark_index_price = create_metrics(name_, "mark_index_price"_sv),
          .funding_rate = create_metrics(name_, "funding_rate"_sv),
          .level2 = create_metrics(name_, "level2"_sv),
          .funding_begin = create_metrics(name_, "funding_begin"_sv),
          .funding_end = create_metrics(name_, "funding_end"_sv),
          .snapshot_24h = create_metrics(name_, "snapshot_24h"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
      },
      shared_(shared), download_({}, [this](auto state) { return download(state); }) {
  if (ping_frequency_.count() == 0)
    log::fatal("Unexpected"_sv);
  log::info("ping_frequency={}"_sv, ping_frequency_);
}

void MarketData::operator()(const Event<Start> &) {
  connection_.start();
}

void MarketData::operator()(const Event<Stop> &) {
  connection_.stop();
}

void MarketData::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
  if (connection_.ready()) {
    if (welcome_) {
      if (next_ping_ < now)
        send_ping(now);
      check_subscribe_queue(now);
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."_sv);
    connection_.close();
  }
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.welcome, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.pong, metrics::PROFILE)
      .write(profile_.ack, metrics::PROFILE)
      .write(profile_.ticker_v2, metrics::PROFILE)
      .write(profile_.ticker, metrics::PROFILE)
      .write(profile_.match, metrics::PROFILE)
      .write(profile_.mark_index_price, metrics::PROFILE)
      .write(profile_.funding_rate, metrics::PROFILE)
      .write(profile_.level2, metrics::PROFILE)
      .write(profile_.funding_begin, metrics::PROFILE)
      .write(profile_.funding_end, metrics::PROFILE)
      .write(profile_.snapshot_24h, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void MarketData::update_subscriptions(std::vector<std::string> &symbols) {
  assert(&symbols != &symbols_);
  auto max_size = Flags::ws_max_subscriptions_per_stream();
  auto offset = symbols_.size();
  if (max_size <= offset)
    return;
  if (symbols.empty())
    return;
  symbols_.reserve(max_size);
  auto length = std::min(max_size - offset, symbols.size());
  assert(length > 0);
  for (size_t i = {}; i < length; ++i) {
    symbols_.emplace_back(symbols.back());
    symbols.pop_back();
  }
  assert(length == (symbols_.size() - offset));
  if (ready())
    subscribe({&symbols_[offset], length});
}

void MarketData::operator()(const core::web::Socket::Connected &) {
  assert(logon_timeout_.count() == 0);
  auto now = core::get_system_clock();
  logon_timeout_ = now + Flags::ws_request_timeout();
}

void MarketData::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
  // experimental
  shared_.mbp_collector.clear();  // XXX HANS this is SHARED !!!
}

void MarketData::operator()(const core::web::Socket::Ready &) {
  // note! wait for welcome
}

void MarketData::operator()(const core::web::Socket::Close &) {
}

void MarketData::operator()(const core::web::Socket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(const core::web::Socket::Binary &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t MarketData::download(MarketDataState state) {
  switch (state) {
    case MarketDataState::UNDEFINED:
      assert(false);
      break;
    case MarketDataState::SUBSCRIBE:
      subscribe(symbols_);
      return {};
    case MarketDataState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void MarketData::subscribe(const roq::span<std::string> &symbols) {
  subscribe("/contract/announcement"_sv);  // XXX HANS ???
  if (std::empty(symbols))
    return;
  if (Flags::ws_subscribe_ticker_v2())
    subscribe("/contractMarket/tickerV2"_sv, symbols);
  else
    subscribe("/contractMarket/ticker"_sv, symbols);
  subscribe("/contractMarket/execution"_sv, symbols);
  subscribe("/contract/instrument"_sv, symbols);
  subscribe("/contractMarket/level2"_sv, symbols);
}

void MarketData::subscribe(const std::string_view &topic) {
  auto now = core::get_system_clock();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"_sv,
      now.count(),
      topic);
  log::debug("message={}"_sv, message);
  subscribe_queue_.emplace_back(now, message);
}

void MarketData::subscribe(const std::string_view &topic, const roq::span<std::string> &symbols) {
  assert(!symbols.empty());
  for (auto &symbol : symbols) {
    auto now = core::get_system_clock();
    auto message = fmt::format(
        R"({{)"
        R"("id":"{}",)"
        R"("type":"subscribe",)"
        R"("topic":"{}:{}",)"
        R"("response":true)"
        R"(}})"_sv,
        now.count(),
        topic,
        symbol);
    log::debug("message={}"_sv, message);
    subscribe_queue_.emplace_back(now, message);
  }
}

void MarketData::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(R"({{"id":{},"type":"ping"}})"_sv, now.count());
  log::debug<1>(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"_sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MarketData::operator()(server::Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("event={{trace_info={}, welcome={}}}"_sv, trace_info, welcome);
    welcome_ = true;
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

void MarketData::operator()(server::Trace<json::Error> const &event) {
  profile_.error([&]() {
    // XXX HANS DEBUG
    auto &[trace_info, error] = event;
    log::warn("error={}"_sv, error);
    // log::fatal("event={{trace_info={}, error={}}}"_sv, trace_info, error);
  });
}

void MarketData::operator()(server::Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("event={{trace_info={}, pong={}}}"_sv, trace_info, pong);
  });
}

void MarketData::operator()(server::Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("event={{trace_info={}, ack={}}}"_sv, trace_info, ack);
  });
}

void MarketData::operator()(server::Trace<json::Ticker> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    log::info<4>("event={{trace_info={}, ticker={}}}"_sv, trace_info, ticker);
    auto &data = ticker.data;
    const TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = data.symbol,
        .layer{
            .bid_price = data.best_bid_price,
            .bid_quantity = data.best_bid_size,
            .ask_price = data.best_ask_price,
            .ask_quantity = data.best_ask_size,
        },
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    server::create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(server::Trace<json::TickerV2> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker_v2] = event;
    log::info<4>("event={{trace_info={}, ticker_v2={}}}"_sv, trace_info, ticker_v2);
    auto &data = ticker_v2.data;
    auto symbol = data.symbol;
    const TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .layer{
            .bid_price = data.best_bid_price,
            .bid_quantity = data.best_bid_size,
            .ask_price = data.best_ask_price,
            .ask_quantity = data.best_ask_size,
        },
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    server::create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(server::Trace<json::Match> const &event) {
  profile_.match([&]() {
    auto &[trace_info, match] = event;
    log::info<4>("event={{trace_info={}, match={}}}"_sv, trace_info, match);
    auto &data = match.data;
    Trade trade{
        .side = json::map(data.side),
        .price = data.price,
        .quantity = data.size,
        .trade_id = data.trade_id,
    };
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = data.symbol,
        .trades = {&trade, 1},
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    server::create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(server::Trace<json::MarkIndexPrice> const &event) {
  profile_.mark_index_price([&]() {
    auto &[trace_info, mark_index_price] = event;
    log::info<4>("event={{trace_info={}, mark_index_price={}}}"_sv, trace_info, mark_index_price);
    auto symbol = json::strip_symbol_from_topic(mark_index_price.topic);
    auto &data = mark_index_price.data;
    Statistics statistics[] = {
        {
            .type = StatisticsType::SETTLEMENT_PRICE,
            .value = data.mark_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::INDEX_VALUE,
            .value = data.index_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    };
    StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    server::create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(server::Trace<json::FundingRate> const &event) {
  profile_.funding_rate([&]() {
    auto &[trace_info, funding_rate] = event;
    log::info<4>("event={{trace_info={}, funding_rate={}}}"_sv, trace_info, funding_rate);
    auto symbol = json::strip_symbol_from_topic(funding_rate.topic);
    auto &data = funding_rate.data;
    Statistics statistics[] = {
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = data.funding_rate,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    };
    StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    server::create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(server::Trace<json::Level2> const &event) {
  profile_.level2([&]() {
    // auto &[trace_info, level2] = event;
    auto &trace_info = event.trace_info;
    auto &level2 = event.value;
    log::info<4>("event={{trace_info={}, level2={}}}"_sv, trace_info, level2);
    auto symbol = json::strip_symbol_from_topic(level2.topic);
    auto &data = level2.data;
    auto first_sequence = data.sequence;
    auto last_sequence = data.sequence;
    auto previous_sequence = data.sequence - 1;
    auto &collector = shared_.mbp_collector[symbol];
    auto [side, price, quantity] = tools::split(data.change);
    MBPUpdate mbp_update{
        .price = price,
        .quantity = quantity,
        .implied_quantity = NaN,
        .price_level = {},
        .number_of_orders = {},
    };
    roq::span<MBPUpdate> bids_or_asks{&mbp_update, 1}, empty;
    auto bids = side == Side::BUY ? bids_or_asks : empty;
    auto asks = side == Side::SELL ? bids_or_asks : empty;
    try {
      collector(
          bids,
          asks,
          first_sequence,
          last_sequence,
          previous_sequence,
          [&](auto &bids, auto &asks) {  // update
            // log::debug(R"(PUBLISH UPDATE symbol="{}")"_sv, symbol);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::INCREMENTAL,
                .exchange_time_utc = {},
                .exchange_sequence = last_sequence,
            };
            server::create_trace_and_dispatch(
                handler_, trace_info, market_by_price_update, true, false);
          },
          [&](auto &bids, auto &asks, auto sequence) {  // snapshot
            log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"_sv, symbol, sequence);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::SNAPSHOT,
                .exchange_time_utc = {},
                .exchange_sequence = collector.last_sequence(),
            };
            server::Trace event(trace_info, market_by_price_update);
            shared_(event, true, [&](auto &market_by_price) {
              collector.apply(market_by_price, sequence, false);
            });
          },
          [&](auto retries) {  // request
            log::debug(R"(REQUEST symbol="{}" (retries={}))"_sv, symbol, retries);
            if (retries > Flags::ws_mbp_request_max_retries()) {
              log::fatal("Unexpected"_sv);
            }
            auto now = trace_info.source_receive_time;
            shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
          });
    } catch (market::BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"_sv, symbol);
      // XXX HANS publish stale
      collector.clear();
      auto now = trace_info.source_receive_time;
      shared_.request_queue.emplace_back(now + Flags::ws_mbp_request_delay(), symbol);
    }
  });
}

void MarketData::operator()(server::Trace<json::FundingBegin> const &event) {
  profile_.funding_begin([&]() {
    auto &[trace_info, funding_begin] = event;
    log::info<4>("event={{trace_info={}, funding_begin={}}}"_sv, trace_info, funding_begin);
    auto &data = funding_begin.data;
    Statistics statistics[] = {
        {
            .type = StatisticsType::FUNDING_RATE_PREDICTION,
            .value = data.funding_rate,
            .begin_time_utc = utils::safe_cast(data.funding_time),
            .end_time_utc = {},
        },
    };
    StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = data.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    server::create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(server::Trace<json::FundingEnd> const &event) {
  profile_.funding_end([&]() {
    auto &[trace_info, funding_end] = event;
    log::info<4>("event={{trace_info={}, funding_end={}}}"_sv, trace_info, funding_end);
    // what to do?
  });
}

void MarketData::operator()(server::Trace<json::Snapshot24h> const &event) {
  profile_.snapshot_24h([&]() {
    auto &[trace_info, snapshot_24h] = event;
    log::info<4>("event={{trace_info={}, snapshot_24h={}}}"_sv, trace_info, snapshot_24h);
    auto symbol = json::strip_symbol_from_topic(snapshot_24h.topic);
    auto &data = snapshot_24h.data;
    Statistics statistics[] = {
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = data.volume,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    };
    StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    server::create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(server::Trace<json::OrderChange> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(server::Trace<json::OrderMarginChange> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(server::Trace<json::AvailableBalanceChange> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(server::Trace<json::WithdrawHoldChange> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(server::Trace<json::PositionChange> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::operator()(server::Trace<json::PositionSettlement> const &) {
  log::fatal("Unexpected"_sv);
}

void MarketData::check_subscribe_queue(std::chrono::nanoseconds now) {
  while (!subscribe_queue_.empty()) {
    auto &tmp = subscribe_queue_.front();
    if (now < tmp.first)
      break;
    if (shared_.can_request(now, [&]() {
          auto &message = tmp.second;
          log::debug(R"(Subscribe: "{}")"_sv, message);
          connection_.send_text(message);
          subscribe_queue_.pop_front();
        })) {
    } else {
      return;
    }
  }
}

}  // namespace kucoin_futures
}  // namespace roq
