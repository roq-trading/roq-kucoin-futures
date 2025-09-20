/* Copyright (c) 2017-2025, Hans Erik Thrane */

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
    server::oms::Order const &order,
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
          Decimal{create_order.quantity, order.quantity_precision.precision});
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
          Decimal{create_order.quantity, order.quantity_precision.precision},
          Decimal{create_order.price, order.price_precision.precision});
      break;
    }
  }
  fmt::format_to(std::back_inserter(buffer), R"(}})"sv);
  return buffer;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
