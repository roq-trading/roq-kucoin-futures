/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/orders.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("empty", "[json_orders]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("currentPage":1,)"
                       R"("pageSize":1000,)"
                       R"("totalNum":0,)"
                       R"("totalPage":0,)"
                       R"("items":[])"
                       R"(})"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Orders obj{message, buffer};
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.current_page == 1);
  CHECK(data.page_size == 1000);
  CHECK(data.total_num == 0);
  CHECK(data.total_page == 0);
  auto &items = data.items;
  CHECK(std::size(items) == 0);
}

TEST_CASE("simple", "[json_orders]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("currentPage":1,)"
                       R"("pageSize":1000,)"
                       R"("totalNum":1,)"
                       R"("totalPage":1,)"
                       R"("items":[{)"
                       R"("id":"358890922685038592",)"
                       R"("symbol":"XBTUSDTM",)"
                       R"("type":"limit",)"
                       R"("side":"buy",)"
                       R"("price":"32000",)"
                       R"("size":1,)"
                       R"("value":"32",)"
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
                       R"("leverage":"3",)"
                       R"("forceHold":false,)"
                       R"("closeOrder":false,)"
                       R"("visibleSize":0,)"
                       R"("clientOid":"cAACI8I11DQAAQAAAAAA",)"
                       R"("remark":null,)"
                       R"("tags":"",)"
                       R"("isActive":true,)"
                       R"("cancelExist":false,)"
                       R"("createdAt":1758379465848,)"
                       R"("updatedAt":1758379465848,)"
                       R"("endAt":null,)"
                       R"("orderTime":1758379465838574219,)"
                       R"("settleCurrency":"USDT",)"
                       R"("marginMode":"CROSS",)"
                       R"("avgDealPrice":"0",)"
                       R"("positionSide":"BOTH",)"
                       R"("filledValue":"0",)"
                       R"("status":"open",)"
                       R"("filledSize":0,)"
                       R"("reduceOnly":false)"
                       R"(})"
                       R"(])"
                       R"(})"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Orders obj{message, buffer};
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.current_page == 1);
  CHECK(data.page_size == 1000);
  CHECK(data.total_num == 1);
  CHECK(data.total_page == 1);
  auto &items = data.items;
  REQUIRE(std::size(items) == 1);
  CHECK(items[0].id == "358890922685038592"sv);
}
