/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::TickerV2;

TEST_CASE("simple", "[json_ticker_v2]") {
  auto message = R"({)"
                 R"("topic":"/contractMarket/tickerV2:XBTUSDTM",)"
                 R"("type":"message",)"
                 R"("subject":"tickerV2",)"
                 R"("sn":1720845437465,)"
                 R"("data":{)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("sequence":1720845437465,)"
                 R"("bestBidSize":685,)"
                 R"("bestBidPrice":"110212",)"
                 R"("bestAskPrice":"110212.1",)"
                 R"("bestAskSize":1941,)"
                 R"("ts":1756199639879000000)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractMarket/tickerV2:XBTUSDTM"sv);
    CHECK(obj.type == json::Type::MESSAGE);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
