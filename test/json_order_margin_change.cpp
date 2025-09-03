/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/order_margin_change.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_margin_change_example", "[json_order_margin_change]") {
  auto const message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "orderMargin.change",)"
                       R"("data": {)"
                       R"("orderMargin": 5923,)"
                       R"("currency":"USDT",)"
                       R"("timestamp": 1553842862614)"
                       R"(})"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::OrderMarginChange obj{message, buffer};
  CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
  CHECK(obj.topic == "/contractAccount/wallet"sv);
  CHECK(obj.subject == json::Subject::ORDER_MARGIN_CHANGE);
  auto &data = obj.data;
  CHECK(data.order_margin == 5923.0_a);
  CHECK(data.currency == "USDT"sv);
  CHECK(data.timestamp == 1553842862614ms);
}
