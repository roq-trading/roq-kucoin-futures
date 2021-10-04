/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/order_entry.h"

#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/kucoin_futures/flags.h"

#include "roq/kucoin_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {

namespace {
static const auto NAME = "om"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::FUNDS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"_sv, stream_id_, NAME, security.get_account())),
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
          .private_token = create_metrics(name_, "private_token"_sv),
          .create_order = create_metrics(name_, "create_order"_sv),
          .cancel_order = create_metrics(name_, "cancel_order"_sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"_sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"_sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"_sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
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
      .write(profile_.create_order, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_all_orders, metrics::PROFILE)
      .write(profile_.create_order_ack, metrics::PROFILE)
      .write(profile_.cancel_order_ack, metrics::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const std::string_view &request_id) {
  throw oms::NotSupportedException();
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v1/orders"_sv;
    auto side = json::map(create_order.side).as_raw_text();
    auto type = "limit"_sv;  // limit or market
    auto leverage = ""_sv;
    auto remark = ""_sv;
    auto reduce_only = create_order.execution_instruction == ExecutionInstruction::DO_NOT_INCREASE;
    auto time_in_force = "GTC"_sv;  // GTC, IOC
    // XXX use encode buffer
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
        R"(}})"_sv,
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
    log::debug(R"(body="{}")"_sv, body);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = {},
        .body = body,
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
        .rate_limit_weight = 1,
    };
    connection_(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](
            [[maybe_unused]] auto &request_id, auto &response) {
          profile_.create_order_ack([&]() { create_order_ack(response, user_id, order_id); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &,
    const oms::Order &,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  throw oms::NotSupportedException();
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  throw oms::NotSupportedException();
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::DELETE;
    auto path = fmt::format("/api/v1/orders/{}"_sv, order.external_order_id);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = {},
        .body = {},
        .quality_of_service = core::web::QualityOfService::IMMEDIATE,
        .rate_limit_weight = 1,
    };
    connection_(
        request_id,
        request,
        [this,
         user_id = message_info.source,
         order_id = cancel_order.order_id,
         version = cancel_order.version]([[maybe_unused]] auto &request_id, auto &response) {
          profile_.cancel_order_ack(
              [&]() { cancel_order_ack(response, user_id, order_id, version); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelAllOrders> &event, [[maybe_unused]] const std::string_view &request_id) {
  profile_.cancel_all_orders([&]() {
    if (ready()) {
      auto method = core::http::Method::DELETE;
      auto path = "/api/v1/orders"_sv;
      core::web::Request request{
          .method = method,
          .path = path,
          .query = {},
          .accept = core::http::Accept::JSON,
          .content_type = core::http::ContentType::JSON,
          .headers = {},
          .body = {},
          .quality_of_service = core::web::QualityOfService::IMMEDIATE,
          .rate_limit_weight = 1,
      };
      connection_(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
        profile_.cancel_all_orders_ack([&]() { cancel_all_orders_ack(response); });
      });
    } else {
      auto &[message_info, cancel_all_orders] = event;
      log::warn(
          R"(*** NOT CONNECTED! UNABLE TO CANCEL ALL ORDERS FOR ACCOUNT="{}")"_sv,
          cancel_all_orders.account);
    }
  });
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
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    case OrderEntryState::UNDEFINED:
      assert(false);
      break;
    case OrderEntryState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

void OrderEntry::create_order_ack(
    const core::web::Response &response, const uint8_t user_id, const uint32_t order_id) {
  log::debug("user_id={}, order_id={}"_sv, user_id, order_id);
  server::TraceInfo trace_info;
  uint8_t version = 1;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        auto body = response.body();
        // XXX HANS PARSE
        break;
      }
      case core::http::Category::CLIENT_ERROR: {
        std::string_view text;
        auto body = response.body();
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
          log::warn("Did not find order: user_id={}, order_id={}"_sv, user_id, order_id);
        }
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (core::NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
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
            user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {
            })) {
    } else {
      log::warn("Did not find order: user_id={}, order_id={}"_sv, user_id, order_id);
    }
  }
}

void OrderEntry::cancel_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  log::debug("user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
  server::TraceInfo trace_info;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        core::json::Buffer buffer(decode_buffer_);
        // XXX HANS PARSE
        break;
      }
      case core::http::Category::CLIENT_ERROR: {  // 4xx
        std::string_view text;
        auto body = response.body();
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
              "Did not find order: user_id={}, order_id={}, version={}"_sv,
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
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
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
            user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {
            })) {
    } else {
      log::warn(
          "Did not find order: user_id={}, order_id={}, version={}"_sv, user_id, order_id, version);
    }
  }
}

void OrderEntry::cancel_all_orders_ack(const core::web::Response &response) {
  server::TraceInfo trace_info;
  try {
    auto category = core::http::to_category(response.raw_status());
    switch (category) {
      case core::http::Category::SUCCESS: {  // 2xx
        core::json::Buffer buffer(decode_buffer_);
        // XXX HANS PARSE
        break;
      }
      case core::http::Category::CLIENT_ERROR: {  // 4xx
        auto body = response.body();
        // XXX HANS PARSE
        // note! this event does not require a response
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (core::NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    // note! this event does not require a response
  }
}

}  // namespace kucoin_futures
}  // namespace roq
