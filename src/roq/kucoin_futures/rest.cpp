/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

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

size_t const MAX_DECODE_BUFFER_DEPTH = 1;

int32_t const SYSTEM_CODE_SUCCESS = 200000;

auto const CFI_CODE_SWAP = "FFWCSX"sv;
auto const CFI_CODE_FUTURES = "FFICSX"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .public_token = create_metrics(shared.settings, name_, "public_token"sv),
          .public_token_ack = create_metrics(shared.settings, name_, "public_token_ack"sv),
          .contracts = create_metrics(shared.settings, name_, "contracts"sv),
          .contracts_ack = create_metrics(shared.settings, name_, "contracts_ack"sv),
          .order_book = create_metrics(shared.settings, name_, "order_book"sv),
          .order_book_ack = create_metrics(shared.settings, name_, "order_book_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
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
  if (ready()) {
    check_request_queue(now);
  }
}

void Rest::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.public_token, metrics::Type::PROFILE)
      .write(profile_.public_token_ack, metrics::Type::PROFILE)
      .write(profile_.contracts, metrics::Type::PROFILE)
      .write(profile_.contracts_ack, metrics::Type::PROFILE)
      .write(profile_.order_book, metrics::Type::PROFILE)
      .write(profile_.order_book_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
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

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void Rest::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
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
      return 0;
  }
  assert(false);
  return 0;
}

// bullet-public

void Rest::get_public_token() {
  profile_.public_token([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = shared_.api.rest_public.bullet_public,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_public_token_ack(event, sequence);
    };
    (*connection_)("public_token"sv, request, callback);
  });
}

void Rest::get_public_token_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = RestState::PUBLIC_TOKEN;
  profile_.public_token_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        json::Token token{body, decode_buffer_};
        if (token.code == SYSTEM_CODE_SUCCESS) {
          Trace event_2{event, token};
          (*this)(event_2);
          download_.check(STATE);
        } else {
          log::fatal("Unexpected: token={}"sv, token);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<json::Token> const &event) {
  auto &[trace_info, token] = event;
  log::info<2>("token={}"sv, token);
  if (std::empty(token.data.instance_servers)) {
    log::fatal("Unexpected: no instance servers"sv);
  }
  auto &instance_server = token.data.instance_servers[0];
  auto query = fmt::format("?token={}"sv, token.data.token);
  auto public_token = PublicToken{
      .uri = instance_server.endpoint,
      .query = query,
      .ping_frequency = instance_server.ping_interval,
  };
  if (public_token.ping_frequency.count() == 0) {
    log::fatal("Unexpected: ping_interval={}"sv, instance_server.ping_interval);
  }
  handler_(public_token);
}

// contracts

void Rest::get_contracts() {
  profile_.contracts([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.rest_public.contracts_active,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_contracts_ack(event, sequence);
    };
    (*connection_)("contracts"sv, request, callback);
  });
}

