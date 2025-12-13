/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/order_book_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::OrderBookAck;

TEST_CASE("simple", "[json_order_book_ack]") {
  auto message = R"({)"
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.code == 200000);
    CHECK(obj.data.sequence == 1695731285612);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
