/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/orders.h"

using namespace roq;
using namespace roq::kucoin_futures;

TEST(json_orders, simple) {
  const auto message = R"({})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Orders>(message, buffer_);
}
