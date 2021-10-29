/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/position_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

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
  EXPECT_EQ(obj.user_id, "5c32d69203aa676ce4b543c7"sv);
  EXPECT_EQ(obj.topic, "/contract/position:XBTUSDM"sv);
  EXPECT_EQ(obj.subject, json::Subject::POSITION_CHANGE);
  auto &data = obj.data;
  EXPECT_DOUBLE_EQ(data.realised_gross_pnl, 0.0);
  EXPECT_EQ(data.cross_mode, false);
  EXPECT_DOUBLE_EQ(data.liquidation_price, 1000000.0);
  EXPECT_DOUBLE_EQ(data.pos_loss, 0.0);
  EXPECT_DOUBLE_EQ(data.avg_entry_price, 7508.22);
  EXPECT_DOUBLE_EQ(data.unrealised_pnl, -0.00014735);
  EXPECT_DOUBLE_EQ(data.mark_price, 7947.83);
  EXPECT_DOUBLE_EQ(data.pos_margin, 0.00266779);
  EXPECT_DOUBLE_EQ(data.risk_limit, 200.0);
  EXPECT_DOUBLE_EQ(data.unrealised_cost, 0.00266375);
  EXPECT_DOUBLE_EQ(data.pos_comm, 0.00000392);
  EXPECT_DOUBLE_EQ(data.pos_maint, 0.00001724);
  EXPECT_DOUBLE_EQ(data.pos_cost, 0.00266375);
  EXPECT_DOUBLE_EQ(data.maint_margin_req, 0.005);
  EXPECT_DOUBLE_EQ(data.bankrupt_price, 1000000.0);
  EXPECT_DOUBLE_EQ(data.realised_cost, 0.00000271);
  EXPECT_DOUBLE_EQ(data.mark_value, 0.0025164);
  EXPECT_DOUBLE_EQ(data.pos_init, 0.00266375);
  EXPECT_DOUBLE_EQ(data.realised_pnl, -0.00000253);
  EXPECT_DOUBLE_EQ(data.maint_margin, 0.00252044);
  EXPECT_DOUBLE_EQ(data.real_leverage, 1.06);
  EXPECT_DOUBLE_EQ(data.current_cost, 0.00266375);
  EXPECT_EQ(data.opening_timestamp, 1558433191000ms);
  EXPECT_DOUBLE_EQ(data.current_qty, -20.0);
  EXPECT_DOUBLE_EQ(data.delev_percentage, 0.52);
  EXPECT_DOUBLE_EQ(data.current_comm, 0.00000271);
  EXPECT_DOUBLE_EQ(data.realised_gross_cost, 0.0);
  EXPECT_EQ(data.is_open, true);
  EXPECT_DOUBLE_EQ(data.pos_cross, 1.2e-7);
  EXPECT_EQ(data.current_timestamp, 1558506060394ms);
  EXPECT_DOUBLE_EQ(data.unrealised_roe_pcnt, -0.0553);
  EXPECT_DOUBLE_EQ(data.unrealised_pnl_pcnt, -0.0553);
  EXPECT_EQ(data.settle_currency, "XBT"sv);
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
  EXPECT_EQ(obj.user_id, "5cd3f1a7b7ebc19ae9558591"sv);
  EXPECT_EQ(obj.topic, "/contract/position:XBTUSDM"sv);
  EXPECT_EQ(obj.subject, json::Subject::POSITION_CHANGE);
  auto &data = obj.data;
  EXPECT_DOUBLE_EQ(data.mark_price, 7947.83);
  EXPECT_DOUBLE_EQ(data.mark_value, 0.0025164);
  EXPECT_DOUBLE_EQ(data.maint_margin, 0.00252044);
  EXPECT_DOUBLE_EQ(data.real_leverage, 10.06);
  EXPECT_DOUBLE_EQ(data.unrealised_pnl, -0.00014735);
  EXPECT_DOUBLE_EQ(data.unrealised_roe_pcnt, -0.0553);
  EXPECT_DOUBLE_EQ(data.unrealised_pnl_pcnt, -0.0553);
  EXPECT_DOUBLE_EQ(data.delev_percentage, 0.52);
  EXPECT_EQ(data.current_timestamp, 1558087175068ms);
  EXPECT_EQ(data.settle_currency, "XBT"sv);
}
