/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "ws_parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::WSAddOrderAck;

TEST_CASE("success", "[json_ws_add_order_ack]") {
  auto message = R"({)"
                 R"("id":"IAACgxI9hU0AAQAAAAAA",)"
                 R"("op":"futures.order",)"
                 R"("code":"200000",)"
                 R"("data":{)"
                 R"("orderId":"403371203999346688",)"
                 R"("clientOid":"IAACgxI9hU0AAQAAAAAA")"
                 R"(},)"
                 R"("inTime":1768984391308,)"
                 R"("outTime":1768984391377)"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "IAACgxI9hU0AAQAAAAAA"sv);
    CHECK(obj.op == json::WSOp::ADD_ORDER_ACK);
    CHECK(obj.code == 200000);
    CHECK(obj.data.order_id == "403371203999346688"sv);
    CHECK(obj.data.client_oid == "IAACgxI9hU0AAQAAAAAA"sv);
  };
  WSParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
