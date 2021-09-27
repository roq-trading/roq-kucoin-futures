/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/snapshot.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::chrono_literals;

TEST(json_snapshot, simple) {
  auto message = R"({)"
                 R"("data":{)"
                 R"("sequence":"1624619965445",)"
                 R"("data":{)"
                 R"("trading":true,)"
                 R"("symbol":"KCS-USDT",)"
                 R"("buy":0.01,)"
                 R"("volValue":308.87557620000000000000,)"
                 R"("baseCurrency":"KCS",)"
                 R"("datetime":1632454260064,)"
                 R"("high":10.76500000000000000000,)"
                 R"("vol":29.18920000000000000000,)"
                 R"("low":10.27700000000000000000,)"
                 R"("takerCoefficient":1.00,)"
                 R"("takerFeeRate":0.001,)"
                 R"("changeRate":0.0047,)"
                 R"("close":10.624,)"
                 R"("lastTradedPrice":10.624,)"
                 R"("makerFeeRate":0.001,)"
                 R"("makerCoefficient":1.00,)"
                 R"("sell":0,)"
                 R"("sort":100,)"
                 R"("market":"USDS",)"
                 R"("quoteCurrency":"USDT",)"
                 R"("symbolCode":"KCS-USDT",)"
                 R"("changePrice":0.050,)"
                 R"("averagePrice":10.42497332,)"
                 R"("board":1,)"
                 R"("mark":0,)"
                 R"("open":10.574)"
                 R"(})"
                 R"(},)"
                 R"("subject":"trade.snapshot",)"
                 R"("topic":"/market/snapshot:KCS-USDT",)"
                 R"("type":"message")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Snapshot>(message, buffer_);
  auto &data = obj.data;
  EXPECT_EQ(data.sequence, 1624619965445);
  auto &data_2 = data.data;
  EXPECT_TRUE(data_2.trading);
  EXPECT_EQ(data_2.symbol, "KCS-USDT"_sv);
  EXPECT_DOUBLE_EQ(data_2.buy, 0.01);
  EXPECT_DOUBLE_EQ(data_2.vol_value, 308.8755762);
  EXPECT_EQ(data_2.base_currency, "KCS"_sv);
  EXPECT_EQ(data_2.datetime, 1632454260064ms);
  EXPECT_DOUBLE_EQ(data_2.high, 10.765);
  EXPECT_DOUBLE_EQ(data_2.vol, 29.1892);
  EXPECT_DOUBLE_EQ(data_2.low, 10.277);
  EXPECT_DOUBLE_EQ(data_2.taker_coefficient, 1.0);
  EXPECT_DOUBLE_EQ(data_2.taker_fee_rate, 0.001);
  EXPECT_DOUBLE_EQ(data_2.change_rate, 0.0047);
  EXPECT_DOUBLE_EQ(data_2.close, 10.624);
  EXPECT_DOUBLE_EQ(data_2.last_traded_price, 10.624);
  EXPECT_DOUBLE_EQ(data_2.maker_fee_rate, 0.001);
  EXPECT_DOUBLE_EQ(data_2.maker_coefficient, 1.0);
  EXPECT_DOUBLE_EQ(data_2.sell, 0.0);
  EXPECT_DOUBLE_EQ(data_2.sort, 100);
  EXPECT_EQ(data_2.market, "USDS"_sv);
  EXPECT_EQ(data_2.quote_currency, "USDT"_sv);
  EXPECT_EQ(data_2.symbol_code, "KCS-USDT"_sv);
  EXPECT_DOUBLE_EQ(data_2.change_price, 0.05);
  EXPECT_DOUBLE_EQ(data_2.average_price, 10.42497332);
  EXPECT_DOUBLE_EQ(data_2.board, 1.0);
  EXPECT_DOUBLE_EQ(data_2.mark, 0.0);
  EXPECT_DOUBLE_EQ(data_2.open, 10.574);
  EXPECT_EQ(obj.subject, json::Subject::TRADE_SNAPSHOT);
  EXPECT_EQ(obj.topic, "/market/snapshot:KCS-USDT"_sv);
  EXPECT_EQ(obj.type, json::Type::MESSAGE);
}
