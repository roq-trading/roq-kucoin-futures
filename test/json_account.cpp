/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/account.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST(json_account, simple) {
  const auto message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("accountEquity":0.00000000,)"
                       R"("unrealisedPNL":0.00000000,)"
                       R"("marginBalance":0.00000000,)"
                       R"("positionMargin":0.00000000,)"
                       R"("orderMargin":0.00000000,)"
                       R"("frozenFunds":0.00000000,)"
                       R"("availableBalance":0.00000000,)"
                       R"("currency":"XBT")"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Account>(message, buffer_);
  EXPECT_EQ(obj.code, 200000);
  auto &data = obj.data;
  EXPECT_DOUBLE_EQ(data.account_equity, 0.0);
  EXPECT_DOUBLE_EQ(data.unrealised_pnl, 0.0);
  EXPECT_DOUBLE_EQ(data.margin_balance, 0.0);
  EXPECT_DOUBLE_EQ(data.position_margin, 0.0);
  EXPECT_DOUBLE_EQ(data.order_margin, 0.0);
  EXPECT_DOUBLE_EQ(data.frozen_funds, 0.0);
  EXPECT_DOUBLE_EQ(data.available_balance, 0.0);
  EXPECT_EQ(data.currency, "XBT"sv);
}
