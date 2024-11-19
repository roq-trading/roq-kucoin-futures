/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/order_change.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_change_example", "[json_order_change]") {
  auto const message = R"({)"
                       R"("type": "message",)"
                       R"("topic": "/contractMarket/tradeOrders",)"
                       R"("subject": "orderChange",)"
                       R"("channelType": "private",)"
                       R"("data": {)"
                       R"("orderId": "5cdfc138b21023a909e5ad55",)"
                       R"("symbol": "XBTUSDM",)"
                       R"("type": "match",)"
                       R"("status": "open",)"
                       R"("matchSize": "",)"
                       R"("matchPrice": "",)"
                       R"("orderType": "limit",)"
                       R"("side": "buy",)"
                       R"("price": "3600",)"
                       R"("size": "20000",)"
                       R"("remainSize": "20001",)"
                       R"("filledSize":"20000",)"
                       R"("canceledSize": "0",)"
                       R"("tradeId": "5ce24c16b210233c36eexxxx",)"
                       R"("clientOid": "5ce24c16b210233c36ee321d",)"
                       R"("orderTime": 1545914149935808589,)"
                       R"("oldSize": "15000",)"
                       R"("liquidity": "maker",)"
                       R"("ts": 1545914149935808589)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  json::OrderChange obj{message, buffer};
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
  CHECK(obj.subject == json::Subject::ORDER_CHANGE);
  CHECK(obj.channel_type == "private"sv);
  auto &data = obj.data;
  CHECK(data.order_id == "5cdfc138b21023a909e5ad55"sv);
  CHECK(data.symbol == "XBTUSDM"sv);
  CHECK(data.type == "match"sv);
  CHECK(data.status == "open"sv);
  CHECK(std::isnan(data.match_size) == true);
  CHECK(std::isnan(data.match_price) == true);
  CHECK(data.order_type == "limit"sv);
  CHECK(data.side == "buy"sv);
  CHECK(data.price == 3600.0_a);
  CHECK(data.size == 20000.0_a);
  CHECK(data.remain_size == 20001.0_a);
  CHECK(data.filled_size == 20000.0_a);
  CHECK(data.canceled_size == 0.0_a);
  CHECK(data.trade_id == "5ce24c16b210233c36eexxxx"sv);
  CHECK(data.client_oid == "5ce24c16b210233c36ee321d"sv);
  CHECK(data.order_time == 1545914149935808589ns);
  CHECK(data.old_size == 15000.0_a);
  CHECK(data.liquidity == "maker"sv);
  CHECK(data.ts == 1545914149935808589ns);
}
