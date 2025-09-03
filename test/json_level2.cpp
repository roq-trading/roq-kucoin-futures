/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/level2.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_level2", "[json_level2]") {
  auto const message = R"({)"
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
  core::json::BufferStack buffer{8192, 1};
  json::Level2 obj{message, buffer};
}
