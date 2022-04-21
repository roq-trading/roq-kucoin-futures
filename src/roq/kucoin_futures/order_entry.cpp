/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

namespace {
const auto NAME = "om"sv;
const Mask SUPPORTS{
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
};

const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  core::web::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = core::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return core::web::Client{handler, context, config};
}
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .private_token = create_metrics(name_, "private_token"sv),
          .private_token_ack = create_metrics(name_, "private_token_ack"sv),
          .account = create_metrics(name_, "account"sv),
          .account_ack = create_metrics(name_, "account_ack"sv),
          .positions = create_metrics(name_, "positions"sv),
          .positions_ack = create_metrics(name_, "positions_ack"sv),
          .orders = create_metrics(name_, "orders"sv),
          .orders_ack = create_metrics(name_, "orders_ack"sv),
          .fills = create_metrics(name_, "fills"sv),
          .fills_ack = create_metrics(name_, "fills_ack"sv),
          .create_order = create_metrics(name_, "create_order"sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"sv),
          .cancel_order = create_metrics(name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      security_(security), shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void OrderEntry::operator()(const Event<Start> &) {
  connection_.start();
}

void OrderEntry::operator()(const Event<Stop> &) {
  connection_.stop();
}

void OrderEntry::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.private_token, metrics::PROFILE)
      .write(profile_.private_token_ack, metrics::PROFILE)
      .write(profile_.account, metrics::PROFILE)
      .write(profile_.account_ack, metrics::PROFILE)
      .write(profile_.positions, metrics::PROFILE)
      .write(profile_.positions_ack, metrics::PROFILE)
      .write(profile_.orders, metrics::PROFILE)
      .write(profile_.orders_ack, metrics::PROFILE)
      .write(profile_.fills, metrics::PROFILE)
      .write(profile_.fills_ack, metrics::PROFILE)
      .write(profile_.create_order, metrics::PROFILE)
      .write(profile_.create_order_ack, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_order_ack, metrics::PROFILE)
      .write(profile_.cancel_all_orders, metrics::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const oms::Order &order, const std::string_view &request_id) {
  create_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &,
    const oms::Order &,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  throw oms::NotSupported("not supported"sv);
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    const std::string_view &request_id,
    const std::string_view &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelAllOrders> &event, const std::string_view &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(const core::web::Client::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
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

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case PRIVATE_TOKEN:
      get_private_token();
      return 1;
    case ACCOUNT:
      get_account();
      return 1;
    case POSITIONS:
      get_positions();
      return 1;
    case ORDERS:
      get_orders();
      return 1;
    case FILLS:
      get_fills();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// private-token

void OrderEntry::get_private_token() {
  profile_.private_token([&]() {
    auto method = core::http::Method::POST;
    auto path = "/api/v1/bullet-private"sv;
    auto headers = security_.create_signature_api_v2(method, path, {}, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    auto sequence = download_.sequence();
    connection_(
        "private_token",
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_private_token_ack(event, sequence);
        });
  });
}

void OrderEntry::get_private_token_ack(const Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.private_token_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::PRIVATE_TOKEN;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto token = core::json::Parser::create<json::Token>(body, buffer);
      if (token.code == 200000) {
        Trace event(trace_info, token);
        (*this)(event);
      } else {
        log::warn("token={}"sv, token);
        log::fatal("Unexpected"sv);
      }
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const Trace<json::Token> &event) {
  auto &[trace_info, token] = event;
  log::info<2>("token={}"sv, token);
  if (std::empty(token.data.instance_servers))
    log::fatal("Unexpected: no instance servers"sv);
  auto &instance_server = token.data.instance_servers[0];
  auto query = fmt::format("?token={}"sv, token.data.token);
  PrivateToken const private_token{
      .account = security_.get_account(),
      .uri = instance_server.endpoint,
      .query = query,
      .ping_frequency = instance_server.ping_interval,
  };
  if (private_token.ping_frequency.count() == 0)
    log::fatal("Unexpected: ping_interval={}"sv, instance_server.ping_interval);
  handler_(private_token);
}

// account

void OrderEntry::get_account() {
  profile_.account([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/account-overview"sv;
    auto headers = security_.create_signature_api_v2(method, path, {}, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    auto sequence = download_.sequence();
    connection_(
        "account", request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_account_ack(event, sequence);
        });
  });
}

void OrderEntry::get_account_ack(const Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.account_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::ACCOUNT;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      switch (category) {
        using enum core::http::Category;
        case SUCCESS:  // 2xx
          break;
        case CLIENT_ERROR:  // 4xx
          log::fatal("{}"sv, response.body());
          break;
        default:
          response.expect(core::http::Status::OK);  // throws
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto account = core::json::Parser::create<json::Account>(body, buffer);
      if (account.code == 200000) {
        Trace event(trace_info, account);
        (*this)(event);
      } else {
        log::warn("account={}"sv, account);
        log::fatal("Unexpected"sv);
      }
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const Trace<json::Account> &event) {
  auto &[trace_info, account] = event;
  log::info<2>("account={}"sv, account);
}

// positions

void OrderEntry::get_positions() {
  profile_.positions([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/positions"sv;
    auto headers = security_.create_signature_api_v2(method, path, {}, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    auto sequence = download_.sequence();
    connection_(
        "positions", request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_positions_ack(event, sequence);
        });
  });
}

void OrderEntry::get_positions_ack(const Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.positions_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::POSITIONS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto positions = core::json::Parser::create<json::Positions>(body, buffer);
      if (positions.code == 200000) {
        Trace event(trace_info, positions);
        (*this)(event);
      } else {
        log::warn("positions={}"sv, positions);
        log::fatal("Unexpected"sv);
      }
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const Trace<json::Positions> &event) {
  auto &[trace_info, positions] = event;
  log::info<2>("positions={}"sv, positions);
}

// orders

void OrderEntry::get_orders() {
  profile_.orders([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/orders"sv;
    auto query = "?status=active"sv;
    auto headers = security_.create_signature_api_v2(method, path, query, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    auto sequence = download_.sequence();
    connection_(
        "orders", request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_orders_ack(event, sequence);
        });
  });
}

void OrderEntry::get_orders_ack(const Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.orders_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::ORDERS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto orders = core::json::Parser::create<json::Orders>(body, buffer);
      if (orders.code == 200000) {
        Trace event(trace_info, orders);
        (*this)(event);
      } else {
        log::warn("orders={}"sv, orders);
        log::fatal("Unexpected"sv);
      }
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const Trace<json::Orders> &event) {
  auto &[trace_info, orders] = event;
  log::info<2>("orders={}"sv, orders);
}

// fills

void OrderEntry::get_fills() {
  profile_.fills([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v1/fills"sv;
    auto headers = security_.create_signature_api_v2(method, path, {}, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    auto sequence = download_.sequence();
    connection_(
        "fills", request, [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_fills_ack(event, sequence);
        });
  });
}

void OrderEntry::get_fills_ack(const Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.fills_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = OrderEntryState::FILLS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto fills = core::json::Parser::create<json::Fills>(body, buffer);
      if (fills.code == 200000) {
        Trace event(trace_info, fills);
        (*this)(event);
      } else {
        log::warn("fills={}"sv, fills);
        log::fatal("Unexpected"sv);
      }
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void OrderEntry::operator()(const Trace<json::Fills> &event) {
  auto &[trace_info, fills] = event;
  log::info<2>("fills={}"sv, fills);
}

// create-order

void OrderEntry::create_order(
    const Event<CreateOrder> &event, const oms::Order &, const std::string_view &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v1/orders"sv;
    auto side = json::map(create_order.side).as_raw_text();
    auto type = "limit"sv;  // limit or market
    auto leverage = ""sv;
    auto remark = ""sv;
    auto reduce_only =
        create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
    auto time_in_force = "GTC"sv;  // GTC, IOC
    auto body = fmt::format(
        R"({{)"
        R"("clientOid":"{}",)"
        R"("side":"{}",)"
        R"("symbol":"{}",)"
        R"("type":"{}",)"
        R"("leverage":"{}",)"
        R"("remark":"{}",)"
        R"("reduceOnly":{},)"
        R"("price":{},)"
        R"("size":{},)"
        R"("timeInForce":"{}")"
        R"(}})"sv,
        request_id,
        side,
        create_order.symbol,
        type,
        leverage,
        remark,
        reduce_only,
        create_order.price,
        create_order.quantity,
        time_in_force);
    log::debug(R"(body="{}")"sv, body);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = {},
        .body = body,
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    connection_(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](
            [[maybe_unused]] auto &request_id, auto &response) {
          uint32_t version = 1;
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          create_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::create_order_ack(
    const Trace<core::web::Response> &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    auto &[trace_info, response] = event;
    log::debug("user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum core::http::Category;
        case SUCCESS: {  // 2xx
          // XXX HANS PARSE
          break;
        }
        case CLIENT_ERROR: {
          std::string_view text;
          // XXX HANS PARSE
          oms::Response response{
              .type = RequestType::CREATE_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          if (shared_.update_order(
                  user_id,
                  order_id,
                  stream_id_,
                  trace_info,
                  response,
                  []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
          }
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CREATE_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
      }
    }
  });
}

// cancel-order

void OrderEntry::cancel_order(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::DELETE;
    auto path = fmt::format("/api/v1/orders/{}"sv, order.external_order_id);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
    };
    connection_(
        request_id,
        request,
        [this,
         user_id = message_info.source,
         order_id = cancel_order.order_id,
         version = cancel_order.version]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          cancel_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::cancel_order_ack(
    const Trace<core::web::Response> &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, response] = event;
    log::debug("user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum core::http::Category;
        case SUCCESS: {  // 2xx
          core::json::Buffer buffer(decode_buffer_);
          // XXX HANS PARSE
          break;
        }
        case CLIENT_ERROR: {  // 4xx
          std::string_view text;
          // XXX HANS PARSE
          oms::Response response{
              .type = RequestType::CANCEL_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          if (shared_.update_order(
                  user_id,
                  order_id,
                  stream_id_,
                  trace_info,
                  response,
                  []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn(
                "Did not find order: user_id={}, order_id={}, version={}"sv,
                user_id,
                order_id,
                version);
          }
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CANCEL_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn(
            "Did not find order: user_id={}, order_id={}, version={}"sv,
            user_id,
            order_id,
            version);
      }
    }
  });
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(
    const Event<CancelAllOrders> &event, [[maybe_unused]] const std::string_view &request_id) {
  profile_.cancel_all_orders([&]() {
    if (ready()) {
      auto method = core::http::Method::DELETE;
      auto path = "/api/v1/orders"sv;
      core::web::Request request{
          .method = method,
          .path = path,
          .query = {},
          .accept = core::http::Accept::JSON,
          .content_type = {},
          .headers = {},
          .body = {},
          .quality_of_service = core::web::QualityOfService::IMMEDIATE,
      };
      connection_(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
        auto trace_info = server::create_trace_info();
        Trace event(trace_info, response);
        cancel_all_orders_ack(event);
      });
    } else {
      auto &[message_info, cancel_all_orders] = event;
      log::warn(
          R"(*** NOT CONNECTED! UNABLE TO CANCEL ALL ORDERS FOR ACCOUNT="{}")"sv,
          cancel_all_orders.account);
    }
  });
}

void OrderEntry::cancel_all_orders_ack(const Trace<core::web::Response> &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum core::http::Category;
        case SUCCESS: {  // 2xx
          core::json::Buffer buffer(decode_buffer_);
          // XXX HANS PARSE
          break;
        }
        case CLIENT_ERROR: {  // 4xx
          // XXX HANS PARSE
          // note! this event does not require a response
          break;
        }
        default:
          response.expect(core::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // note! this event does not require a response
    }
  });
}

}  // namespace kucoin_futures
}  // namespace roq
