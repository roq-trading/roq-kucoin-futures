/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/gateway/order_entry_rest.hpp"

#include "roq/mask.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/kucoin_futures/protocol/json/encoder.hpp"
#include "roq/kucoin_futures/protocol/json/map.hpp"
#include "roq/kucoin_futures/protocol/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::FUNDS,
    SupportType::POSITION,
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::TRADE,
};

auto const SUPPORTS_WSAPI = Mask{
    SupportType::FUNDS,
    SupportType::POSITION,
    SupportType::TRADE,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;

int32_t const SYSTEM_CODE_SUCCESS = 200'000;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account.name);
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &shared) {
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
  return web::rest::Client::create(handler, context, config, shared.throttle);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntryREST::OrderEntryREST(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared, Request &request)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account)},
      connection_{create_connection(*this, shared.settings, context, shared)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .private_token = create_metrics(shared.settings, name_, "private_token"sv),
          .private_token_ack = create_metrics(shared.settings, name_, "private_token_ack"sv),
          .account = create_metrics(shared.settings, name_, "account"sv),
          .account_ack = create_metrics(shared.settings, name_, "account_ack"sv),
          .positions = create_metrics(shared.settings, name_, "positions"sv),
          .positions_ack = create_metrics(shared.settings, name_, "positions_ack"sv),
          .orders = create_metrics(shared.settings, name_, "orders"sv),
          .orders_ack = create_metrics(shared.settings, name_, "orders_ack"sv),
          .order = create_metrics(shared.settings, name_, "order"sv),
          .order_ack = create_metrics(shared.settings, name_, "order_ack"sv),
          .fills = create_metrics(shared.settings, name_, "fills"sv),
          .fills_ack = create_metrics(shared.settings, name_, "fills_ack"sv),
          .create_order = create_metrics(shared.settings, name_, "create_order"sv),
          .create_order_ack = create_metrics(shared.settings, name_, "create_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(shared.settings, name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(shared.settings, name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, request_{request}, download_{shared.settings.rest.request_timeout, [this](auto &event) { return download(event); }} {
}

// server::Stream

void OrderEntryREST::operator()(Trace<Start> const &) {
  (*connection_).start();
}

void OrderEntryREST::operator()(Trace<Stop> const &) {
  (*connection_).stop();
}

void OrderEntryREST::operator()(Trace<Timer> const &event) {
  auto &[trace_info, timer] = event;
  if ((*connection_).refresh(trace_info)) {
    if (ready()) {
      check_request_queue(trace_info);
    }
  }
}

void OrderEntryREST::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.private_token, metrics::Type::PROFILE)
      .write(profile_.private_token_ack, metrics::Type::PROFILE)
      .write(profile_.account, metrics::Type::PROFILE)
      .write(profile_.account_ack, metrics::Type::PROFILE)
      .write(profile_.positions, metrics::Type::PROFILE)
      .write(profile_.positions_ack, metrics::Type::PROFILE)
      .write(profile_.orders, metrics::Type::PROFILE)
      .write(profile_.orders_ack, metrics::Type::PROFILE)
      .write(profile_.order, metrics::Type::PROFILE)
      .write(profile_.order_ack, metrics::Type::PROFILE)
      .write(profile_.fills, metrics::Type::PROFILE)
      .write(profile_.fills_ack, metrics::Type::PROFILE)
      .write(profile_.create_order, metrics::Type::PROFILE)
      .write(profile_.create_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void OrderEntryREST::operator()(Trace<ConnectionStatus> const &event, std::string_view const &reason) {
  auto &[trace_info, connection_status] = event;
  connection_status_ = connection_status;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
      .supports = shared_.settings.misc.disable_ws_api ? SUPPORTS : SUPPORTS_WSAPI,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .proxy = (*connection_).get_proxy(),
      .peer = (*connection_).get_current_peer(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

// server::OrderActionStream

uint16_t OrderEntryREST::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  create_order(event, order, ref_data, request_id);
  return stream_id_;
}

uint16_t OrderEntryREST::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntryREST::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntryREST::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

// web::rest::Client::Handler

void OrderEntryREST::operator()(Trace<web::rest::Connected> const &event) {
  auto &[trace_info, disconnected] = event;
  download_.begin(trace_info);
}

void OrderEntryREST::operator()(Trace<web::rest::Disconnected> const &event) {
  auto &[trace_info, disconnected] = event;
  ++counter_.disconnect;
  create_trace_and_dispatch_2(trace_info, ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void OrderEntryREST::operator()(Trace<web::rest::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

// core::Download

int32_t OrderEntryREST::download(Trace<State> const &event) {
  auto &[trace_info, state] = event;
  auto publish_downloading = [&]() { create_trace_and_dispatch_2(trace_info, ConnectionStatus::DOWNLOADING, magic_enum::enum_name(state)); };
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case PRIVATE_TOKEN: {
      if (!has_downloaded_private_token_) {  // note! initial download
        publish_downloading();
        get_private_token(trace_info);
        return 1;
      } else {
        return 0;
      }
    }
    case DONE:
      create_trace_and_dispatch_2(trace_info, ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// bullet-private

void OrderEntryREST::get_private_token(TraceInfo const &trace_info) {
  profile_.private_token([&]() {
    auto method = web::http::Method::POST;
    auto path = shared_.api.rest_private.bullet_private;
    auto headers = account_.create_headers(trace_info, method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this](auto &event, [[maybe_unused]] auto &request_id) { get_private_token_ack(event); };
    (*connection_)(request, callback, "bullet-private");
  });
}

void OrderEntryREST::get_private_token_ack(Trace<web::rest::Response> const &event) {
  auto const STATE = State::PRIVATE_TOKEN;
  profile_.private_token_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      if (download_.downloading()) {
        download_.retry(STATE);
      }
    };
    auto handle_success = [&](auto &body) {
      protocol::json::Token token{body, decode_buffer_};
      if (token.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, token};
        (*this)(event_2);
      } else {
        log::fatal("Unexpected: token={}"sv, token);
      }
      download_.check_relaxed(trace_info, STATE);
      has_downloaded_private_token_ = true;
    };
    process_response(event, handle_error, handle_success);
    request_.private_token.commit(trace_info);
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::Token> const &event) {
  auto &[trace_info, token] = event;
  log::info<2>("token={}"sv, token);
  if (std::empty(token.data.instance_servers)) {
    log::fatal("Unexpected: no instance servers"sv);
  }
  auto &instance_server = token.data.instance_servers[0];
  auto query = fmt::format("?token={}"sv, token.data.token);
  auto private_token = PrivateToken{
      .account = account_.name,
      .uri = instance_server.endpoint,
      .query = query,
      .ping_frequency = instance_server.ping_interval,
  };
  if (private_token.ping_frequency.count() == 0) {
    log::fatal("Unexpected: ping_interval={}"sv, instance_server.ping_interval);
  }
  create_trace_and_dispatch(handler_, trace_info, private_token);
}

// account

void OrderEntryREST::get_account(TraceInfo const &trace_info) {
  profile_.account([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.rest_private.get_account_list;
    auto headers = account_.create_headers(trace_info, method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this](auto &event, [[maybe_unused]] auto &request_id) { get_account_ack(event); };
    (*connection_)(request, callback, "account"sv);
  });
}

void OrderEntryREST::get_account_ack(Trace<web::rest::Response> const &event) {
  profile_.account_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::AccountAck account_ack{body, decode_buffer_};
      if (account_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, account_ack};
        (*this)(event_2);
      } else {
        auto error = map(account_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, account_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
    request_.balances.commit(trace_info);
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::AccountAck> const &event) {
  auto &[trace_info, account_ack] = event;
  log::info<2>("account_ack={}"sv, account_ack);
  log::debug("account_ack={}"sv, account_ack);
  auto &data = account_ack.data;
  auto funds_update = FundsUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .currency = data.currency,
      .margin_mode = {},
      .balance = data.available_balance,
      .hold = NaN,  // ???
      .borrowed = NaN,
      .unrealized_pnl = NaN,
      .external_account = {},
      .update_type = UpdateType::SNAPSHOT,
      .exchange_time_utc = {},  // ???
      .exchange_sequence = {},  // ???
      .sending_time_utc = {},
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, funds_update, true);
}

// positions

void OrderEntryREST::get_positions(TraceInfo const &trace_info) {
  profile_.positions([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.rest_private.get_position_list;
    auto headers = account_.create_headers(trace_info, method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this](auto &event, [[maybe_unused]] auto &request_id) { get_positions_ack(event); };
    (*connection_)(request, callback, "positions");
  });
}

void OrderEntryREST::get_positions_ack(Trace<web::rest::Response> const &event) {
  profile_.positions_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::PositionsAck positions_ack{body, decode_buffer_};
      if (positions_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, positions_ack};
        (*this)(event_2);
      } else {
        auto error = map(positions_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, positions_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
    request_.positions.commit(trace_info);
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::PositionsAck> const &event) {
  auto &[trace_info, positions_ack] = event;
  log::info<2>("positions_ack={}"sv, positions_ack);
  for (auto &item : positions_ack.data) {
    log::debug("item={}"sv, item);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .margin_mode = map(item.margin_mode),
        .external_account = {},
        .long_quantity = std::max(item.current_qty, 0.0),    // XXX FIXME TODO qty ???
        .short_quantity = std::max(-item.current_qty, 0.0),  // XXX FIXME TODO qty ???
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = {},  // ???
        .exchange_sequence = {},
        .sending_time_utc = item.current_timestamp,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, position_update, true);
  }
}

// orders

void OrderEntryREST::get_orders(TraceInfo const &trace_info) {
  profile_.orders([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.rest_private.get_order_list;
    auto query = fmt::format(
        "?status=active"
        "&pageSize={}"sv,
        1000);  // XXX FIXME TODO flags
    auto headers = account_.create_headers(trace_info, method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this](auto &event, [[maybe_unused]] auto &request_id) { get_orders_ack(event); };
    (*connection_)(request, callback, "orders");
  });
}

void OrderEntryREST::get_orders_ack(Trace<web::rest::Response> const &event) {
  profile_.orders_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::OrdersAck orders_ack{body, decode_buffer_};
      if (orders_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, orders_ack};
        (*this)(event_2);
      } else {
        auto error = map(orders_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, orders_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
    request_.open_orders.commit(trace_info);
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::OrdersAck> const &event) {
  auto &[trace_info, orders_ack] = event;
  log::info<2>("orders_ack={}"sv, orders_ack);
  for (auto &item : orders_ack.data.items) {
    log::debug("item={}"sv, item);
    // note! following shouldn't be necessary due to only downloading active orders -- just being safe
    auto order_status = [&]() -> OrderStatus {
      auto result = map(item.status).template get<OrderStatus>();
      if (result == OrderStatus::COMPLETED) {
        if (!utils::is_equal(item.size, item.filled_size)) {
          return OrderStatus::CANCELED;
        }
      }
      return result;
    }();
    auto remaining_quantity = [&]() -> double {
      if (order_status == OrderStatus::CANCELED) {
        return 0.0;
      }
      return item.size - item.filled_size;
    }();
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.side),
        .position_effect = map(item.position_side, item.side),
        .margin_mode = map(item.margin_mode),
        .max_show_quantity = item.visible_size,
        .order_type = map(item.type),
        .time_in_force = map(item.time_in_force),
        .execution_instructions = {},
        .execution_destination = {},
        .create_time_utc = item.created_at,
        .update_time_utc = item.updated_at,
        .external_account = {},
        .external_order_id = item.id,
        .client_order_id = item.client_oid,
        .order_status = order_status,
        .error = {},
        .text = {},
        .quantity = item.size,
        .price = item.price,
        .stop_price = item.stop_price,
        .leverage = NaN,
        .remaining_quantity = remaining_quantity,
        .traded_quantity = item.filled_size,
        .average_traded_price = item.avg_deal_price,
        .last_traded_quantity = NaN,
        .last_traded_price = NaN,
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
  }
}

// order

void OrderEntryREST::get_order(TraceInfo const &trace_info, std::string_view const &client_order_id, std::string_view const &external_order_id) {
  profile_.order([&]() {
    auto helper = [&](std::string_view const &path, std::string_view const &query) {
      auto method = web::http::Method::GET;
      auto headers = account_.create_headers(trace_info, method, path, query, {});
      auto request = web::rest::Request{
          .method = method,
          .path = path,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = {},
          .headers = headers,
          .body = {},
          .quality_of_service = io::QualityOfService::IMMEDIATE,
      };
      auto callback = [this, client_order_id = std::string{client_order_id}](auto &event, [[maybe_unused]] auto &request_id) {
        get_order_ack(event, client_order_id);
      };
      (*connection_)(request, callback, "order");
    };
    if (std::empty(external_order_id)) {
      auto path = shared_.api.rest_private.get_order_by_client_order_id;
      auto query = fmt::format("?clientOid={}"sv, client_order_id);
      helper(path, query);
    } else {
      auto path = fmt::format("{}{}"sv, shared_.api.rest_private.get_order_by_order_id, external_order_id);
      helper(path, {});
    }
  });
}

void OrderEntryREST::get_order_ack(Trace<web::rest::Response> const &event, std::string_view const &client_order_id) {
  profile_.order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::OrderAck order_ack{body, decode_buffer_};
      if (order_ack.code == 100001) {
        auto order_update = server::oms::OrderUpdate{
            .account = account_.name,
            .exchange = {},
            .symbol = {},
            .side = {},
            .position_effect = {},
            .margin_mode = {},
            .max_show_quantity = NaN,
            .order_type = {},
            .time_in_force = {},
            .execution_instructions = {},
            .execution_destination = {},
            .create_time_utc = {},
            .update_time_utc = {},
            .external_account = {},
            .external_order_id = {},
            .client_order_id = client_order_id,
            .order_status = OrderStatus::REJECTED,
            .error = {},
            .text = {},
            .quantity = NaN,
            .price = NaN,
            .stop_price = NaN,
            .leverage = NaN,
            .remaining_quantity = NaN,
            .traded_quantity = NaN,
            .average_traded_price = NaN,
            .last_traded_quantity = NaN,
            .last_traded_price = NaN,
            .last_liquidity = {},
            .routing_id = {},
            .max_request_version = {},
            .max_response_version = {},
            .max_accepted_version = {},
            .update_type = UpdateType::SNAPSHOT,
            .sending_time_utc = {},
        };
        create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
      } else if (order_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, order_ack};
        (*this)(event_2);
      } else {
        auto error = map(order_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, order_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
    //
    if (download_stale_orders_counter_) {
      --download_stale_orders_counter_;
    } else {
      log::fatal("Unexpected"sv);
    }
    log::warn("DEBUG remaining orders: {}"sv, download_stale_orders_counter_);
    if (download_stale_orders_counter_ == 0) {
      request_.stale_orders.commit(trace_info);
    }
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::OrderAck> const &event) {
  auto &[trace_info, order_ack] = event;
  log::info<2>("order_ack={}"sv, order_ack);
  auto &item = order_ack.data;
  // note! following shouldn't be necessary due to only downloading active order -- just being safe
  auto order_status = [&]() -> OrderStatus {
    auto result = map(item.status).template get<OrderStatus>();
    if (result == OrderStatus::COMPLETED) {
      if (!utils::is_equal(item.size, item.filled_size)) {
        return OrderStatus::CANCELED;
      }
    }
    return result;
  }();
  auto remaining_quantity = [&]() -> double {
    if (order_status == OrderStatus::CANCELED) {
      return 0.0;
    }
    return item.size - item.filled_size;
  }();
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = item.symbol,
      .side = map(item.side),
      .position_effect = map(item.position_side, item.side),
      .margin_mode = map(item.margin_mode),
      .max_show_quantity = item.visible_size,
      .order_type = map(item.type),
      .time_in_force = map(item.time_in_force),
      .execution_instructions = {},
      .execution_destination = {},
      .create_time_utc = item.created_at,
      .update_time_utc = item.updated_at,
      .external_account = {},
      .external_order_id = item.id,
      .client_order_id = item.client_oid,
      .order_status = order_status,
      .error = {},
      .text = item.remark,
      .quantity = item.size,
      .price = item.price,
      .stop_price = item.stop_price,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,
      .traded_quantity = item.filled_size,
      .average_traded_price = item.avg_deal_price,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = UpdateType::SNAPSHOT,
      .sending_time_utc = {},
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, order_update, stream_id_);
}

// fills

void OrderEntryREST::get_fills(TraceInfo const &trace_info) {
  profile_.fills([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.rest_private.get_recent_fills;
    auto headers = account_.create_headers(trace_info, method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this](auto &event, [[maybe_unused]] auto &request_id) { get_fills_ack(event); };
    (*connection_)(request, callback, "fills");
  });
}

void OrderEntryREST::get_fills_ack(Trace<web::rest::Response> const &event) {
  profile_.fills_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::FillsAck fills_ack{body, decode_buffer_};
      if (fills_ack.code == SYSTEM_CODE_SUCCESS) {
        Trace event_2{event, fills_ack};
        (*this)(event_2);
      } else {
        auto error = map(fills_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, fills_ack.msg);
      }
    };
    process_response(event, handle_error, handle_success);
    request_.trades.commit(trace_info);
  });
}

// note! we don't get a client_oid
void OrderEntryREST::operator()(Trace<protocol::json::FillsAck> const &event) {
  auto &[trace_info, fills_ack] = event;
  log::info<2>("fills_ack={}"sv, fills_ack);
  for (auto &item : fills_ack.data) {
    log::debug("item={}"sv, item);
    auto fill = Fill{
        .exchange_time_utc = {},
        .external_trade_id = item.trade_id,
        .quantity = item.size,
        .price = item.price,
        .liquidity = map(item.liquidity),
        .commission_amount = NaN,  // XXX FIXME TODO open_fee_pay + close_fee_pay
        .commission_currency = {},
        .base_amount = NaN,
        .quote_amount = NaN,
        .profit_loss_amount = NaN,
    };
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},  // XXX FIXME TODO this is an issue !!!
        .exchange = shared_.settings.exchange,
        .symbol = item.symbol,
        .side = map(item.side),
        .position_effect = map(item.position_side, item.side),
        .margin_mode = map(item.margin_mode),
        .create_time_utc = item.trade_time,
        .update_time_utc = item.trade_time,
        .external_account = {},
        .external_order_id = item.order_id,
        .client_order_id = {},  // XXX FIXME TODO this is an issue !!!
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
        .user = {},
        .strategy_id = {},
    };
    // create_trace_and_dispatch(shared_.dispatcher, trace_info, trade_update, true, order.user_id, order.client_order_id);
  }
}

