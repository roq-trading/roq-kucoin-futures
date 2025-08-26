/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/funding_end.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_funding_end", "[json_funding_end]") {
  auto const message = R"({)"
                       // XXX FIXME don't have
                       R"(})";
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::FundingEnd obj{message, buffer};
}
