/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/account.h"

using namespace roq;
using namespace roq::kucoin_futures;

TEST(json_position_item_item, unlisted) {
  const auto message = R"({")"
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
}
