/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/rest.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kucoin_futures/flags.h"

#include "roq/kucoin_futures/tools/splitter.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {

namespace {
static const auto NAME = "rest"_sv;

static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

void emplace(MBPUpdate &result, double price, double size) {
  new (&result) MBPUpdate{
      .price = price,
      .quantity = size,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .public_token = create_metrics(name_, "public_token"_sv),
          .public_token_ack = create_metrics(name_, "public_token_ack"_sv),
          .contracts = create_metrics(name_, "contracts"_sv),
          .contracts_ack = create_metrics(name_, "contracts_ack"_sv),
          .order_book = create_metrics(name_, "order_book"_sv),
          .order_book_ack = create_metrics(name_, "order_book_ack"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
      },
      shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(const Event<Start> &) {
  connection_.start();
}

void Rest::operator()(const Event<Stop> &) {
  connection_.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.public_token, metrics::PROFILE)
      .write(profile_.public_token_ack, metrics::PROFILE)
      .write(profile_.contracts, metrics::PROFILE)
      .write(profile_.contracts_ack, metrics::PROFILE)
      .write(profile_.order_book, metrics::PROFILE)
      .write(profile_.order_book_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

void Rest::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    case RestState::UNDEFINED:
      assert(false);
      break;
    case RestState::PUBLIC_TOKEN:
      get_public_token();
      return 1;
    case RestState::CONTRACTS:
      get_contracts();
      return 1;
    case RestState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// public-token

void Rest::get_public_token() {
  profile_.public_token([&]() {
    core::web::Request request{
        .method = core::http::Method::POST,
        .path = "/api/v1/bullet-public"_sv,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    connection_(
        "public_token"_sv, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
          get_public_token_ack(response);
        });
  });
}

void Rest::get_public_token_ack(const core::web::Response &response) {
  profile_.public_token_ack([&]() {
    try {
      response.expect(core::http::Status::OK);
      auto body = response.body();
      log::debug(R"(body="{}")"_sv, body);
      core::json::Buffer buffer(decode_buffer_);
      auto token = core::json::Parser::create<json::Token>(body, buffer);
      (*this)(token);
      download_.check(RestState::PUBLIC_TOKEN);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
      download_.retry(RestState::PUBLIC_TOKEN);
    }
  });
}

void Rest::operator()(const json::Token &token) {
  log::info<2>("token={}"_sv, token);
  if (std::empty(token.data.instance_servers))
    log::fatal("Unexpected: no instance servers"_sv);
  auto &instance_server = token.data.instance_servers[0];
  auto query = fmt::format("?token={}"_sv, token.data.token);
  PublicToken const public_token{
      .uri = instance_server.endpoint,
      .query = query,
      .ping_frequency = instance_server.ping_interval,
  };
  if (public_token.ping_frequency.count() == 0)
    log::fatal("Unexpected: ping_interval={}"_sv, instance_server.ping_interval);
  handler_(public_token);
}

// contracts

void Rest::get_contracts() {
  profile_.contracts([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/contracts/active"_sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    connection_("contracts"_sv, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
      get_contracts_ack(response);
    });
  });
}

void Rest::get_contracts_ack(const core::web::Response &response) {
  profile_.contracts_ack([&]() {
    try {
      response.expect(core::http::Status::OK);
      auto body = response.body();
      log::debug(R"(body="{}")"_sv, body);
      core::json::Buffer buffer(decode_buffer_);
      auto contracts = core::json::Parser::create<json::Contracts>(body, buffer);
      (*this)(contracts);
      download_.check(RestState::CONTRACTS);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
      download_.retry(RestState::CONTRACTS);
    }
  });
}

void Rest::operator()(const json::Contracts &contracts) {
  log::info<2>("contracts={}"_sv, contracts);
  server::TraceInfo trace_info;  // XXX not correct (*parsing* already done)
  // reference data
  std::vector<std::string> symbols;
  // symbols.reserve(std::size(symbols.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(contracts.data); ++i) {
    auto &item = contracts.data[i];
    auto &symbol = item.symbol;
    if (shared_.discard_symbol(item.symbol))
      continue;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols.emplace_back(symbol);
    ++counter;
    const ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = {},
        .security_type = {},
        .currency = item.quote_currency,            // XXX check
        .settlement_currency = item.base_currency,  // XXX check
        .commission_currency = {},
        .tick_size = item.tick_size,
        .multiplier = item.multiplier,
        .min_trade_vol = 1.0,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = utils::safe_cast(item.first_open_date),
        .settlement_date = utils::safe_cast(item.settle_date),
        .expiry_datetime = utils::safe_cast(item.expire_date),
        .expiry_datetime_utc = utils::safe_cast(item.expire_date),
    };
    server::create_trace_and_dispatch(trace_info, reference_data, handler_, true);
  }
  if (!std::empty(symbols)) {
    SymbolsUpdate symbols_update{
        .symbols = symbols,
    };
    handler_(symbols_update);
  }
  if (ROQ_UNLIKELY(counter > 0))
    log::info("Contracts {} / {}"_sv, counter, std::size(contracts.data));
  // market status
  for (auto &item : contracts.data) {
    auto &symbol = item.symbol;
    if (all_symbols_.find(symbol) == all_symbols_.end())
      continue;
    auto trading_status =
        item.status == json::Status::OPEN ? TradingStatus::OPEN : TradingStatus::CLOSE;
    const MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .trading_status = trading_status,
    };
    server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
  }
}

