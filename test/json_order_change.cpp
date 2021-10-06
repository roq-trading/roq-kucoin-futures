/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/order_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

TEST(json_order_change, example) {
  const auto message = R"({)"
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OrderChange>(message, buffer_);
}
