/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::Match;

TEST_CASE("simple", "[json_match]") {
  auto message = R"({)"
                 R"("topic":"/contractMarket/execution:ETHUSDCM",)"
                 R"("type":"message",)"
                 R"("subject":"match",)"
                 R"("sn":1704252565555,)"
                 R"("data":{)"
                 R"("symbol":"ETHUSDCM",)"
                 R"("sequence":1704252565555,)"
                 R"("side":"sell",)"
                 R"("size":17,)"
                 R"("price":"4419.15",)"
                 R"("takerOrderId":"349748069560008705",)"
                 R"("makerOrderId":"349748069539065857",)"
                 R"("tradeId":"1704252565555",)"
                 R"("ts":1756199639791000000)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractMarket/execution:ETHUSDCM"sv);
    CHECK(obj.type == json::Type::MESSAGE);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
