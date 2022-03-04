/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/position_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_position_change_example_1", "[json_position_change]") {
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
  CHECK(obj.user_id == "5c32d69203aa676ce4b543c7"sv);
  CHECK(obj.topic == "/contract/position:XBTUSDM"sv);
  CHECK(obj.subject == json::Subject::POSITION_CHANGE);
  auto &data = obj.data;
  CHECK(data.realised_gross_pnl == 0.0_a);
  CHECK(data.cross_mode == false);
  CHECK(data.liquidation_price == 1000000.0_a);
  CHECK(data.pos_loss == 0.0_a);
  CHECK(data.avg_entry_price == 7508.22_a);
  CHECK(data.unrealised_pnl == -0.00014735_a);
  CHECK(data.mark_price == 7947.83_a);
  CHECK(data.pos_margin == 0.00266779_a);
  CHECK(data.risk_limit == 200.0_a);
  CHECK(data.unrealised_cost == 0.00266375_a);
  CHECK(data.pos_comm == 0.00000392_a);
  CHECK(data.pos_maint == 0.00001724_a);
  CHECK(data.pos_cost == 0.00266375_a);
  CHECK(data.maint_margin_req == 0.005_a);
  CHECK(data.bankrupt_price == 1000000.0_a);
  CHECK(data.realised_cost == 0.00000271_a);
  CHECK(data.mark_value == 0.0025164_a);
  CHECK(data.pos_init == 0.00266375_a);
  CHECK(data.realised_pnl == -0.00000253_a);
  CHECK(data.maint_margin == 0.00252044_a);
  CHECK(data.real_leverage == 1.06_a);
  CHECK(data.current_cost == 0.00266375_a);
  CHECK(data.opening_timestamp == 1558433191000ms);
  CHECK(data.current_qty == -20.0_a);
  CHECK(data.delev_percentage == 0.52_a);
  CHECK(data.current_comm == 0.00000271_a);
  CHECK(data.realised_gross_cost == 0.0_a);
  CHECK(data.is_open == true);
  CHECK(data.pos_cross == 1.2e-7_a);
  CHECK(data.current_timestamp == 1558506060394ms);
  CHECK(data.unrealised_roe_pcnt == -0.0553_a);
  CHECK(data.unrealised_pnl_pcnt == -0.0553_a);
  CHECK(data.settle_currency == "XBT"sv);
}

TEST_CASE("json_position_change_example_2", "[json_position_change]") {
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
  CHECK(obj.user_id == "5cd3f1a7b7ebc19ae9558591"sv);
  CHECK(obj.topic == "/contract/position:XBTUSDM"sv);
  CHECK(obj.subject == json::Subject::POSITION_CHANGE);
  auto &data = obj.data;
  CHECK(data.mark_price == 7947.83_a);
  CHECK(data.mark_value == 0.0025164_a);
  CHECK(data.maint_margin == 0.00252044_a);
  CHECK(data.real_leverage == 10.06_a);
  CHECK(data.unrealised_pnl == -0.00014735_a);
  CHECK(data.unrealised_roe_pcnt == -0.0553_a);
  CHECK(data.unrealised_pnl_pcnt == -0.0553_a);
  CHECK(data.delev_percentage == 0.52_a);
  CHECK(data.current_timestamp == 1558087175068ms);
  CHECK(data.settle_currency == "XBT"sv);
}
