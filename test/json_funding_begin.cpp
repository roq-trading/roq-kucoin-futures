/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/funding_begin.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_funding_begin", "[json_funding_begin]") {
  auto const message = R"({)"
                       // XXX FIXME don't have
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  [[maybe_unused]] json::FundingBegin obj{message, buffer};
}
