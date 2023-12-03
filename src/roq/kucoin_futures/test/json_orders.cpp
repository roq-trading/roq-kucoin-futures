/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/orders.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_orders_simple", "[json_orders]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("currentPage":1,)"
                       R"("pageSize":50,)"
                       R"("totalNum":0,)"
                       R"("totalPage":0,)"
                       R"("items":[])"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::Orders::create(message, buffer);
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.current_page == 1);
  CHECK(data.page_size == 50);
  CHECK(data.total_num == 0);
  CHECK(data.total_page == 0);
  auto &items = data.items;
  CHECK(std::size(items) == 0);
}