// create-order

void OrderEntryREST::create_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  profile_.create_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    auto method = web::http::Method::POST;
    auto path = shared_.api.rest_private.add_order;
    auto body = protocol::json::Encoder::add_order(encode_buffer_, create_order, order, ref_data, request_id, shared_.margin_mode);
    log::debug(R"(body="{}")"sv, body);
    auto headers = account_.create_headers(message_info, method, path, {}, body);
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = body,
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id](auto &event, [[maybe_unused]] auto &request_id) {
      auto version = 1;
      create_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request, callback, request_id);
  });
}

void OrderEntryREST::create_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_trace_id = {},
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      log::warn("response={}, user_id={}, order_id={}"sv, response, user_id, order_id);
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::AddOrderAck add_order_ack{body, decode_buffer_};
      log::info<2>("add_order_ack={}"sv, add_order_ack);
      if (add_order_ack.code != SYSTEM_CODE_SUCCESS) {
        auto error = map(add_order_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, add_order_ack.msg);
      } else if (shared_.settings.oms.allow_unsafe_order_ack) {
        Trace event_2{event, add_order_ack};
        (*this)(event_2, user_id, order_id, version);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryREST::operator()(
    Trace<protocol::json::AddOrderAck> const &event, [[maybe_unused]] uint8_t user_id, [[maybe_unused]] uint64_t order_id, [[maybe_unused]] uint32_t version) {
  [[maybe_unused]] auto &[trace_info, add_order_ack] = event;
  // XXX FIXME TODO we only have the order-id, then what?
}

// cancel_order

void OrderEntryREST::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto method = web::http::Method::DELETE;
    auto path = fmt::format("{}/{}"sv, shared_.api.rest_private.cancel_order, order.external_order_id);
    auto headers = account_.create_headers(message_info, method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = io::QualityOfService::IMMEDIATE,
    };
    auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
                        auto &event, [[maybe_unused]] auto &request_id) { cancel_order_ack(event, user_id, order_id, version); };
    (*connection_)(request, callback, request_id);
  });
}

