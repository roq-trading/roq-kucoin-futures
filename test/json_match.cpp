/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/match.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_match", "[json_match]") {
  auto const message = R"({)"
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::Match obj{message, buffer};
}
