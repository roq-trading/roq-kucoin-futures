/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/json/encoder.hpp"

#include "roq/decimal.hpp"

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

// missing: leverage
std::string_view Encoder::add_order(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    roq::MarginMode default_margin_mode) {
  buffer.clear();
  auto side = map(create_order.side).template get<json::Side>();
  auto type = map(create_order.order_type).template get<json::OrderType>();
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  auto margin_mode = [&]() {
    auto margin_mode = create_order.margin_mode != roq::MarginMode{} ? create_order.margin_mode : default_margin_mode;
    return map(margin_mode).template get<json::MarginMode>();
  }();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("clientOid":"{}",)"
      R"("symbol":"{}",)"
      R"("side":"{}",)"
      R"("marginMode":"{}",)"
      R"("type":"{}")"sv,
      request_id,
      create_order.symbol,
      side.as_raw_text(),
      margin_mode.as_raw_text(),
      type.as_raw_text());
  if (create_order.execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
    fmt::format_to(std::back_inserter(buffer), R"(,"postOnly":true)"sv);
  }
  switch (create_order.order_type) {
    using enum roq::OrderType;
    case UNDEFINED:
      assert(false);
      break;
    case MARKET:
      fmt::format_to(
          std::back_inserter(buffer),
          R"(,"reduceOnly":{})"
          R"(,"size":"{}")"sv,
          reduce_only,
          Decimal{create_order.quantity, ref_data.quantity.precision});
      break;
    case LIMIT: {
      auto time_in_force = map(create_order.time_in_force).template get<json::TimeInForce>();
      fmt::format_to(
          std::back_inserter(buffer),
          R"(,"timeInForce":"{}")"
          R"(,"reduceOnly":{})"
          R"(,"size":"{}")"
          R"(,"price":"{}")"sv,
          time_in_force.as_raw_text(),
          reduce_only,
          Decimal{create_order.quantity, ref_data.quantity.precision},
          Decimal{create_order.price, ref_data.price.precision});
      break;
    }
  }
  fmt::format_to(std::back_inserter(buffer), R"(}})"sv);
  return buffer;
}

// WSAPI

std::string_view Encoder::ws_add_order(
    std::string &buffer,
    CreateOrder const &create_order,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    roq::MarginMode margin_mode) {
  auto args = add_order(buffer, create_order, order, ref_data, request_id, margin_mode);
  // note! just overwrite
  buffer = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("op":"futures.order",)"
      R"("args":{})"
      R"(}})"sv,
      request_id,
      args);
  return buffer;
}

std::string_view Encoder::ws_cancel_order(
    std::string &buffer,
    CancelOrder const &,
    server::oms::Order const &order,
    server::oms::RefData const &,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":"{}",)"
      R"("op":"futures.cancel",)"
      R"("args":{{)"
      R"("symbol":"{}",)"
      R"("clientOid":"{}")"
      R"(}})"
      R"(}})"sv,
      request_id,
      order.symbol,
      order.client_order_id);
  return buffer;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
