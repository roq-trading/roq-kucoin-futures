/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/positions.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::chrono_literals;

TEST(json_positions, simple) {
  const auto message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[{)"
                       R"("id":"615d67c7fa1b4f000638dd66",)"
                       R"("symbol":"XBTUSDTM",)"
                       R"("autoDeposit":false,)"
                       R"("maintMarginReq":0.005,)"
                       R"("riskLimit":200,)"
                       R"("realLeverage":0.00,)"
                       R"("crossMode":false,)"
                       R"("delevPercentage":0.0,)"
                       R"("currentTimestamp":1633511367833,)"
                       R"("currentQty":0,)"
                       R"("currentCost":0.00000000,)"
                       R"("currentComm":0.00000000,)"
                       R"("unrealisedCost":0.00000000,)"
                       R"("realisedGrossCost":0.00000000,)"
                       R"("realisedCost":0.00000000,)"
                       R"("isOpen":false,)"
                       R"("markPrice":50664.85,)"
                       R"("markValue":0.00000000,)"
                       R"("posCost":0.00000000,)"
                       R"("posCross":0,)"
                       R"("posInit":0.00000000,)"
                       R"("posComm":0.00000000,)"
                       R"("posLoss":0.00000000,)"
                       R"("posMargin":0.00000000,)"
                       R"("posMaint":0.00000000,)"
                       R"("maintMargin":0.00000000,)"
                       R"("realisedGrossPnl":0.00000000,)"
                       R"("realisedPnl":0.00000000,)"
                       R"("unrealisedPnl":0.00000000,)"
                       R"("unrealisedPnlPcnt":0,)"
                       R"("unrealisedRoePcnt":0,)"
                       R"("avgEntryPrice":0.00,)"
                       R"("liquidationPrice":0.00,)"
                       R"("bankruptPrice":0.00,)"
                       R"("settleCurrency":"USDT")"
                       R"(},{)"
                       R"("id":"615d67c71b8efa0006eecd0e",)"
                       R"("symbol":"XBTUSDM",)"
                       R"("autoDeposit":false,)"
                       R"("maintMarginReq":0.005,)"
                       R"("riskLimit":200,)"
                       R"("realLeverage":0.00,)"
                       R"("crossMode":false,)"
                       R"("delevPercentage":0.0,)"
                       R"("currentTimestamp":1633511367864,)"
                       R"("currentQty":0,)"
                       R"("currentCost":0.00000000,)"
                       R"("currentComm":0.00000000,)"
                       R"("unrealisedCost":0.00000000,)"
                       R"("realisedGrossCost":0.00000000,)"
                       R"("realisedCost":0.00000000,)"
                       R"("isOpen":false,)"
                       R"("markPrice":32421.3400000000,)"
                       R"("markValue":0.00000000,)"
                       R"("posCost":0.00000000,)"
                       R"("posCross":0,)"
                       R"("posInit":0.00000000,)"
                       R"("posComm":0.00000000,)"
                       R"("posLoss":0.00000000,)"
                       R"("posMargin":0.00000000,)"
                       R"("posMaint":0.00000000,)"
                       R"("maintMargin":0.00000000,)"
                       R"("realisedGrossPnl":0.00000000,)"
                       R"("realisedPnl":0.00000000,)"
                       R"("unrealisedPnl":0.00000000,)"
                       R"("unrealisedPnlPcnt":0,)"
                       R"("unrealisedRoePcnt":0,)"
                       R"("avgEntryPrice":0.00,)"
                       R"("liquidationPrice":0.00,)"
                       R"("bankruptPrice":0.00,)"
                       R"("settleCurrency":"XBT")"
                       R"(})"
                       R"(])"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Positions>(message, buffer_);
  EXPECT_EQ(obj.code, 200000);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 2);
  auto &d0 = data[0];
  EXPECT_EQ(d0.id, "615d67c7fa1b4f000638dd66"_sv);
  EXPECT_EQ(d0.symbol, "XBTUSDTM"_sv);
  EXPECT_EQ(d0.auto_deposit, false);
  EXPECT_DOUBLE_EQ(d0.maint_margin_req, 0.005);
  EXPECT_DOUBLE_EQ(d0.risk_limit, 200.0);
  EXPECT_DOUBLE_EQ(d0.real_leverage, 0.0);
  EXPECT_EQ(d0.cross_mode, false);
  EXPECT_DOUBLE_EQ(d0.delev_percentage, 0.0);
  EXPECT_EQ(d0.current_timestamp, 1633511367833ms);
  EXPECT_DOUBLE_EQ(d0.current_qty, 0.0);
  EXPECT_DOUBLE_EQ(d0.current_cost, 0.0);
  EXPECT_DOUBLE_EQ(d0.current_comm, 0.0);
  EXPECT_DOUBLE_EQ(d0.unrealised_cost, 0.0);
  EXPECT_DOUBLE_EQ(d0.realised_gross_cost, 0.0);
  EXPECT_DOUBLE_EQ(d0.realised_cost, 0.0);
  EXPECT_EQ(d0.is_open, false);
  EXPECT_DOUBLE_EQ(d0.mark_price, 50664.85);
  EXPECT_DOUBLE_EQ(d0.mark_value, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_cost, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_cross, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_init, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_comm, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_loss, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_margin, 0.0);
  EXPECT_DOUBLE_EQ(d0.pos_maint, 0.0);
  EXPECT_DOUBLE_EQ(d0.maint_margin, 0.0);
  EXPECT_DOUBLE_EQ(d0.realised_gross_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d0.realised_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d0.unrealised_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d0.unrealised_pnl_pcnt, 0.0);
  EXPECT_DOUBLE_EQ(d0.unrealised_roe_pcnt, 0.0);
  EXPECT_DOUBLE_EQ(d0.avg_entry_price, 0.0);
  EXPECT_DOUBLE_EQ(d0.liquidation_price, 0.0);
  EXPECT_DOUBLE_EQ(d0.bankrupt_price, 0.0);
  EXPECT_EQ(d0.settle_currency, "USDT"_sv);
  auto &d1 = data[1];
  EXPECT_EQ(d1.id, "615d67c71b8efa0006eecd0e"_sv);
  EXPECT_EQ(d1.symbol, "XBTUSDM"_sv);
  EXPECT_EQ(d1.auto_deposit, false);
  EXPECT_DOUBLE_EQ(d1.maint_margin_req, 0.005);
  EXPECT_DOUBLE_EQ(d1.risk_limit, 200.0);
  EXPECT_DOUBLE_EQ(d1.real_leverage, 0.0);
  EXPECT_EQ(d1.cross_mode, false);
  EXPECT_DOUBLE_EQ(d1.delev_percentage, 0.0);
  EXPECT_EQ(d1.current_timestamp, 1633511367864ms);
  EXPECT_DOUBLE_EQ(d1.current_qty, 0.0);
  EXPECT_DOUBLE_EQ(d1.current_cost, 0.0);
  EXPECT_DOUBLE_EQ(d1.current_comm, 0.0);
  EXPECT_DOUBLE_EQ(d1.unrealised_cost, 0.0);
  EXPECT_DOUBLE_EQ(d1.realised_gross_cost, 0.0);
  EXPECT_DOUBLE_EQ(d1.realised_cost, 0.0);
  EXPECT_EQ(d1.is_open, false);
  EXPECT_DOUBLE_EQ(d1.mark_price, 32421.34);
  EXPECT_DOUBLE_EQ(d1.mark_value, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_cost, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_cross, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_init, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_comm, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_loss, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_margin, 0.0);
  EXPECT_DOUBLE_EQ(d1.pos_maint, 0.0);
  EXPECT_DOUBLE_EQ(d1.maint_margin, 0.0);
  EXPECT_DOUBLE_EQ(d1.realised_gross_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d1.realised_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d1.unrealised_pnl, 0.0);
  EXPECT_DOUBLE_EQ(d1.unrealised_pnl_pcnt, 0.0);
  EXPECT_DOUBLE_EQ(d1.unrealised_roe_pcnt, 0.0);
  EXPECT_DOUBLE_EQ(d1.avg_entry_price, 0.0);
  EXPECT_DOUBLE_EQ(d1.liquidation_price, 0.0);
  EXPECT_DOUBLE_EQ(d1.bankrupt_price, 0.0);
  EXPECT_EQ(d1.settle_currency, "XBT"_sv);
}
