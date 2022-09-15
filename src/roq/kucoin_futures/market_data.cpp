/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/market_data.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

#include "roq/kucoin_futures/tools/splitter.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

namespace {
auto const NAME = "md"sv;
const Mask SUPPORTS{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context, auto const &uri, auto const &query) {
  io::web::URI uri_{uri};
  web::socket::Client::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = server::Flags::net_disconnect_on_idle_timeout(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri_, 1},
      .query = query,
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.size,
      .implied_quantity = NaN,
      .number_of_orders = {},
      .update_action = {},
      .price_level = {},
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler,
    io::Context &context,
    uint32_t stream_id,
    Shared &shared,
    size_t index,
    std::string_view const &uri,
    std::string_view const &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)), index_(index),
      ping_frequency_(ping_frequency), connection_(create_connection(*this, context, uri, query)),
      decode_buffer_(Flags::decode_buffer_size()),
      request_id_(static_cast<uint64_t>(stream_id_) * 1000000),  // scale (debugging)
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
          .total_bytes_received = create_metrics(name_, "total_bytes_received"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .welcome = create_metrics(name_, "welcome"sv),
          .error = create_metrics(name_, "error"sv),
          .pong = create_metrics(name_, "pong"sv),
          .ack = create_metrics(name_, "ack"sv),
          .ticker_v2 = create_metrics(name_, "ticker_v2"sv),
          .ticker = create_metrics(name_, "ticker"sv),
          .match = create_metrics(name_, "match"sv),
          .execution = create_metrics(name_, "execution"sv),
          .mark_index_price = create_metrics(name_, "mark_index_price"sv),
          .funding_rate = create_metrics(name_, "funding_rate"sv),
          .level2 = create_metrics(name_, "level2"sv),
          .funding_begin = create_metrics(name_, "funding_begin"sv),
          .funding_end = create_metrics(name_, "funding_end"sv),
          .snapshot_24h = create_metrics(name_, "snapshot_24h"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      shared_(shared) {
  if (ping_frequency_.count() == 0)
    log::fatal("Unexpected"sv);
  log::info("ping_frequency={}"sv, ping_frequency_);
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if ((*connection_).ready()) {
    if (welcome_) {
      if (next_ping_ < now)
        send_ping(now);
      check_subscribe_queue(now);
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."sv);
    (*connection_).close();
  }
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      .write(counter_.total_bytes_received, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.welcome, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.pong, metrics::PROFILE)
      .write(profile_.ack, metrics::PROFILE)
      .write(profile_.ticker_v2, metrics::PROFILE)
      .write(profile_.ticker, metrics::PROFILE)
      .write(profile_.match, metrics::PROFILE)
      .write(profile_.execution, metrics::PROFILE)
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

void MarketData::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MarketData::operator()(web::socket::Client::Connected const &) {
  assert(logon_timeout_.count() == 0);
  auto now = core::clock::GetSystem();
  logon_timeout_ = now + Flags::ws_request_timeout();
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
  // experimental
  shared_.mbp_collector.clear();  // XXX HANS this is SHARED !!!
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  // note! wait for welcome
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
  counter_.total_bytes_received.update((*connection_).total_bytes_received());
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  // subscribe("/contract/announcement"sv);  // XXX deprecated with v2
  if (std::empty(symbols))
    return;
  subscribe(shared_.api.execution, symbols);
  subscribe(shared_.api.level2, symbols);
  switch (shared_.api.version) {
    case 1: {
      subscribe("/contract/instrument"sv, symbols);  // replaced with mark price + funding rate
      if (Flags::ws_subscribe_ticker_v2())
        subscribe(shared_.api.ticker, symbols);
      else
        subscribe("/contractMarket/ticker"sv, symbols);
      break;
    }
    case 2: {
      subscribe(shared_.api.mark_price);
      subscribe(shared_.api.funding_rate, symbols);
      subscribe(shared_.api.ticker, symbols);
      break;
    }
    default:
      log::fatal("Unexpected"sv);
  }
}

void MarketData::subscribe(std::string_view const &topic) {
  auto now = core::clock::GetSystem();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"sv,
      now.count(),
      topic);
  log::debug("message={}"sv, message);
  subscribe_queue_.emplace_back(message);
}

void MarketData::subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    auto now = core::clock::GetSystem();
    auto message = fmt::format(
        R"({{)"
        R"("id":"{}",)"
        R"("type":"subscribe",)"
        R"("topic":"{}:{}",)"
        R"("response":true)"
        R"(}})"sv,
        now.count(),
        topic,
        symbol);
    log::debug("message={}"sv, message);
    subscribe_queue_.emplace_back(message);
  }
}

void MarketData::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(R"({{"id":{},"type":"ping"}})"sv, now.count());
  log::debug<1>(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("event={{welcome={}, trace_info={}}}"sv, welcome, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    welcome_ = true;
    (*this)(ConnectionStatus::READY);
    subscribe();
  });
}

