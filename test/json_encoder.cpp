/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/encoder.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("create_market", "[json_encoder]") {
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
      .order_type = OrderType::MARKET,
      .time_in_force = TimeInForce::GTC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 1.0,
      .price = NaN,
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
  auto message = json::Encoder::add_order(buffer, create_order, order, request_id, {});
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":"ISOLATED",)"
                 R"("type":"market",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1")"
                 R"(})"sv);
}

TEST_CASE("create_limit", "[json_add_order]") {
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
  auto message = json::Encoder::add_order(buffer, create_order, order, request_id, {});
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":"ISOLATED",)"
                 R"("type":"limit",)"
                 R"("timeInForce":"GTC",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1",)"
                 R"("price":"32000.0")"
                 R"(})"sv);
}
