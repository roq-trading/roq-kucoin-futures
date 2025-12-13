/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/cancel_order_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::CancelOrderAck;

TEST_CASE("success", "[json_cancel_order_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("cancelledOrderIds":[)"
                       R"("358883601460314112")"
                       R"(])"
                       R"(})"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    CHECK(std::empty(obj.msg));
    auto &data = obj.data;
    REQUIRE(std::size(data.cancelled_order_ids) == 1);
    CHECK(data.cancelled_order_ids[0] == "358883601460314112"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("order_quantity_too_high", "[json_cancel_order_ack]") {
  auto const message = R"({)"
                       R"("msg":"The order cannot be canceled.",)"
                       R"("code":"100004")"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 100004);
    CHECK(obj.msg == "The order cannot be canceled."sv);
    auto &data = obj.data;
    REQUIRE(std::empty(data.cancelled_order_ids));
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
