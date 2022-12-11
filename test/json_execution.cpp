/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/kucoin_futures/json/execution.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_v2_execution_simple", "[json_execution]") {
  auto const message = R"({)"
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Execution>(message, buffer_);
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
  CHECK(data.ts == 1656667043469ms);
}
