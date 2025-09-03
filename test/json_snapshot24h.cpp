/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/snapshot24h.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_snapshot24h", "[json_snapshot24h]") {
  auto const message = R"({)"
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Snapshot24h obj{message, buffer};
}
