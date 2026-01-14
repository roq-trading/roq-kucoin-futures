/* Copyright (c) 2017-2026, Hans Erik Thrane */

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
    CHECK(obj.subject == json::Subject::POSITION_CHANGE);
    CHECK(obj.user_id == "67f914adc8d0110001ca099e"sv);
    CHECK(obj.channel_type == "private"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
