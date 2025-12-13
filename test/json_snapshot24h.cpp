/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::Snapshot24h;

TEST_CASE("simple", "[json_snapshot24h]") {
  auto message = R"({)"
                 R"("topic":"/contractMarket/snapshot:ETHUSDCM",)"
                 R"("type":"message",)"
                 R"("subject":"snapshot.24h",)"
                 R"("id":"68ad7e39d5bc9700014a4ebd",)"
                 R"("data":{)"
                 R"("highPrice":4684.50,)"
                 R"("lastPrice":4433.86,)"
                 R"("lowPrice":4310.45,)"
                 R"("price24HoursBefore":4594.44,)"
                 R"("priceChg":-160.58,)"
                 R"("priceChgPct":-0.0349,)"
                 R"("symbol":"ETHUSDCM",)"
                 R"("ts":1756200505012494819,)"
                 R"("turnover":3068698.8596315383,)"
                 R"("volume":684.327)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractMarket/snapshot:ETHUSDCM"sv);
    CHECK(obj.type == json::Type::MESSAGE);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
