/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/position_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

TEST(json_position_change, example_1) {
  const auto message = R"({ )"
                       R"("userId": "5c32d69203aa676ce4b543c7",)"
                       R"("topic": "/contract/position:XBTUSDM",  )"
                       R"("subject": "position.change", )"
                       R"("data": {)"
                       R"("realisedGrossPnl": 0E-8,)"
                       R"("crossMode": false,)"
                       R"("liquidationPrice": 1000000.0,)"
                       R"("posLoss": 0E-8,)"
                       R"("avgEntryPrice": 7508.22,)"
                       R"("unrealisedPnl": -0.00014735,)"
                       R"("markPrice": 7947.83,)"
                       R"("posMargin": 0.00266779,)"
                       R"("riskLimit": 200,)"
                       R"("unrealisedCost": 0.00266375,)"
                       R"("posComm": 0.00000392,)"
                       R"("posMaint": 0.00001724,)"
                       R"("posCost": 0.00266375,)"
                       R"("maintMarginReq": 0.005,)"
                       R"("bankruptPrice": 1000000.0,)"
                       R"("realisedCost": 0.00000271,)"
                       R"("markValue": 0.00251640,)"
                       R"("posInit": 0.00266375,)"
                       R"("realisedPnl": -0.00000253,)"
                       R"("maintMargin": 0.00252044,)"
                       R"("realLeverage": 1.06,)"
                       R"("currentCost": 0.00266375,)"
                       R"("openingTimestamp": 1558433191000,)"
                       R"("currentQty": -20,)"
                       R"("delevPercentage": 0.52,)"
                       R"("currentComm": 0.00000271,)"
                       R"("realisedGrossCost": 0E-8,)"
                       R"("isOpen": true,)"
                       R"("posCross": 1.2E-7,)"
                       R"("currentTimestamp": 1558506060394,)"
                       R"("unrealisedRoePcnt": -0.0553,)"
                       R"("unrealisedPnlPcnt": -0.0553,)"
                       R"("settleCurrency": "XBT")"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::PositionChange>(message, buffer_);
}

TEST(json_position_change, example_2) {
  const auto message = R"({ )"
                       R"("userId": "5cd3f1a7b7ebc19ae9558591",)"
                       R"("topic": "/contract/position:XBTUSDM",  )"
                       R"("subject": "position.change", )"
                       R"("data": {)"
                       R"("markPrice": 7947.83,)"
                       R"("markValue": 0.00251640,)"
                       R"("maintMargin": 0.00252044,)"
                       R"("realLeverage": 10.06,)"
                       R"("unrealisedPnl": -0.00014735,)"
                       R"("unrealisedRoePcnt": -0.0553,)"
                       R"("unrealisedPnlPcnt": -0.0553,)"
                       R"("delevPercentage": 0.52,)"
                       R"("currentTimestamp": 1558087175068,)"
                       R"("settleCurrency": "XBT")"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::PositionChange>(message, buffer_);
}
