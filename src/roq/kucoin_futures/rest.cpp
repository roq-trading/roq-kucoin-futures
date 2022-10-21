/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/rest/client_factory.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/tools/splitter.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  web::rest::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = web::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return web::rest::ClientFactory::create(handler, context, config);
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(create_name(stream_id_)),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .public_token = create_metrics(name_, "public_token"sv),
          .public_token_ack = create_metrics(name_, "public_token_ack"sv),
          .contracts = create_metrics(name_, "contracts"sv),
          .contracts_ack = create_metrics(name_, "contracts_ack"sv),
          .order_book = create_metrics(name_, "order_book"sv),
          .order_book_ack = create_metrics(name_, "order_book_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      shared_(shared), download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(Event<Start> const &) {
  (*connection_).start();
}

void Rest::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void Rest::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready())
    check_request_queue(now);
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
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void Rest::operator()(web::rest::Client::Connected const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(web::rest::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(web::rest::Client::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    using enum RestState;
    case UNDEFINED:
      assert(false);
      break;
    case PUBLIC_TOKEN:
      get_public_token();
      return 1;
    case CONTRACTS:
      get_contracts();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// public-token

void Rest::get_public_token() {
  profile_.public_token([&]() {
    web::rest::Request request{
        .method = web::http::Method::POST,
        .path = "/api/v1/bullet-public"sv,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event{trace_info, response};
      get_public_token_ack(event, sequence);
    };
    (*connection_)("public_token"sv, request, callback);
  });
}

void Rest::get_public_token_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  constexpr auto const STATE = RestState::PUBLIC_TOKEN;
  profile_.public_token_ack([&]() {
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        core::json::Buffer buffer{decode_buffer_};
        auto token = core::json::Parser::create<json::Token>(body, buffer);
        Trace event_2{event, token};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void Rest::operator()(Trace<json::Token> const &event) {
  auto &[trace_info, token] = event;
  log::info<2>("token={}"sv, token);
  if (std::empty(token.data.instance_servers))
    log::fatal("Unexpected: no instance servers"sv);
  auto &instance_server = token.data.instance_servers[0];
  auto query = fmt::format("?token={}"sv, token.data.token);
  PublicToken const public_token{
      .uri = instance_server.endpoint,
      .query = query,
      .ping_frequency = instance_server.ping_interval,
  };
  if (public_token.ping_frequency.count() == 0)
    log::fatal("Unexpected: ping_interval={}"sv, instance_server.ping_interval);
  handler_(public_token);
}

void Rest::get_contracts() {
  profile_.contracts([&]() {
    web::rest::Request request{
        .method = web::http::Method::GET,
        .path = shared_.api.get_contracts_active,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event{trace_info, response};
      get_contracts_ack(event, sequence);
    };
    (*connection_)("contracts"sv, request, callback);
  });
}

void Rest::get_contracts_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  constexpr auto const STATE = RestState::CONTRACTS;
  profile_.contracts_ack([&]() {
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        core::json::Buffer buffer{decode_buffer_};
        auto contracts = core::json::Parser::create<json::Contracts>(body, buffer);
        Trace event_2{event, contracts};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void Rest::operator()(Trace<json::Contracts> const &event) {
  auto &[trace_info, contracts] = event;
  log::info<4>("contracts={}"sv, contracts);
  // reference data
  std::vector<Symbol> symbols;
  // symbols.reserve(std::size(symbols.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(contracts.data); ++i) {
    auto &item = contracts.data[i];
    log::info<2>("item={}"sv, item);
    auto &symbol = item.symbol;
    auto discard = shared_.discard_symbol(symbol);
    ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = {},
        .security_type = {},
        .base_currency = item.base_currency,
        .quote_currency = item.quote_currency,
        .margin_currency = item.settle_currency,  // correct? is_inverse
        .commission_currency = {},
        .tick_size = item.tick_size,
        .multiplier = item.multiplier,
        .min_notional = NaN,
        .min_trade_vol = 1.0,
        .max_trade_vol = NaN,
        .trade_vol_step_size = 1.0,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = utils::safe_cast(item.first_open_date),
        .settlement_date = utils::safe_cast(item.settle_date),
        .expiry_datetime = utils::safe_cast(item.expire_date),
        .expiry_datetime_utc = utils::safe_cast(item.expire_date),
        .discard = discard,
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, true);
    if (discard)
      continue;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols.emplace_back(symbol);
    ++counter;
  }
  if (!std::empty(symbols)) {
    SymbolsUpdate symbols_update{
        .symbols = symbols,
    };
    handler_(symbols_update);
  }
  if (counter > 0)
    log::info("Contracts {} / {}"sv, counter, std::size(contracts.data));
  // market status
  for (auto &item : contracts.data) {
    auto &symbol = item.symbol;
    if (all_symbols_.find(symbol) == std::end(all_symbols_))
      continue;
    auto trading_status = item.status == json::Status::OPEN ? TradingStatus::OPEN : TradingStatus::CLOSE;
    MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .trading_status = trading_status,
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  }
}

void Rest::get_order_book(std::string_view const &symbol) {
  profile_.order_book([&]() {
    auto query = fmt::format("?symbol={}"sv, symbol);
    web::rest::Request request{
        .method = web::http::Method::GET,
        .path = shared_.api.get_order_book,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event{trace_info, response};
      get_order_book_ack(event, symbol);
    };
    (*connection_)("order_book"sv, request, callback);
  });
}

void Rest::get_order_book_ack(
    Trace<web::rest::Response> const &event, [[maybe_unused]] std::string_view const &symbol) {
  profile_.order_book_ack([&]() {
    auto handle_success = [&](auto &body) {
      core::json::Buffer buffer{decode_buffer_};
      auto order_book = core::json::Parser::create<json::OrderBook>(body, buffer);
      Trace event_2{event, order_book};
      (*this)(event_2);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      // XXX WHAT ???
    };
    process_response(event, handle_success, handle_error);
  });
}

void Rest::operator()(Trace<json::OrderBook> const &event) {
  auto &trace_info = event.trace_info;
  auto &order_book = event.value;
  log::info<4>("event={{order_book={}, trace_info={}}}"sv, order_book, trace_info);
  auto &data = order_book.data;
  auto sequence = data.sequence;
  auto symbol = data.symbol;
  auto &collector = shared_.mbp_collector[symbol];
  core::back_emplacer bids(shared_.bids), asks(shared_.asks);
  auto create_mbp_update = []<typename T>(T &result, auto price, auto size) {
    new (&result) T{
        .price = price,
        .quantity = size,
        .implied_quantity = NaN,
        .number_of_orders = {},
        .update_action = {},
        .price_level = {},
    };
  };
  for (auto &item : data.bids)
    bids.emplace_back([&](auto &result) { create_mbp_update(result, item.price, item.size); });
  for (auto &item : data.asks)
    asks.emplace_back([&](auto &result) { create_mbp_update(result, item.price, item.size); });
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence) {
      log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
      MarketByPriceUpdate market_by_price_update{
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
      Trace event{trace_info, market_by_price_update};
      shared_(event, true, [&](auto &market_by_price) { collector.apply(market_by_price, sequence, false); });
    };
    auto request_snapshot = [&](auto retries) {
      log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
      if (Flags::ws_mbp_request_max_retries() && Flags::ws_mbp_request_max_retries() < retries) {
        log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
      }
      shared_.depth_request_queue.emplace_back(symbol);
    };
    collector(bids, asks, sequence, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
    // XXX HANS publish stale
    collector.clear();
    shared_.depth_request_queue.emplace_back(symbol);
  }
}

void Rest::check_request_queue(std::chrono::nanoseconds now) {
  shared_.depth_request_queue.dispatch(
      [&](auto now) { return shared_.rate_limiter.can_request(now); },
      [&](auto &symbol) {
        log::debug(R"(Requesting order book snapshot symbol="{}")"sv, symbol);
        get_order_book(symbol);
      },
      now);
}

template <typename SuccessHandler, typename ErrorHandler>
void Rest::process_response(
    web::rest::Response const &response, SuccessHandler success_handler, ErrorHandler error_handler) {
  try {
    auto [status, category, body] = response.result();
    log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
    switch (category) {
      using enum web::http::Category;
      case SUCCESS:  // 2xx
        success_handler(body);
        break;
      case CLIENT_ERROR:        // 4xx
        success_handler(body);  // throws
        break;
      case SERVER_ERROR:  // 5xx
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, magic_enum::enum_name(status));
        break;
      default:
        response.expect(web::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}
}  // namespace kucoin_futures
}  // namespace roq
