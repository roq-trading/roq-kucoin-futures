/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::PositionChange;

TEST_CASE("simple", "[json_position_change]") {
  auto message = R"({ )"
                 R"("topic":"/contract/positionAll",)"
                 R"("type":"message",)"
                 R"("data":{)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("maintMarginReq":0.004,)"
                 R"("crossMode":true,)"
                 R"("delevPercentage":0,)"
                 R"("openingTimestamp":1758421075976,)"
                 R"("currentTimestamp":1758424326819,)"
                 R"("currentQty":0,)"
                 R"("currentCost":-0.1138,)"
                 R"("currentComm":0.110982432,)"
                 R"("unrealisedCost":0,)"
                 R"("realisedCost":-0.002817568,)"
                 R"("isOpen":false,)"
                 R"("markPrice":115672.14,)"
                 R"("markValue":0,)"
                 R"("posCost":0,)"
                 R"("posInit":0,)"
                 R"("posMaint":0,)"
                 R"("avgEntryPrice":115549.8,)"
                 R"("liquidationPrice":0,)"
                 R"("bankruptPrice":0,)"
                 R"("settleCurrency":"USDT",)"
                 R"("changeReason":"positionChange",)"
                 R"("realisedGrossCost":-0.1138,)"
                 R"("realisedGrossPnl":0.1138,)"
                 R"("realisedPnl":0.002817568,)"
                 R"("unrealisedPnl":0,)"
                 R"("unrealisedPnlPcnt":0,)"
                 R"("unrealisedRoePcnt":0,)"
                 R"("leverage":3,)"
                 R"("marginMode":"CROSS",)"
                 R"("positionSide":"BOTH",)"
                 R"("tax":0,)"
                 R"("dealComm":-0.110982432,)"
                 R"("fundingFee":0)"
                 R"(},)"
                 R"("subject":"position.change",)"
                 R"("userId":"67f914adc8d0110001ca099e",)"
                 R"("channelType":"private")"
                 R"(})";
  auto helper = [](value_type const &obj) {
    /*
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
    */
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
