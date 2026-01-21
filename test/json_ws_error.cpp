/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "ws_parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::WSError;

TEST_CASE("create_order", "[json_ws_parser]") {
  auto message = R"({)"
                 R"("id":"_QACfSLXfE0AAQAAAAAA",)"
                 R"("op":"futures.order",)"
                 R"("code":"330008",)"
                 R"("msg":"Order quantity is too high, insufficient available margin.",)"
                 R"("inTime":1768970303228,)"
                 R"("outTime":1768970303239)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "_QACfSLXfE0AAQAAAAAA"sv);
    CHECK(obj.op == json::WSOp::ADD_ORDER_ACK);
    CHECK(obj.code == 330008);
  };
  WSParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("cancel_order", "[json_ws_parser]") {
  auto message = R"({)"
                 R"("id":"FQACOz1kfU0AAgAAAAAA",)"
                 R"("op":"futures.cancel",)"
                 R"("code":"100004",)"
                 R"("msg":"The order cannot be canceled.",)"
                 R"("inTime":1768971229148,)"
                 R"("outTime":1768971229150,)"
                 R"("userRateLimit":{)"
                 R"("remaining":1998,)"
                 R"("limit":2000,)"
                 R"("reset":24617)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "FQACOz1kfU0AAgAAAAAA"sv);
    CHECK(obj.op == json::WSOp::CANCEL_ORDER_ACK);
    CHECK(obj.code == 100004);
  };
  WSParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
