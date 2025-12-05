/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/add_order_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::AddOrderAck;

TEST_CASE("success", "[json_add_order_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("orderId":"358883601460314112",)"
                       R"("clientOid":"rgACZ0Er0zQAAQAAAAAA")"
                       R"(})"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    CHECK(std::empty(obj.msg));
    auto &data = obj.data;
    CHECK(data.order_id == "358883601460314112"sv);
    CHECK(data.client_oid == "rgACZ0Er0zQAAQAAAAAA"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("order_quantity_too_high", "[json_add_order_ack]") {
  auto const message = R"({)"
                       R"("msg":"Order quantity is too high, insufficient available margin.",)"
                       R"("code":"330008")"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 330008);
    CHECK(obj.msg == "Order quantity is too high, insufficient available margin."sv);
    auto &data = obj.data;
    CHECK(std::empty(data.order_id));
    CHECK(std::empty(data.client_oid));
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
