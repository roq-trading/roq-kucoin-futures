/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/level2.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_v2_level2", "[json_level2]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/futuresMarket/level2:YFIUSDTM",)"
                       R"("subject":"level2",)"
                       R"("sn":80788065,)"
                       R"("data":{)"
                       R"("start":80788046,)"
                       R"("bids":[)"
                       R"(["5254.3",0],)"
                       R"(["5218.0",0],)"
                       R"(["5162.8",0],)"
                       R"(["5160.8",31],)"
                       R"(["5049.6",8])"
                       R"(],)"
                       R"("end":80788065,)"
                       R"("ts":1656674556040,)"
                       R"("asks":[)"
                       R"(["5259.7",28],)"
                       R"(["5286.0",0],)"
                       R"(["5308.9",0],)"
                       R"(["5310.7",28],)"
                       R"(["5325.0",0],)"
                       R"(["5327.7",0],)"
                       R"(["5427.0",294],)"
                       R"(["5438.2",0],)"
                       R"(["5439.3",29])"
                       R"(])"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::Level2::create(message, buffer);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/futuresMarket/level2:YFIUSDTM"sv);
  CHECK(obj.subject == json::Subject::LEVEL2);
  CHECK(obj.sn == 80788065);
  auto &data = obj.data;
  CHECK(data.start == 80788046);
  auto &bids = data.bids;
  REQUIRE(std::size(bids) == 5);
  CHECK(bids[0].price == 5254.3_a);
  CHECK(bids[0].size == 0.0_a);
  CHECK(data.end == 80788065);
  CHECK(data.ts == 1656674556040ms);
  auto &asks = data.asks;
  REQUIRE(std::size(asks) == 9);
  CHECK(asks[0].price == 5259.7_a);
  CHECK(asks[0].size == 28.0_a);
}
