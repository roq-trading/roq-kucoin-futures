/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/kucoin_futures/json/fills.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_fills_simple", "[json_fills]") {
  const auto message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("currentPage":1,)"
                       R"("pageSize":50,)"
                       R"("totalNum":0,)"
                       R"("totalPage":0,)"
                       R"("items":[])"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Fills>(message, buffer_);
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.current_page == 1);
  CHECK(data.page_size == 50);
  CHECK(data.total_num == 0);
  CHECK(data.total_page == 0);
  auto &items = data.items;
  CHECK(std::size(items) == 0);
}
