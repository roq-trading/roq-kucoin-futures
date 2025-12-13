/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/positions_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::PositionsAck;

TEST_CASE("empty", "[json_positions_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[])"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    CHECK(std::empty(obj.data));
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("simple", "[json_positions_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[{)"
                       R"("id":"400000000001959252",)"
                       R"("symbol":"XBTUSDTM",)"
                       R"("crossMode":true,)"
                       R"("maintMarginReq":0.0040000133,)"
                       R"("delevPercentage":0.4,)"
                       R"("openingTimestamp":1758421075976,)"
                       R"("currentTimestamp":1758422055184,)"
                       R"("currentQty":1,)"
                       R"("currentCost":115.5498,)"
                       R"("currentComm":0.055463904,)"
                       R"("unrealisedCost":115.5498,)"
                       R"("realisedGrossCost":0.0,)"
                       R"("realisedCost":0.055463904,)"
                       R"("isOpen":true,)"
                       R"("markPrice":115582.97,)"
                       R"("markValue":115.58297,)"
                       R"("posCost":115.5498,)"
                       R"("posInit":38.5165999962,)"
                       R"("posMargin":38.5276566629,)"
                       R"("posMaint":0.4623334173,)"
                       R"("realisedGrossPnl":0.0,)"
                       R"("realisedPnl":-0.055463904,)"
                       R"("unrealisedPnl":0.03317,)"
                       R"("unrealisedPnlPcnt":3.0E-4,)"
                       R"("unrealisedRoePcnt":9.0E-4,)"
                       R"("avgEntryPrice":115549.8,)"
                       R"("liquidationPrice":15675.5,)"
                       R"("bankruptPrice":15605.27,)"
                       R"("settleCurrency":"USDT",)"
                       R"("isInverse":false,)"
                       R"("maintainMargin":0.0040000133,)"
                       R"("marginMode":"CROSS",)"
                       R"("positionSide":"BOTH",)"
                       R"("leverage":3,)"
                       R"("dealComm":-0.055463904,)"
                       R"("fundingFee":0,)"
                       R"("tax":0)"
                       R"(})"
                       R"(])"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    auto &data = obj.data;
    REQUIRE(std::size(data) == 1);
    /*
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
    */
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
