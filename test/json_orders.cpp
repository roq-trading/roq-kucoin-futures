/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/orders.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST(json_orders, simple) {
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
  auto obj = core::json::Parser::create<json::Orders>(message, buffer_);
  EXPECT_EQ(obj.code, 200000);
  auto &data = obj.data;
  EXPECT_EQ(data.current_page, 1);
  EXPECT_EQ(data.page_size, 50);
  EXPECT_EQ(data.total_num, 0);
  EXPECT_EQ(data.total_page, 0);
  auto &items = data.items;
  EXPECT_EQ(std::size(items), 0);
}
