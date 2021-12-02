/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/order_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

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
  EXPECT_EQ(obj.type, json::Type::MESSAGE);
  EXPECT_EQ(obj.topic, "/contractMarket/tradeOrders"sv);
  EXPECT_EQ(obj.subject, json::Subject::ORDER_CHANGE);
  EXPECT_EQ(obj.channel_type, "private"sv);
  auto &data = obj.data;
  EXPECT_EQ(data.order_id, "5cdfc138b21023a909e5ad55"sv);
  EXPECT_EQ(data.symbol, "XBTUSDM"sv);
  EXPECT_EQ(data.type, "match"sv);
  EXPECT_EQ(data.status, "open"sv);
  EXPECT_EQ(std::isnan(data.match_size), true);
  EXPECT_EQ(std::isnan(data.match_price), true);
  EXPECT_EQ(data.order_type, "limit"sv);
  EXPECT_EQ(data.side, "buy"sv);
  EXPECT_DOUBLE_EQ(data.price, 3600.0);
  EXPECT_DOUBLE_EQ(data.size, 20000.0);
  EXPECT_DOUBLE_EQ(data.remain_size, 20001.0);
  EXPECT_DOUBLE_EQ(data.filled_size, 20000.0);
  EXPECT_DOUBLE_EQ(data.canceled_size, 0.0);
  EXPECT_EQ(data.trade_id, "5ce24c16b210233c36eexxxx"sv);
  EXPECT_EQ(data.client_oid, "5ce24c16b210233c36ee321d"sv);
  EXPECT_EQ(data.order_time, 1545914149935808589ns);
  EXPECT_DOUBLE_EQ(data.old_size, 15000.0);
  EXPECT_EQ(data.liquidity, "maker"sv);
  EXPECT_EQ(data.ts, 1545914149935808589ns);
}