void MarketData::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    // XXX HANS DEBUG
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
    // log::fatal("event={{error={}, trace_info={}}}"sv, error, trace_info);
  });
}

void MarketData::operator()(Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("event={{pong={}, trace_info={}}}"sv, pong, trace_info);
  });
}

void MarketData::operator()(Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("event={{ack={}, trace_info={}}}"sv, ack, trace_info);
  });
}

void MarketData::operator()(Trace<json::Ticker> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    log::info<4>("event={{ticker={}, trace_info={}}}"sv, ticker, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = ticker.data;
    auto ts_factor = shared_.api.version == 1 ? 1 : 1000000;
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
        .exchange_time_utc = utils::safe_cast(data.ts * ts_factor),
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::TickerV2> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker_v2] = event;
    log::info<4>("event={{ticker_v2={}, trace_info={}}}"sv, ticker_v2, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
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
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::Match> const &event) {
  profile_.match([&]() {
    auto &[trace_info, match] = event;
    log::info<4>("event={{match={}, trace_info={}}}"sv, match, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
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
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::Execution> const &event) {
  profile_.execution([&]() {
    auto &[trace_info, execution] = event;
    log::info<4>("event={{execution={}, trace_info={}}}"sv, execution, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = execution.data;
    auto trade_id = fmt::format("{}"sv, data.trade_id);  // alloc
    Trade trade{
        .side = json::map(data.match_side),
        .price = data.price,
        .quantity = data.size,
        .trade_id = trade_id,
    };
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = data.symbol,
        .trades = {&trade, 1},
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::MarkIndexPrice> const &event) {
  profile_.mark_index_price([&]() {
    auto &[trace_info, mark_index_price] = event;
    log::info<4>("event={{mark_index_price={}, trace_info={}}}"sv, mark_index_price, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = mark_index_price.data;
    auto symbol = shared_.api.version == 1 ? json::strip_symbol_from_topic(mark_index_price.topic) : data.symbol;
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
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::FundingRate> const &event) {
  profile_.funding_rate([&]() {
    auto &[trace_info, funding_rate] = event;
    log::info<4>("event={{funding_rate={}, trace_info={}}}"sv, funding_rate, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
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
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::Level2> const &event) {
  profile_.level2([&]() {
    // auto &[trace_info, level2] = event;
    auto &trace_info = event.trace_info;
    auto &level2 = event.value;
    log::info<4>("event={{level2={}, trace_info={}}}"sv, level2, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::strip_symbol_from_topic(level2.topic);
    auto &data = level2.data;
    if (shared_.api.version == 1) {
      auto first_sequence = data.sequence;
      auto last_sequence = data.sequence;
      auto previous_sequence = data.sequence - 1;
      auto &collector = shared_.mbp_collector[symbol];
      auto [side, price, quantity] = tools::split(data.change);
      MBPUpdate mbp_update{
          .price = price,
          .quantity = quantity,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      std::span<MBPUpdate> bids_or_asks{&mbp_update, 1}, empty;
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
                                           // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
              const MarketByPriceUpdate market_by_price_update{
                  .stream_id = stream_id_,
                  .exchange = Flags::exchange(),
                  .symbol = symbol,
                  .bids = bids,
                  .asks = asks,
                  .update_type = UpdateType::INCREMENTAL,
                  .exchange_time_utc = {},
                  .exchange_sequence = last_sequence,
                  .price_decimals = {},
                  .quantity_decimals = {},
                  .checksum = {},
              };
              create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
            },
            [&](auto &bids, auto &asks, auto sequence) {  // snapshot
              log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
              const MarketByPriceUpdate market_by_price_update{
                  .stream_id = stream_id_,
                  .exchange = Flags::exchange(),
                  .symbol = symbol,
                  .bids = bids,
                  .asks = asks,
                  .update_type = UpdateType::SNAPSHOT,
                  .exchange_time_utc = {},
                  .exchange_sequence = collector.last_sequence(),
                  .price_decimals = {},
                  .quantity_decimals = {},
                  .checksum = {},
              };
              Trace event(trace_info, market_by_price_update);
              shared_(event, true, [&](auto &market_by_price) { collector.apply(market_by_price, sequence, false); });
            },
            [&](auto retries) {  // request
              log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
              if (Flags::ws_mbp_request_max_retries() && Flags::ws_mbp_request_max_retries() < retries) {
                log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
              }
              auto now = trace_info.source_receive_time;
              shared_.depth_request_queue.emplace_back(symbol);
            });
      } catch (BadState &) {
        log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
        // XXX HANS publish stale
        collector.clear();
        auto now = trace_info.source_receive_time;
        shared_.depth_request_queue.emplace_back(symbol);
      }
    } else {
      auto first_sequence = data.start + 1;
      auto last_sequence = data.end;
      auto previous_sequence = data.start;
      auto &collector = shared_.mbp_collector[symbol];
      core::back_emplacer bids(shared_.bids), asks(shared_.asks);
      for (auto &item : data.bids)
        bids.emplace_back([&item](auto &result) { emplace(result, item); });
      for (auto &item : data.asks)
        asks.emplace_back([&item](auto &result) { emplace(result, item); });
      try {
        // log::debug("sequence=({}, {})"sv, first_sequence, last_sequence);
        collector(
            bids,
            asks,
            first_sequence,
            last_sequence,
            previous_sequence,
            [&](auto &bids, auto &asks) {  // update
              // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
              const MarketByPriceUpdate market_by_price_update{
                  .stream_id = stream_id_,
                  .exchange = Flags::exchange(),
                  .symbol = symbol,
                  .bids = bids,
                  .asks = asks,
                  .update_type = UpdateType::INCREMENTAL,
                  .exchange_time_utc = data.ts,
                  .exchange_sequence = last_sequence,
                  .price_decimals = {},
                  .quantity_decimals = {},
                  .checksum = {},
              };
              create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
            },
            [&](auto &bids, auto &asks, auto sequence) {  // snapshot
              log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
              const MarketByPriceUpdate market_by_price_update{
                  .stream_id = stream_id_,
                  .exchange = Flags::exchange(),
                  .symbol = symbol,
                  .bids = bids,
                  .asks = asks,
                  .update_type = UpdateType::SNAPSHOT,
                  .exchange_time_utc = data.ts,
                  .exchange_sequence = collector.last_sequence(),
                  .price_decimals = {},
                  .quantity_decimals = {},
                  .checksum = {},
              };
              Trace event(trace_info, market_by_price_update);
              shared_(event, true, [&](auto &market_by_price) { collector.apply(market_by_price, sequence, false); });
            },
            [&](auto retries) {  // request
              log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
              if (Flags::ws_mbp_request_max_retries() && Flags::ws_mbp_request_max_retries() < retries) {
                log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
              }
              auto now = trace_info.source_receive_time;
              shared_.depth_request_queue.emplace_back(symbol);
            });
      } catch (BadState &) {
        log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
        // XXX HANS publish stale
        collector.clear();
        auto now = trace_info.source_receive_time;
        shared_.depth_request_queue.emplace_back(symbol);
      }
    }
  });
}

void MarketData::operator()(Trace<json::FundingBegin> const &event) {
  profile_.funding_begin([&]() {
    auto &[trace_info, funding_begin] = event;
    log::info<4>("event={{funding_begin={}, trace_info={}}}"sv, funding_begin, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = funding_begin.data;
    Statistics statistics[] = {
        {
            .type = StatisticsType::FUNDING_RATE_PREDICTION,
            .value = data.funding_rate,
            .begin_time_utc = utils::safe_cast(data.funding_time),
            .end_time_utc = {},
        },
    };
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = data.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.timestamp),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::FundingEnd> const &event) {
  profile_.funding_end([&]() {
    auto &[trace_info, funding_end] = event;
    log::info<4>("event={{funding_end={}, trace_info={}}}"sv, funding_end, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
    // what to do?
  });
}

void MarketData::operator()(Trace<json::Snapshot24h> const &event) {
  profile_.snapshot_24h([&]() {
    auto &[trace_info, snapshot_24h] = event;
    log::info<4>("event={{snapshot_24h={}, trace_info={}}}"sv, snapshot_24h, trace_info);
    (*connection_).touch(trace_info.source_receive_time);
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
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::OrderChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::OrderMarginChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::AvailableBalanceChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::WithdrawHoldChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::PositionChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::PositionSettlement> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::check_subscribe_queue(std::chrono::nanoseconds now) {
  subscribe_queue_.dispatch(
      [&](auto now) { return shared_.rate_limiter.can_request(now); },
      [&](auto &message) {
        log::debug(R"(Subscribe: "{}")"sv, message);
        (*connection_).send_text(message);
      },
      now);
}

}  // namespace kucoin_futures
}  // namespace roq
