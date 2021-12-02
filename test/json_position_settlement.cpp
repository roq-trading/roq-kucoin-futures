/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/position_settlement.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_position_settlement, example) {
  const auto message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contract/position:XBTUSDM",)"
                       R"("subject": "position.settlement",)"
                       R"("data": {)"
                       R"("fundingTime": 1551770400000,)"
                       R"("qty": 100,)"
                       R"("markPrice": 3610.85,)"
                       R"("fundingRate": -0.002966,)"
                       R"("fundingFee": -296,)"
                       R"("ts": 1547697294838004923,)"
                       R"("settleCurrency": "XBT")"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::PositionSettlement>(message, buffer_);
  EXPECT_EQ(obj.user_id, "xbc453tg732eba53a88ggyt8c"sv);
  EXPECT_EQ(obj.topic, "/contract/position:XBTUSDM"sv);
  EXPECT_EQ(obj.subject, json::Subject::POSITION_SETTLEMENT);
  auto &data = obj.data;
  EXPECT_EQ(data.funding_time, 1551770400000ms);
  EXPECT_DOUBLE_EQ(data.qty, 100.0);
  EXPECT_DOUBLE_EQ(data.mark_price, 3610.85);
  EXPECT_DOUBLE_EQ(data.funding_rate, -0.002966);
  EXPECT_DOUBLE_EQ(data.funding_fee, -296.0);
  EXPECT_EQ(data.ts, 1547697294838004923ns);
  EXPECT_EQ(data.settle_currency, "XBT"sv);
}
