/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/position_settlement.h"

using namespace roq;
using namespace roq::kucoin_futures;

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
}
