/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/rest.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/charconv.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kucoin_futures/flags.h"

#include "roq/kucoin_futures/json/order_book.h"

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
}  // namespace

Rest::Rest(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
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
          .currencies = create_metrics(name_, "currencies"_sv),
          .symbols = create_metrics(name_, "symbols"_sv),
          .order_book = create_metrics(name_, "order_book"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
      },
      security_(security), shared_(shared),
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
      .write(profile_.currencies, metrics::PROFILE)
      .write(profile_.symbols, metrics::PROFILE)
      .write(profile_.order_book, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::get_order_book(const std::string_view &symbol, uint16_t stream_id) {
  auto method = core::http::Method::GET;
  auto path = "/api/v3/market/orderbook/level2"_sv;
  auto query = fmt::format("?symbol={}"_sv, symbol);
  auto headers = security_.create_signature_api_v1(method, path, query, {});
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
      [this, symbol = std::string{symbol}, stream_id](
          [[maybe_unused]] auto &request_id, auto &response) {
        profile_.order_book([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto body = response.body();
            log::debug(R"(body="{}")"_sv, body);
            core::json::Buffer buffer(decode_buffer_);
            auto order_book = core::json::Parser::create<json::OrderBook>(body, buffer);
            log::debug("order_book={}"_sv, order_book);
            // XXX HANS publish mbp snapshot
            Level2Snapshot response{
                .symbol = symbol,
                .sequence = order_book.sequence,
                .stream_id = stream_id,
            };
            handler_(response);
          } catch (core::NetworkError &e) {
            log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
            Level2Snapshot response{
                .symbol = symbol,
                .sequence = -1,
                .stream_id = stream_id,
            };
            handler_(response);
          }
        });
      });
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

template <>
void Rest::get(std::function<void(const core::Promise<json::Token> &)> &&callback) {
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
      "public_token"_sv,
      request,
      [this, callback{std::move(callback)}]([[maybe_unused]] auto &request_id, auto &response) {
        profile_.public_token([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto body = response.body();
            log::debug(R"(body="{}")"_sv, body);
            core::json::Buffer buffer(decode_buffer_);
            auto token = core::json::Parser::create<json::Token>(body, buffer);
            if (utils::compare(token.code, "200000"_sv) == 0) {
              log::info<1>("token={}"_sv, token);
              core::Promise<json::Token> promise(token);
              callback(promise);
            } else {
              log::warn("token={}"_sv, token);
              log::fatal("Unexpected"_sv);
            }
          } catch (core::NetworkError &e) {
            log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
            core::Promise<json::Token> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

template <>
void Rest::get(std::function<void(const core::Promise<json::Currencies> &)> &&callback) {
  auto method = core::http::Method::GET;
  auto path = "/api/v1/currencies"_sv;
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
  connection_(
      "currencies"_sv,
      request,
      [this, callback{std::move(callback)}]([[maybe_unused]] auto &request_id, auto &response) {
        profile_.currencies([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto body = response.body();
            log::debug(R"(body="{}")"_sv, body);
            core::json::Buffer buffer(decode_buffer_);
            auto currencies = core::json::Parser::create<json::Currencies>(body, buffer);
            if (utils::compare(currencies.code, "200000"_sv) == 0) {
              log::info<1>("currencies={}"_sv, currencies);
              core::Promise<json::Currencies> promise(currencies);
              callback(promise);
            } else {
              log::warn("currencies={}"_sv, currencies);
              log::fatal("Unexpected"_sv);
            }
          } catch (core::NetworkError &e) {
            log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
            core::Promise<json::Currencies> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

template <>
void Rest::get(std::function<void(const core::Promise<json::Symbols> &)> &&callback) {
  auto method = core::http::Method::GET;
  auto path = "/api/v1/symbols"_sv;
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
  connection_(
      "symbols"_sv,
      request,
      [this, callback{std::move(callback)}]([[maybe_unused]] auto &request_id, auto &response) {
        profile_.symbols([&]() {
          try {
            response.expect(core::http::Status::OK);
            auto body = response.body();
            log::debug(R"(body="{}")"_sv, body);
            core::json::Buffer buffer(decode_buffer_);
            auto symbols = core::json::Parser::create<json::Symbols>(body, buffer);
            if (utils::compare(symbols.code, "200000"_sv) == 0) {
              log::info<1>("symbols={}"_sv, symbols);
              core::Promise<json::Symbols> promise(symbols);
              callback(promise);
            } else {
              log::warn("symbols={}"_sv, symbols);
              log::fatal("Unexpected"_sv);
            }
          } catch (core::NetworkError &e) {
            log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
            core::Promise<json::Symbols> promise(std::current_exception());
            callback(promise);
          }
        });
      });
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
      download_public_token();
      return 1;
    case RestState::CURRENCIES:
      download_currencies();
      return 1;
    case RestState::SYMBOLS:
      download_symbols();
      return 1;
    case RestState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

void Rest::download_public_token() {
  constexpr auto state = RestState::PUBLIC_TOKEN;
  auto sequence = download_.sequence();
  get<json::Token>([this, sequence](auto &promise) {
    try {
      if (download_.skip(sequence, state))
        return;
      (*this)(promise.get());
      download_.check(state);
    } catch (core::NetworkError &) {
      download_.retry(state);
    }
  });
}

void Rest::download_currencies() {
  constexpr auto state = RestState::CURRENCIES;
  auto sequence = download_.sequence();
  get<json::Currencies>([this, sequence](auto &promise) {
    try {
      if (download_.skip(sequence, state))
        return;
      (*this)(promise.get());
      download_.check(state);
    } catch (core::NetworkError &) {
      download_.retry(state);
    }
  });
}

void Rest::download_symbols() {
  constexpr auto state = RestState::SYMBOLS;
  auto sequence = download_.sequence();
  get<json::Symbols>([this, sequence](auto &promise) {
    try {
      if (download_.skip(sequence, state))
        return;
      (*this)(promise.get());
      download_.check(state);
    } catch (core::NetworkError &) {
      download_.retry(state);
    }
  });
}

void Rest::operator()(const json::Token &token) {
  log::info<2>("token={}"_sv, token);
  if (std::empty(token.data.instance_servers))
    log::fatal("Unexpected: no instance servers"_sv);
  auto &instance_server = token.data.instance_servers[0];
  auto uri = fmt::format("{}?token={}"_sv, instance_server.endpoint, token.data.token);
  PublicToken const public_token{
      .token = token.data.token,
      .endpoint = instance_server.endpoint,
      .uri = uri,
      .ping_frequency = instance_server.ping_interval,
  };
  if (public_token.ping_frequency.count() == 0)
    log::fatal("Unexpected: ping_interval={}"_sv, instance_server.ping_interval);
  handler_(public_token);
}

void Rest::operator()(const json::Currencies &currencies) {
  log::info<2>("currencies={}"_sv, currencies);
}

void Rest::operator()(const json::Symbols &symbols) {
  log::info<2>("symbols={}"_sv, symbols);
  server::TraceInfo trace_info;  // XXX not correct (*parsing* already done)
  // reference data
  std::vector<std::string> symbols_2;
  // symbols_2.reserve(std::size(symbols.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(symbols.data); ++i) {
    auto &item = symbols.data[i];
    auto &symbol = item.symbol;
    if (shared_.discard_symbol(item.symbol))
      continue;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols_2.emplace_back(symbol);
    ++counter;
    const ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = item.name,
        .security_type = {},
        .currency = item.quote_currency,            // XXX check
        .settlement_currency = item.base_currency,  // XXX check
        .commission_currency = {},
        .tick_size = item.price_increment,  // XXX check
        .multiplier = 1.0,
        .min_trade_vol = item.quote_min_size,  // XXX check
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
    };
    server::create_trace_and_dispatch(trace_info, reference_data, handler_, true);
  }
  if (!std::empty(symbols_2)) {
    SymbolsUpdate symbols_update{
        .symbols = symbols_2,
    };
    handler_(symbols_update);
  }
  if (ROQ_UNLIKELY(counter > 0))
    log::info("Symbols {} / {}"_sv, counter, std::size(symbols.data));
  // market status
  for (auto &item : symbols.data) {
    auto &symbol = item.symbol;
    if (all_symbols_.find(symbol) == all_symbols_.end())
      continue;
    const MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .trading_status = item.enable_trading ? TradingStatus::OPEN : TradingStatus::CLOSE,
    };
    server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
  }
}

}  // namespace kucoin_futures
}  // namespace roq
