/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/funding_rate.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_funding_rate", "[json_funding_rate]") {
  auto const message = R"({)"
                       R"("topic":"/contract/instrument:XBTUSDCM",)"
                       R"("type":"message",)"
                       R"("subject":"funding.rate",)"
                       R"("data":{)"
                       R"("period":1,)"
                       R"("granularity":60000,)"
                       R"("fundingRate":0.000077,)"
                       R"("timestamp":1756201920000)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::FundingRate obj{message, buffer};
}