void OrderEntryREST::cancel_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .external_trace_id = {},
          .external_order_id = {},
          .client_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      log::warn("response={}, user_id={}, order_id={}"sv, response, user_id, order_id);
      create_trace_and_dispatch(shared_.dispatcher, trace_info, response, stream_id_, user_id, order_id);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelOrderAck cancel_order_ack{body, decode_buffer_};
      log::info<2>("cancel_order_ack={}"sv, cancel_order_ack);
      if (cancel_order_ack.code != SYSTEM_CODE_SUCCESS) {
        auto error = map(cancel_order_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, cancel_order_ack.msg);
      } else if (std::empty(cancel_order_ack.data.cancelled_order_ids)) {
        auto error = Error::UNKNOWN_ORDER_ID;
        auto msg = std::empty(cancel_order_ack.msg) ? magic_enum::enum_name(error) : cancel_order_ack.msg;
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN_ORDER_ID, msg);
      } else if (shared_.settings.oms.allow_unsafe_order_ack) {
        Trace event_2{event, cancel_order_ack};
        (*this)(event_2, user_id, order_id, version);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryREST::operator()(
    Trace<protocol::json::CancelOrderAck> const &event,
    [[maybe_unused]] uint8_t user_id,
    [[maybe_unused]] uint64_t order_id,
    [[maybe_unused]] uint32_t version) {
  [[maybe_unused]] auto &[trace_info, cancel_order_ack] = event;
}

// cancel_all_orders

void OrderEntryREST::cancel_all_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready()) [[unlikely]] {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_all_orders] = event;
    auto send_ack = [&](auto &symbol) {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = cancel_all_orders.order_id,
          .exchange = cancel_all_orders.exchange,
          .symbol = symbol,
          .side = cancel_all_orders.side,
          .origin = Origin::GATEWAY,
          .request_status = RequestStatus::FORWARDED,
          .error = {},
          .text = {},
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = cancel_all_orders.strategy_id,
      };
      create_trace_and_dispatch(shared_.dispatcher, message_info, cancel_all_orders_ack);
    };
    auto callback = [&](auto &symbol) {
      if (!std::empty(cancel_all_orders.symbol) && symbol != cancel_all_orders.symbol) {
        return;
      }
      auto method = web::http::Method::DELETE;
      auto path = shared_.api.rest_private.cancel_all_orders;
      auto query = fmt::format("?symbol={}"sv, symbol);
      auto headers = account_.create_headers(message_info, method, path, query, {});
      auto request = web::rest::Request{
          .method = method,
          .path = path,
          .query = query,
          .accept = web::http::Accept::APPLICATION_JSON,
          .content_type = {},
          .headers = headers,
          .body = {},
          .quality_of_service = io::QualityOfService::IMMEDIATE,
      };
      auto callback = [this](auto &event, auto &request_id) { cancel_all_orders_ack(event, request_id); };
      (*connection_)(request, callback, request_id);
      send_ack(symbol);
    };
    //
    if (shared_.dispatcher.get_all_order_symbols(callback, account_.name)) {
    } else {
      log::warn("*** NOT POSSIBLE TO CANCEL ALL OPEN ORDERS (NO SYMBOLS) ***"sv);
    }
  });
}

