/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::Level2;

TEST_CASE("simple", "[json_level2]") {
  auto message = R"({)"
                 R"("topic":"/contractMarket/level2:XBTUSDTM",)"
                 R"("type":"message",)"
                 R"("subject":"level2",)"
                 R"("sn":1720848028237,)"
                 R"("data":{)"
                 R"("sequence":1720848028237,)"
                 R"("change":"110224.7,sell,0",)"
                 R"("timestamp":1756205427835)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractMarket/level2:XBTUSDTM"sv);
    CHECK(obj.type == json::Type::MESSAGE);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