void Rest::get_contracts_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = RestState::CONTRACTS;
  profile_.contracts_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        json::ContractsAck contracts_ack{body, decode_buffer_};
        if (contracts_ack.code == SYSTEM_CODE_SUCCESS) {
          Trace event_2{event, contracts_ack};
          (*this)(event_2);
          download_.check(STATE);
        } else {
          handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(contracts_ack.code), contracts_ack.msg);
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<json::ContractsAck> const &event) {
  auto &[trace_info, contracts_ack] = event;
  log::info<4>("contracts_ack={}"sv, contracts_ack);
  // reference data
  std::vector<Symbol> symbols;
  size_t counter = 0;
  for (auto &item : contracts_ack.data) {
    log::info<2>("item={}"sv, item);
    auto &symbol = item.symbol;
    auto security_type = [&]() -> SecurityType {
      if (item.type == CFI_CODE_SWAP) {
        return SecurityType::SWAP;
      }
      if (item.type == CFI_CODE_FUTURES) {
        return SecurityType::FUTURES;
      }
      return {};
    }();
    auto discard = shared_.discard_symbol(symbol);
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .description = {},
        .security_type = security_type,
        .external_security_id = {},
        .cfi_code = item.type,
        .base_currency = item.base_currency,
        .quote_currency = item.quote_currency,
        .settlement_currency = item.settle_currency,
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = item.tick_size,
        .tick_size_steps = {},
        .multiplier = item.multiplier,
        .min_notional = NaN,
        .min_trade_vol = item.lot_size,
        .max_trade_vol = item.max_order_qty,
        .trade_vol_step_size = item.lot_size,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = item.root_symbol,
        .time_zone = {},
        .issue_date = utils::safe_cast(item.first_open_date),
        .settlement_date = utils::safe_cast(item.settle_date),
        .expiry_datetime = utils::safe_cast(item.expire_date),
        .expiry_datetime_utc = utils::safe_cast(item.expire_date),
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
        .discard = discard,
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, true);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, item.symbol);
      continue;
    }
    if (shared_.all_symbols.emplace(symbol).second) {  // only include new
      symbols.emplace_back(symbol);
    }
    ++counter;
  }
  if (!std::empty(symbols)) {
    auto symbols_update = SymbolsUpdate{
        .symbols = symbols,
    };
    handler_(symbols_update);
  }
  if (counter > 0) {
    log::info("Contracts {} / {}"sv, counter, std::size(contracts_ack.data));
  }
  // market status
  for (auto &item : contracts_ack.data) {
    auto &symbol = item.symbol;
    if (shared_.all_symbols.find(symbol) == std::end(shared_.all_symbols)) {
      continue;
    }
    auto trading_status = [&]() -> TradingStatus {
      switch (item.status) {
        using enum json::Status::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          break;
        case OPEN:
          return TradingStatus::OPEN;
      }
      return {};
    }();
    auto market_status = MarketStatus{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .trading_status = trading_status,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  }
}

// order-book

void Rest::get_order_book(std::string_view const &symbol) {
  profile_.order_book([&]() {
    auto query = fmt::format("?symbol={}"sv, symbol);
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.rest_public.level2_snapshot,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_order_book_ack(event, symbol);
    };
    (*connection_)("order_book"sv, request, callback);
  });
}

// XXX TODO FIXME we need the symbol for retrying !!!
void Rest::get_order_book_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] std::string_view const &symbol) {
  profile_.order_book_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      // XXX WHAT ???
    };
    auto handle_success = [&](auto &body) {
      json::OrderBookAck order_book_ack{body, decode_buffer_};
      if (order_book_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, order_book_ack};
        (*this)(event_2);
      };
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<json::OrderBookAck> const &event) {
  auto &[trace_info, order_book_ack] = event;
  log::info<4>("order_book_ack={}"sv, order_book_ack);
  auto &data = order_book_ack.data;
  auto sequence = data.sequence;
  auto symbol = data.symbol;
  auto &sequencer = shared_.mbp_sequencer[symbol];
  auto &mbp = shared_.get_mbp();
  auto emplace_back = [](auto &result, auto &item) {
    auto mbp_update = MBPUpdate{
        .price = item.price,
        .quantity = item.size,
        .implied_quantity = NaN,
        .number_of_orders = {},
        .update_action = {},
        .price_level = {},
    };
    result.emplace_back(std::move(mbp_update));
  };
  for (auto &item : data.bids) {
    emplace_back(mbp.bids, item);
  }
  for (auto &item : data.asks) {
    emplace_back(mbp.asks, item);
  }
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, [[maybe_unused]] auto retries, [[maybe_unused]] auto delay) {
      log::info(R"(DEBUG PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .bids = bids,
          .asks = asks,
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = data.ts,
          .exchange_sequence = sequencer.last_sequence(),
          .sending_time_utc = data.ts,
          .price_precision = {},
          .quantity_precision = {},
          .max_depth = {},
          .checksum = {},
      };
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
    sequencer(mbp.bids, mbp.asks, sequence, false, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
    // XXX HANS publish stale
    sequencer.clear();
    shared_.depth_request_queue.emplace_back(symbol);
  }
}

void Rest::check_request_queue(std::chrono::nanoseconds now) {
  shared_.depth_request_queue.dispatch(
      [&](auto now) { return shared_.rate_limiter.can_request(now); },
      [&](auto &symbol) {
        log::info(R"(DEBUG Requesting order book snapshot symbol="{}")"sv, symbol);
        get_order_book(symbol);
      },
      now);
}

void Rest::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR:
        success_handler(body);  // throws
        break;
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
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
