/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/order_book.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

TEST_CASE("json_order_book", "[json_order_book]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("sequence":1695731285612,)"
                       R"("symbol":"XBTUSDCM",)"
                       R"("bids":[)"
                       R"([110135.5,176],)"
                       R"([110131.8,640],)"
                       R"([0.1,15602000]],)"
                       R"("asks":[)"
                       R"([110269.7,176],)"
                       R"([110275.0,612],)"
                       R"([141999.9,3])"
                       R"(],)"
                       R"("ts":1756205428766000000)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  json::OrderBook obj{message, buffer};
}
