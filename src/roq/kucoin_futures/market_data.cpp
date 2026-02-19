/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/market_data.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/kucoin_futures/json/map.hpp"
#include "roq/kucoin_futures/json/utils.hpp"

#include "roq/kucoin_futures/tools/splitter.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context, auto const &uri) {
  io::web::URI uri_{uri};
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri_, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Shared &shared,
    size_t index,
    std::string_view const &uri,
    std::string_view const &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, ping_frequency_{ping_frequency}, query_{query},
      connection_{create_connection(*this, shared.settings, context, uri)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
          .total_bytes_received = create_metrics(shared.settings, name_, "total_bytes_received"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .welcome = create_metrics(shared.settings, name_, "welcome"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .pong = create_metrics(shared.settings, name_, "pong"sv),
          .ack = create_metrics(shared.settings, name_, "ack"sv),
          .ticker_v2 = create_metrics(shared.settings, name_, "ticker_v2"sv),
          .ticker = create_metrics(shared.settings, name_, "ticker"sv),
          .match = create_metrics(shared.settings, name_, "match"sv),
          .execution = create_metrics(shared.settings, name_, "execution"sv),
          .mark_index_price = create_metrics(shared.settings, name_, "mark_index_price"sv),
          .funding_rate = create_metrics(shared.settings, name_, "funding_rate"sv),
          .level2 = create_metrics(shared.settings, name_, "level2"sv),
          .funding_begin = create_metrics(shared.settings, name_, "funding_begin"sv),
          .funding_end = create_metrics(shared.settings, name_, "funding_end"sv),
          .snapshot_24h = create_metrics(shared.settings, name_, "snapshot_24h"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      shared_{shared} {
  if (ping_frequency_.count() == 0) {
    log::fatal("Unexpected"sv);
  }
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
      if (next_ping_ < now) {
        send_ping(now);
      }
      check_subscribe_queue(now);
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."sv);
    (*connection_).close();
  }
}

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      .write(counter_.total_bytes_received, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.welcome, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.ack, metrics::Type::PROFILE)
      .write(profile_.ticker_v2, metrics::Type::PROFILE)
      .write(profile_.ticker, metrics::Type::PROFILE)
      .write(profile_.match, metrics::Type::PROFILE)
      .write(profile_.execution, metrics::Type::PROFILE)
      .write(profile_.mark_index_price, metrics::Type::PROFILE)
      .write(profile_.funding_rate, metrics::Type::PROFILE)
      .write(profile_.level2, metrics::Type::PROFILE)
      .write(profile_.funding_begin, metrics::Type::PROFILE)
      .write(profile_.funding_end, metrics::Type::PROFILE)
      .write(profile_.snapshot_24h, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
}

void MarketData::operator()(web::socket::Client::Connected const &) {
  assert(logon_timeout_.count() == 0);
  auto now = clock::get_system();
  logon_timeout_ = now + shared_.settings.ws.request_timeout;
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
  // experimental
  shared_.mbp_sequencer.clear();  // XXX HANS this is SHARED !!!
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  // note! wait for welcome
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
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
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols)) {
    return;
  }
  subscribe(shared_.api.ticker, symbols);
  subscribe(shared_.api.execution, symbols);
  subscribe(shared_.api.instrument, symbols);
  subscribe(shared_.api.snapshot, symbols);
  subscribe(shared_.api.announcement, symbols);
  subscribe(shared_.api.level2, symbols);
}

void MarketData::subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  for (auto &item : symbols) {
    auto now = clock::get_system();
    auto message = fmt::format(
        R"({{)"
        R"("id":"{}",)"
        R"("type":"subscribe",)"
        R"("topic":"{}:{}",)"
        R"("response":true)"
        R"(}})"sv,
        now.count(),
        topic,
        item);
    subscribe_queue_.emplace_back(message);
  }
}

void MarketData::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("type":"ping")"
      R"(}})"sv,
      now.count());
  (*connection_).send_text(message);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("welcome={}"sv, welcome);
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
    log::info<4>("pong={}"sv, pong);
  });
}

void MarketData::operator()(Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("ack={}"sv, ack);
  });
}

