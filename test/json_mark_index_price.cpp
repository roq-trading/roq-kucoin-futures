/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/mark_index_price.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_mark_index_price", "[json_mark_price]") {
  auto const message = R"({)"
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
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::MarkIndexPrice obj{message, buffer};
}
