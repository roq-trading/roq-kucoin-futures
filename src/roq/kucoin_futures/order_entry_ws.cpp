/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/order_entry_ws.hpp"

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/kucoin_futures/json/encoder.hpp"
#include "roq/kucoin_futures/json/map.hpp"
#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account.name);
}

auto create_query(auto &account) {
  return account.create_ws_query();
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  io::web::URI uri_{settings.ws.uri};
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri_, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
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
  return web::socket::Client::create(handler, context, config, []() -> std::string { return {}; });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

OrderEntryWS::OrderEntryWS(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account)}, query_{create_query(account)},
      connection_{create_connection(*this, shared.settings, context)}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .auth = create_metrics(shared.settings, name_, "auth"sv),
          .welcome = create_metrics(shared.settings, name_, "welcome"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .pong = create_metrics(shared.settings, name_, "pong"sv),
          .add_order_ack = create_metrics(shared.settings, name_, "add_order_ack"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared} {
}

bool OrderEntryWS::ready() const {
  return (*connection_).ready();
}

void OrderEntryWS::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntryWS::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntryWS::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if ((*connection_).ready()) {
    if (welcome_) {
      if (next_ping_ < now) {
        send_ping(now);
      }
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."sv);
    (*connection_).close();
  }
}

void OrderEntryWS::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.auth, metrics::Type::PROFILE)
      .write(profile_.welcome, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.add_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

uint16_t OrderEntryWS::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  auto &[message_info, create_order] = event;
  auto message = json::Encoder::ws_add_order(encode_buffer_, create_order, order, ref_data, request_id, shared_.margin_mode);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t OrderEntryWS::operator()(
    Event<ModifyOrder> const &,
    server::oms::Order const &,
    server::oms::RefData const &,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t OrderEntryWS::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  auto &[message_info, cancel_order] = event;
  auto message = json::Encoder::ws_cancel_order(encode_buffer_, cancel_order, order, ref_data, request_id, previous_request_id);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t OrderEntryWS::operator()(Event<CancelAllOrders> const &, [[maybe_unused]] std::string_view const &request_id) {
  throw server::oms::NotSupported{"not supported"sv};
}

void OrderEntryWS::operator()(web::socket::Client::Connected const &) {
  assert(logon_timeout_.count() == 0);
  auto now = clock::get_system();
  logon_timeout_ = now + shared_.settings.ws.request_timeout;
}

void OrderEntryWS::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  welcome_ = false;
  ping_freq_ = {};
  ready_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
}

void OrderEntryWS::operator()(web::socket::Client::Ready const &) {
  // note! wait for welcome
}

void OrderEntryWS::operator()(web::socket::Client::Close const &) {
}

void OrderEntryWS::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntryWS::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void OrderEntryWS::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void OrderEntryWS::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
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

void OrderEntryWS::send_ping(std::chrono::nanoseconds now) {
  assert(ping_freq_.count() > 0);
  next_ping_ = now + ping_freq_ / 2;
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("op":"ping")"
      R"(}})"sv,
      now.count());
  (*connection_).send_text(message);
}

void OrderEntryWS::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::WSParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void OrderEntryWS::operator()(Trace<json::WSAuth> const &event, std::string_view const &message) {
  profile_.auth([&]() {
    auto &[trace_info, auth] = event;
    log::info<1>("auth={}"sv, auth);
    auto response = account_.create_ws_auth(message);
    (*connection_).send_text(response);
    (*this)(ConnectionStatus::DOWNLOADING);
  });
}

void OrderEntryWS::operator()(Trace<json::WSWelcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("welcome={}"sv, welcome);
    welcome_ = true;
    assert(welcome.ping_interval.count());
    ping_freq_ = welcome.ping_interval;
    (*this)(ConnectionStatus::READY);
  });
}

void OrderEntryWS::operator()(Trace<json::WSError> const &event) {
  profile_.error([&]() {
    auto &[trace_info, error] = event;
    auto helper = [&](auto request_type) {
      auto error_2 = json::guess_error(error.code);
      auto response = server::oms::Response{
          .request_type = request_type,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::REJECTED,
          .error = error_2,
          .text = error.msg,
          .version = {},
          .request_id = error.id,
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      log::warn("response={}"sv, response);
      shared_.update_order(error.id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
    };
    switch (error.op) {
      using enum json::WSOp::type_t;
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
      case PONG:
        break;
      case ADD_ORDER_ACK:
        helper(RequestType::CREATE_ORDER);
        return;  // note!
      case CANCEL_ORDER_ACK:
        helper(RequestType::CANCEL_ORDER);
        return;  // note!
    }
    log::error("error={}"sv, error);
  });
}

void OrderEntryWS::operator()(Trace<json::WSPong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("pong={}"sv, pong);
  });
}

void OrderEntryWS::operator()(Trace<json::WSAddOrderAck> const &event) {
  profile_.add_order_ack([&]() {
    auto &[trace_info, add_order_ack] = event;
    log::info<4>("add_order_ack={}"sv, add_order_ack);
    auto response = server::oms::Response{
        .request_type = RequestType::CREATE_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::ACCEPTED,
        .error = {},
        .text = {},
        .version = {},
        .request_id = add_order_ack.id,
        .external_order_id = add_order_ack.data.order_id,
        .quantity = NaN,
        .price = NaN,
    };
    log::warn("response={}"sv, response);
    shared_.update_order(add_order_ack.data.client_oid, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
  });
}

void OrderEntryWS::operator()(Trace<json::WSCancelOrderAck> const &event) {
  profile_.cancel_order_ack([&]() {
    auto &[trace_info, cancel_order_ack] = event;
    log::info<4>("cancel_order_ack={}"sv, cancel_order_ack);
    auto response = server::oms::Response{
        .request_type = RequestType::CANCEL_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::ACCEPTED,
        .error = {},
        .text = {},
        .version = {},
        .request_id = cancel_order_ack.id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    log::warn("response={}"sv, response);
    shared_.update_order(cancel_order_ack.data.client_oid, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
  });
}

}  // namespace kucoin_futures
}  // namespace roq
