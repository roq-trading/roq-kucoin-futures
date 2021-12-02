/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/withdraw_hold_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_withdraw_hold_change, example) {
  const auto message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "withdrawHold.change",)"
                       R"("data": {)"
                       R"("withdrawHold": 5923,)"
                       R"("currency":"USDT",)"
                       R"("timestamp": 1553842862614)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::WithdrawHoldChange>(message, buffer_);
  EXPECT_EQ(obj.user_id, "xbc453tg732eba53a88ggyt8c"sv);
  EXPECT_EQ(obj.topic, "/contractAccount/wallet"sv);
  EXPECT_EQ(obj.subject, json::Subject::WITHDRAW_HOLD_CHANGE);
  auto &data = obj.data;
  EXPECT_EQ(data.withdraw_hold, 5923.0);
  EXPECT_EQ(data.currency, "USDT"sv);
  EXPECT_EQ(data.timestamp, 1553842862614ms);
}
