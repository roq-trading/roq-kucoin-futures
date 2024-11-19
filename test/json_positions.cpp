/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/positions.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_positions_simple", "[json_positions]") {
  auto const message = R"({)"
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
  std::vector<std::byte> buffer(8192);
  json::Positions obj{message, buffer};
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  REQUIRE(std::size(data) == 2);
  auto &d0 = data[0];
  CHECK(d0.id == "615d67c7fa1b4f000638dd66"sv);
  CHECK(d0.symbol == "XBTUSDTM"sv);
  CHECK(d0.auto_deposit == false);
  CHECK(d0.maint_margin_req == 0.005_a);
  CHECK(d0.risk_limit == 200.0_a);
  CHECK(d0.real_leverage == 0.0_a);
  CHECK(d0.cross_mode == false);
  CHECK(d0.delev_percentage == 0.0_a);
  CHECK(d0.current_timestamp == 1633511367833ms);
  CHECK(d0.current_qty == 0.0_a);
  CHECK(d0.current_cost == 0.0_a);
  CHECK(d0.current_comm == 0.0_a);
  CHECK(d0.unrealised_cost == 0.0_a);
  CHECK(d0.realised_gross_cost == 0.0_a);
  CHECK(d0.realised_cost == 0.0_a);
  CHECK(d0.is_open == false);
  CHECK(d0.mark_price == 50664.85_a);
  CHECK(d0.mark_value == 0.0_a);
  CHECK(d0.pos_cost == 0.0_a);
  CHECK(d0.pos_cross == 0.0_a);
  CHECK(d0.pos_init == 0.0_a);
  CHECK(d0.pos_comm == 0.0_a);
  CHECK(d0.pos_loss == 0.0_a);
  CHECK(d0.pos_margin == 0.0_a);
  CHECK(d0.pos_maint == 0.0_a);
  CHECK(d0.maint_margin == 0.0_a);
  CHECK(d0.realised_gross_pnl == 0.0_a);
  CHECK(d0.realised_pnl == 0.0_a);
  CHECK(d0.unrealised_pnl == 0.0_a);
  CHECK(d0.unrealised_pnl_pcnt == 0.0_a);
  CHECK(d0.unrealised_roe_pcnt == 0.0_a);
  CHECK(d0.avg_entry_price == 0.0_a);
  CHECK(d0.liquidation_price == 0.0_a);
  CHECK(d0.bankrupt_price == 0.0_a);
  CHECK(d0.settle_currency == "USDT"sv);
  auto &d1 = data[1];
  CHECK(d1.id == "615d67c71b8efa0006eecd0e"sv);
  CHECK(d1.symbol == "XBTUSDM"sv);
  CHECK(d1.auto_deposit == false);
  CHECK(d1.maint_margin_req == 0.005_a);
  CHECK(d1.risk_limit == 200.0_a);
  CHECK(d1.real_leverage == 0.0_a);
  CHECK(d1.cross_mode == false);
  CHECK(d1.delev_percentage == 0.0_a);
  CHECK(d1.current_timestamp == 1633511367864ms);
  CHECK(d1.current_qty == 0.0_a);
  CHECK(d1.current_cost == 0.0_a);
  CHECK(d1.current_comm == 0.0_a);
  CHECK(d1.unrealised_cost == 0.0_a);
  CHECK(d1.realised_gross_cost == 0.0_a);
  CHECK(d1.realised_cost == 0.0_a);
  CHECK(d1.is_open == false);
  CHECK(d1.mark_price == 32421.34_a);
  CHECK(d1.mark_value == 0.0_a);
  CHECK(d1.pos_cost == 0.0_a);
  CHECK(d1.pos_cross == 0.0_a);
  CHECK(d1.pos_init == 0.0_a);
  CHECK(d1.pos_comm == 0.0_a);
  CHECK(d1.pos_loss == 0.0_a);
  CHECK(d1.pos_margin == 0.0_a);
  CHECK(d1.pos_maint == 0.0_a);
  CHECK(d1.maint_margin == 0.0_a);
  CHECK(d1.realised_gross_pnl == 0.0_a);
  CHECK(d1.realised_pnl == 0.0_a);
  CHECK(d1.unrealised_pnl == 0.0_a);
  CHECK(d1.unrealised_pnl_pcnt == 0.0_a);
  CHECK(d1.unrealised_roe_pcnt == 0.0_a);
  CHECK(d1.avg_entry_price == 0.0_a);
  CHECK(d1.liquidation_price == 0.0_a);
  CHECK(d1.bankrupt_price == 0.0_a);
  CHECK(d1.settle_currency == "XBT"sv);
}