void MarketData::operator()(Trace<json::TickerV2> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker_v2] = event;
    log::info<4>("ticker_v2={}"sv, ticker_v2);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = ticker_v2.data;
    auto symbol = data.symbol;
    auto top_of_book = TopOfBook{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .layer{
            .bid_price = data.best_bid_price,
            .bid_quantity = data.best_bid_size,
            .ask_price = data.best_ask_price,
            .ask_quantity = data.best_ask_size,
        },
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = data.ts,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::Match> const &event) {
  profile_.match([&]() {
    auto &[trace_info, match] = event;
    log::info<4>("match={}"sv, match);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = match.data;
    auto trade = Trade{
        .side = map(data.side),
        .price = data.price,
        .quantity = data.size,
        .trade_id = data.trade_id,
        .taker_order_id = data.taker_order_id,
        .maker_order_id = data.maker_order_id,
    };
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = data.symbol,
        .trades = {&trade, 1},
        .exchange_time_utc = data.ts,
        .exchange_sequence = data.sequence,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::Execution> const &event) {
  profile_.execution([&]() {
    auto &[trace_info, execution] = event;
    log::fatal("Unexpected: execution={}"sv, execution);  // XXX FIXME TODO what is this event ???
    log::info<4>("execution={}"sv, execution);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = execution.data;
    auto trade_id = fmt::format("{}"sv, data.trade_id);  // alloc
    auto trade = Trade{
        .side = map(data.match_side),
        .price = data.price,
        .quantity = data.size,
        .trade_id = trade_id,
        .taker_order_id = {},
        .maker_order_id = {},
    };
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = data.symbol,
        .trades = {&trade, 1},
        .exchange_time_utc = data.ts,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::operator()(Trace<json::MarkIndexPrice> const &event) {
  profile_.mark_index_price([&]() {
    auto &[trace_info, mark_index_price] = event;
    log::info<4>("mark_index_price={}"sv, mark_index_price);
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::strip_symbol_from_topic(mark_index_price.topic);
    auto &data = mark_index_price.data;
    std::array<Statistics, 2> statistics{{
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
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = data.timestamp,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::FundingRate> const &event) {
  profile_.funding_rate([&]() {
    auto &[trace_info, funding_rate] = event;
    log::info<4>("funding_rate={}"sv, funding_rate);
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::strip_symbol_from_topic(funding_rate.topic);
    auto &data = funding_rate.data;
    std::array<Statistics, 1> statistics{{
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = data.funding_rate,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = data.timestamp,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::Level2> const &event) {
  profile_.level2([&]() {
    auto &trace_info = event.trace_info;
    auto &level2 = event.value;
    log::info<4>("level2={}"sv, level2);
    (*connection_).touch(trace_info.source_receive_time);
    auto symbol = json::strip_symbol_from_topic(level2.topic);
    auto &data = level2.data;
    //
    auto first_sequence = data.sequence;
    auto last_sequence = data.sequence;
    auto previous_sequence = data.sequence - 1;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    auto [side, price, quantity] = tools::split(data.change);
    auto mbp_update = MBPUpdate{
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
      auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) -> MarketByPriceUpdate {
        return {
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = update_type,
            .exchange_time_utc = data.timestamp,
            .exchange_sequence = exchange_sequence,
            .sending_time_utc = {},
            .price_precision = {},
            .quantity_precision = {},
            .max_depth = {},
            .checksum = {},
        };
      };
      auto publish_update = [&](auto &bids, auto &asks) {
        auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL, last_sequence);
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      };
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, [[maybe_unused]] auto retries, [[maybe_unused]] auto delay) {
        log::info(R"(DEBUG PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
        auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT, sequencer.last_sequence());
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, false); });
      };
      auto request_snapshot = [&](auto retries) {
        log::info(R"(DEBUG REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
        if (shared_.settings.ws.mbp_request_max_retries && shared_.settings.ws.mbp_request_max_retries < retries) {
          log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
        }
        shared_.depth_request_queue.emplace_back(symbol);
      };
      sequencer(bids, asks, first_sequence, last_sequence, previous_sequence, publish_update, publish_snapshot, request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      // XXX HANS publish stale
      sequencer.clear();
      shared_.depth_request_queue.emplace_back(symbol);
    }
  });
}

void MarketData::operator()(Trace<json::FundingBegin> const &event) {
  profile_.funding_begin([&]() {
    auto &[trace_info, funding_begin] = event;
    log::info<4>("funding_begin={}"sv, funding_begin);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = funding_begin.data;
    std::array<Statistics, 1> statistics{{
        {
            .type = StatisticsType::FUNDING_RATE_PREDICTION,
            .value = data.funding_rate,
            .begin_time_utc = utils::safe_cast(data.funding_time),
            .end_time_utc = {},
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = data.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = data.timestamp,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::FundingEnd> const &event) {
  profile_.funding_end([&]() {
    auto &[trace_info, funding_end] = event;
    log::info<4>("funding_end={}"sv, funding_end);
    (*connection_).touch(trace_info.source_receive_time);
    // what to do?
  });
}

void MarketData::operator()(Trace<json::Snapshot24h> const &event) {
  profile_.snapshot_24h([&]() {
    auto &[trace_info, snapshot_24h] = event;
    log::info<4>("snapshot_24h={}"sv, snapshot_24h);
    (*connection_).touch(trace_info.source_receive_time);
    auto &data = snapshot_24h.data;
    std::array<Statistics, 3> statistics{{
        {
            .type = StatisticsType::HIGHEST_TRADED_PRICE,
            .value = data.high_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::LOWEST_TRADED_PRICE,
            .value = data.low_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = data.volume,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = data.symbol,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = data.ts,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
  });
}

void MarketData::operator()(Trace<json::WalletBalanceChange> const &) {
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

void MarketData::operator()(Trace<json::PositionAdjustRiskLimit> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::SymbolOrderChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(Trace<json::OrderChange> const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::check_subscribe_queue(std::chrono::nanoseconds now) {
  subscribe_queue_.dispatch([&](auto now) { return shared_.rate_limiter.can_request(now); }, [&](auto &message) { (*connection_).send_text(message); }, now);
}

}  // namespace kucoin_futures
}  // namespace roq
