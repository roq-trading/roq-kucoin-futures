/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "ws_parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::WSCancelOrderAck;

TEST_CASE("success", "[json_ws_cancel_order_ack]") {
  auto message = R"({)"
                 R"("id":"1AACM_ObhU0AAgAAAAAA",)"
                 R"("op":"futures.cancel",)"
                 R"("code":"200000",)"
                 R"("data":{)"
                 R"("clientOid":"1wACM_ObhU0AAQAAAAAA")"
                 R"(},)"
                 R"("inTime":1768985019587,)"
                 R"("outTime":1768985019589,)"
                 R"("userRateLimit":{)"
                 R"("remaining":1998,)"
                 R"("limit":2000,)"
                 R"("reset":6080)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.id == "1AACM_ObhU0AAgAAAAAA"sv);
    CHECK(obj.op == json::WSOp::CANCEL_ORDER_ACK);
    CHECK(obj.code == 200000);
    CHECK(obj.data.client_oid == "1wACM_ObhU0AAQAAAAAA"sv);
  };
  WSParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
