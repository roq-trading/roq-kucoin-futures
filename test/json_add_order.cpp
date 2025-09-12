/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/utils.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_add_order_simple", "[json_add_order]") {
  std::string buffer;
  auto create_order = CreateOrder{
      .account = {},
      .order_id = {},
      .exchange = {},
      .symbol = "XBTUSDTM"sv,
      .side = Side::BUY,
      .position_effect = {},
      .margin_mode = MarginMode::ISOLATED,
      .quantity_type = {},
      .max_show_quantity = NaN,
      .order_type = OrderType::LIMIT,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 1.0,
      .price = 32000.0,
      .stop_price = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  order.quantity_precision = {
      .increment = 1.0,
      .precision = Precision::_0,
  };
  order.price_precision = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  auto request_id = "1234"sv;
  auto message = json::add_order(buffer, create_order, order, request_id);
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("side":"buy",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("type":"limit",)"
                 R"("leverage":1,)"
                 R"("remark":"",)"
                 R"("reduceOnly":false,)"
                 R"("price":32000,)"
                 R"("size":1,)"
                 R"("timeInForce":"GTC")"
                 R"(})"sv);
}
