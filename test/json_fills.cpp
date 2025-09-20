/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/fills.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("empty", "[json_fills]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[])"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Fills obj{message, buffer};
  CHECK(obj.code == 200000);
  CHECK(std::empty(obj.data));
}
