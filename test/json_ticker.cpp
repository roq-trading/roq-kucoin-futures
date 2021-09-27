/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/ticker.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::chrono_literals;

TEST(json_ticker, simple) {
  auto message = R"({)"
                 R"("data":{)"
                 R"("sequence":"1630671492542",)"
                 R"("bestAsk":"0",)"
                 R"("size":"6",)"
                 R"("bestBidSize":"100",)"
                 R"("price":"50",)"
                 R"("time":1632470442803,)"
                 R"("bestAskSize":"0",)"
                 R"("bestBid":"1.12")"
                 R"(},)"
                 R"("subject":"trade.ticker",)"
                 R"("topic":"/market/ticker:XRP-USDT",)"
                 R"("type":"message")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Ticker>(message, buffer_);
  auto &data = obj.data;
  EXPECT_EQ(data.sequence, 1630671492542);
  EXPECT_DOUBLE_EQ(data.best_ask, 0.0);
  EXPECT_DOUBLE_EQ(data.size, 6.0);
  EXPECT_DOUBLE_EQ(data.best_bid_size, 100.0);
  EXPECT_DOUBLE_EQ(data.price, 50.0);
  EXPECT_EQ(data.time, 1632470442803ms);
  EXPECT_DOUBLE_EQ(data.best_ask_size, 0.0);
  EXPECT_DOUBLE_EQ(data.best_bid, 1.12);
  EXPECT_EQ(obj.subject, json::Subject::TRADE_TICKER);
  EXPECT_EQ(obj.topic, "/market/ticker:XRP-USDT"_sv);
  EXPECT_EQ(obj.type, json::Type::MESSAGE);
}
