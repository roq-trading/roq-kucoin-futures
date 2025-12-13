/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::MarkIndexPrice;

TEST_CASE("simple", "[json_mark_price]") {
  auto message = R"({)"
                 R"("topic":"/contract/instrument:XBTUSDTM",)"
                 R"("type":"message",)"
                 R"("subject":"mark.index.price",)"
                 R"("data":{)"
                 R"("markPrice":110110.10,)"
                 R"("indexPrice":110109.61,)"
                 R"("granularity":1000,)"
                 R"("timestamp":1756201897000)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contract/instrument:XBTUSDTM"sv);
    CHECK(obj.type == json::Type::MESSAGE);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
