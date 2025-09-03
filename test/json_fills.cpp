/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/fills.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_fills_simple", "[json_fills]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[)"
                       R"(])"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Fills obj{message, buffer};
  /*
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.current_page == 1);
  CHECK(data.page_size == 50);
  CHECK(data.total_num == 0);
  CHECK(data.total_page == 0);
  auto &items = data.items;
  CHECK(std::size(items) == 0);
  */
}