void OrderEntryREST::cancel_all_orders_ack(Trace<web::rest::Response> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders_ack([&]() {
    auto &[trace_info, response] = event;
    auto send_ack = [&](auto origin, auto status, Error error, std::string_view const &text) {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = {},
          .symbol = {},
          .side = {},
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, cancel_all_orders_ack);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      send_ack(origin, status, error, text);
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CancelAllOrdersAck cancel_all_orders_ack{body, decode_buffer_};
      if (cancel_all_orders_ack.code != SYSTEM_CODE_SUCCESS) {
        auto error = map(cancel_all_orders_ack.code).template get<Error>();
        handle_error(Origin::EXCHANGE, RequestStatus::REJECTED, error, cancel_all_orders_ack.msg);
      } else {
        Trace event_2{event, cancel_all_orders_ack};
        (*this)(event_2);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void OrderEntryREST::operator()(Trace<protocol::json::CancelAllOrdersAck> const &event) {
  auto &[trace_info, cancel_all_orders_ack] = event;
  log::info<2>("cancel_all_orders_ack={}"sv, cancel_all_orders_ack);
}

// helpers

void OrderEntryREST::check_request_queue(TraceInfo const &trace_info) {
  request_.private_token.check_pending(trace_info, [this](auto &trace_info) { get_private_token(trace_info); });
  //
  request_.balances.check_pending(trace_info, [this](auto &trace_info) { get_account(trace_info); });
  request_.positions.check_pending(trace_info, [this](auto &trace_info) { get_positions(trace_info); });
  request_.open_orders.check_pending(trace_info, [this](auto &trace_info) { get_orders(trace_info); });
  request_.stale_orders.check_pending(trace_info, [this](auto &trace_info) {
    // XXX FIXME TODO throttle requests
    auto helper = [&](auto &order) { get_order(trace_info, order.client_order_id, order.external_order_id); };
    for (auto &[user_id, order_id] : request_.details_stale_orders) {
      if (shared_.dispatcher.find_order(user_id, order_id, helper)) {
        ++download_stale_orders_counter_;
      } else {
        log::warn("Unexpected: user_id={}, order_id={}"sv, user_id, order_id);
      }
    }
    request_.details_stale_orders.clear();
  });
  request_.trades.check_pending(trace_info, [this](auto &trace_info) { get_fills(trace_info); });
}

void OrderEntryREST::process_response(Trace<web::rest::Response> const &event, auto error_handler, auto success_handler) {
  auto &[trace_info, response] = event;
  try {
    auto [status, category, body] = response.result();
    log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
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
        success_handler(body);
        break;
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

}  // namespace gateway
}  // namespace kucoin_futures
}  // namespace roq
