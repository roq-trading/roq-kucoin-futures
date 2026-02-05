/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Execution;

TEST_CASE("simple", "[json_execution]") {
  auto message = R"({)"
                 R"("type":"message",)"
                 R"("topic":"/futuresMarket/execution:BTCUSDM",)"
                 R"("subject":"execution",)"
                 R"("sn":636707859,)"
                 R"("data":{)"
                 R"("symbol":"BTCUSDM",)"
                 R"("matchSide":"sell",)"
                 R"("size":86,)"
                 R"("price":"19465.0",)"
                 R"("tradeId":636707859,)"
                 R"("ts":1656667043469)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.type == json::Type::MESSAGE);
    CHECK(obj.topic == "/futuresMarket/execution:BTCUSDM"sv);
    CHECK(obj.subject == json::Subject::EXECUTION);
    CHECK(obj.sn == 636707859);
    auto &data = obj.data;
    CHECK(data.symbol == "BTCUSDM"sv);
    CHECK(data.match_side == json::Side::SELL);
    CHECK(data.size == 86.0_a);
    CHECK(data.price == 19465.0_a);
    CHECK(data.trade_id == 636707859);
    CHECK(data.ts == 1656667043469ns);  // XXX ???
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
