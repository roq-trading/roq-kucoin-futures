/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/encoder.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

namespace {
auto create_ref_data() {
  auto ref_data = server::oms::RefData{
      .security_type = {},
      .external_security_id = {},
      .multiplier = NaN,
      .quantity = {},
      .price = {},
      .has_tick_size_steps = false,
  };
  ref_data.quantity = {
      .increment = 1.0,
      .precision = Precision::_0,
  };
  ref_data.price = {
      .increment = 0.1,
      .precision = Precision::_1,
  };
  return ref_data;
}
}  // namespace

// REST

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
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::add_order(buffer, create_order, order, ref_data, request_id, {});
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
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::add_order(buffer, create_order, order, ref_data, request_id, {});
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

TEST_CASE("create_ioc", "[json_add_order]") {
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
      .time_in_force = TimeInForce::IOC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 1.0,
      .price = 32000.0,
      .stop_price = NaN,
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::add_order(buffer, create_order, order, ref_data, request_id, {});
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":)"
                 R"("ISOLATED",)"
                 R"("type":"limit",)"
                 R"("timeInForce":"IOC",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1",)"
                 R"("price":"32000.0")"
                 R"(})"sv);
}

TEST_CASE("create_post_only", "[json_add_order]") {
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
      .time_in_force = TimeInForce::IOC,
      .execution_instructions = {ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE},
      .request_template = {},
      .quantity = 1.0,
      .price = 32000.0,
      .stop_price = NaN,
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::add_order(buffer, create_order, order, ref_data, request_id, {});
  CHECK(
      message == R"({)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":)"
                 R"("ISOLATED",)"
                 R"("type":"limit",)"
                 R"("postOnly":true,)"
                 R"("timeInForce":"IOC",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1",)"
                 R"("price":"32000.0")"
                 R"(})"sv);
}

// WSAPI

TEST_CASE("ws_create_market", "[json_encoder]") {
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
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::ws_add_order(buffer, create_order, order, ref_data, request_id, {});
  CHECK(
      message == R"({)"
                 R"("id":"1234",)"
                 R"("op":"futures.order",)"
                 R"("args":{)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":"ISOLATED",)"
                 R"("type":"market",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1")"
                 R"(})"
                 R"(})"sv);
}

TEST_CASE("ws_create_ioc", "[json_encoder]") {
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
      .time_in_force = TimeInForce::IOC,
      .execution_instructions = {},
      .request_template = {},
      .quantity = 1.0,
      .price = NaN,
      .stop_price = NaN,
      .leverage = NaN,
      .routing_id = {},
      .strategy_id = {},
  };
  server::oms::Order order;
  auto ref_data = create_ref_data();
  auto request_id = "1234"sv;
  auto message = json::Encoder::ws_add_order(buffer, create_order, order, ref_data, request_id, {});
  CHECK(
      message == R"({)"
                 R"("id":"1234",)"
                 R"("op":"futures.order",)"
                 R"("args":{)"
                 R"("clientOid":"1234",)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("side":"buy",)"
                 R"("marginMode":"ISOLATED",)"
                 R"("type":"market",)"
                 R"("reduceOnly":false,)"
                 R"("size":"1")"
                 R"(})"
                 R"(})"sv);
}
