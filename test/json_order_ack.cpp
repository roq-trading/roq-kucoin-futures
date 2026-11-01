/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/protocol/json/order_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = protocol::json::OrderAck;

TEST_CASE("does_not_exist", "[json_order_ack]") {
  auto const message = R"({)"
                       R"("msg":"error.getOrder.orderNotExist",)"
                       R"("code":"100001")"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.msg == "error.getOrder.orderNotExist"sv);
    CHECK(obj.code == 100001);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("canceled", "[json_order_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("id":"506354345504202753",)"
                       R"("symbol":"XBTUSDTM",)"
                       R"("type":"limit",)"
                       R"("side":"buy",)"
                       R"("price":"50000.0",)"
                       R"("size":1,)"
                       R"("value":"50.0",)"
                       R"("dealValue":"0",)"
                       R"("dealSize":0,)"
                       R"("stp":"",)"
                       R"("stop":"",)"
                       R"("stopPriceType":"",)"
                       R"("stopTriggered":false,)"
                       R"("stopPrice":null,)"
                       R"("timeInForce":"GTC",)"
                       R"("postOnly":false,)"
                       R"("hidden":false,)"
                       R"("iceberg":false,)"
                       R"("leverage":"50.0",)"
                       R"("forceHold":false,)"
                       R"("closeOrder":false,)"
                       R"("visibleSize":0,)"
                       R"("clientOid":"nQACS6EFsIYAAQAAAAAA",)"
                       R"("remark":"Order manually canceled.",)"
                       R"("tags":"",)"
                       R"("isActive":false,)"
                       R"("cancelExist":true,)"
                       R"("createdAt":1793537485576,)"
                       R"("updatedAt":1793537490025,)"
                       R"("endAt":1793537490025,)"
                       R"("orderTime":1793537485576977572,)"
                       R"("settleCurrency":"USDT",)"
                       R"("marginMode":"CROSS",)"
                       R"("avgDealPrice":"0",)"
                       R"("positionSide":"BOTH",)"
                       R"("filledSize":0,)"
                       R"("filledValue":"0",)"
                       R"("status":"done",)"
                       R"("reduceOnly":false)"
                       R"(})"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(std::empty(obj.msg));
    CHECK(obj.code == 200000);
    CHECK(obj.data.id == "506354345504202753"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
