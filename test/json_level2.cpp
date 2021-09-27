/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/level2.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::chrono_literals;

TEST(json_level2, simple) {
  auto message = R"({)"
                 R"("data":{)"
                 R"("sequenceStart":1624609231622,)"
                 R"("symbol":"BTC-USDT",)"
                 R"("changes":{)"
                 R"("asks":[],)"
                 R"("bids":[["0","0","1624609231622"]])"
                 R"(},)"
                 R"("sequenceEnd":1624609231622)"
                 R"(},)"
                 R"("subject":"trade.l2update",)"
                 R"("topic":"/market/level2:BTC-USDT",)"
                 R"("type":"message")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Level2>(message, buffer_);
  auto &data = obj.data;
  EXPECT_EQ(data.sequence_start, 1624609231622);
  EXPECT_EQ(data.symbol, "BTC-USDT"_sv);
  auto &changes = data.changes;
  EXPECT_TRUE(std::empty(changes.asks));
  ASSERT_EQ(std::size(changes.bids), 1);
  auto &bid_0 = changes.bids[0];
  EXPECT_DOUBLE_EQ(bid_0.price, 0.0);
  EXPECT_DOUBLE_EQ(bid_0.size, 0.0);
  EXPECT_EQ(bid_0.sequence, 1624609231622);
  EXPECT_EQ(data.sequence_end, 1624609231622);
  EXPECT_EQ(obj.subject, json::Subject::TRADE_L2UPDATE);
  EXPECT_EQ(obj.topic, "/market/level2:BTC-USDT"_sv);
  EXPECT_EQ(obj.type, json::Type::MESSAGE);
}