// order-book

void Rest::get_order_book(const std::string_view &symbol) {
  profile_.order_book([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/level2/snapshot"_sv;
    auto query = fmt::format("?symbol={}"_sv, symbol);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
        .rate_limit_weight = 1,
    };
    connection_(
        "order_book"_sv,
        request,
        [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
          get_order_book_ack(symbol, response);
        });
  });
}

void Rest::get_order_book_ack(const std::string_view &symbol, const core::web::Response &response) {
  profile_.order_book_ack([&]() {
    try {
      server::TraceInfo trace_info;
      response.expect(core::http::Status::OK);
      auto body = response.body();
      // log::debug(R"(body="{}")"_sv, body);
      core::json::Buffer buffer(decode_buffer_);
      auto order_book = core::json::Parser::create<json::OrderBook>(body, buffer);
      server::Trace event(trace_info, order_book);
      (*this)(event);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
      get_order_book_retry(symbol);
    }
  });
}

void Rest::operator()(server::Trace<json::OrderBook> const &event) {
  auto &[trace_info, order_book] = event;
  log::debug("event={{trace_info={}, order_book={}}}"_sv, trace_info, order_book);
  auto &data = order_book.data;
  auto sequence = data.sequence;
  auto &symbol = data.symbol;
  auto &collector = shared_.mbp_collector[symbol];
  auto &history = collector.history;
  if (history.empty()) {
    // note! probably disconnected
    return;
  }
  // we need the next sequence number to be available
  if ((sequence + 1) < history.front().first) {
    log::warn(
        "No change history for order-book snapshot, sequence={}, first={}"_sv,
        sequence,
        history.front().first);
    get_order_book_retry(symbol);
  } else {
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : data.bids)
      bids.emplace_back([&](auto &result) { emplace(result, item.price, item.size); });
    for (auto &item : data.asks)
      asks.emplace_back([&](auto &result) { emplace(result, item.price, item.size); });
    const MarketByPriceUpdate market_by_price_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .bids = bids,
        .asks = asks,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = utils::safe_cast(data.ts),
    };
    server::create_trace_and_dispatch(
        trace_info, market_by_price_update, shared_, true, [&](auto &market_by_price) {
          std::pair<int64_t, std::string> current{sequence, {}};
          auto iter =
              std::upper_bound(history.begin(), history.end(), current, [](auto &lhs, auto &rhs) {
                return lhs.first < rhs.first;
              });
          for (auto expected = data.sequence + 1; iter != history.end(); ++iter, ++expected) {
            auto [sequence, change] = *iter;
            if (sequence != expected)
              log::fatal("Wrong sequence: expected={}, got={}"_sv, expected, sequence);
            auto [side, price, quantity] = tools::split(change);
            market_by_price(side, price, quantity);
          }
        });
    collector.ready = true;
    history.clear();
  }
}

// XXX HANS we need to count this towards the rate limiter
void Rest::get_order_book_retry(const std::string_view &symbol) {
  auto &collector = shared_.mbp_collector[symbol];
  if (++collector.retries < Flags::ws_mbp_request_max_retries()) {
    log::warn(R"(Retrying order-book for symbol="{}")"_sv, symbol);
    get_order_book(symbol);
  } else {
    log::fatal("Reached max retries"_sv);
  }
}

}  // namespace kucoin_futures
}  // namespace roq
