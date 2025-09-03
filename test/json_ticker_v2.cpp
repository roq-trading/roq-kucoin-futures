/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/ticker_v2.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_ticker_v2", "[json_ticker_v2]") {
  auto const message = R"({)"
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::TickerV2 obj{message, buffer};
}
