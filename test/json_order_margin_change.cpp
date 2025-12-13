/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OrderMarginChange;

TEST_CASE("simple", "[json_order_margin_change]") {
  /*
  auto message = R"({)"
                 R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                 R"("topic": "/contractAccount/wallet",)"
                 R"("subject": "orderMargin.change",)"
                 R"("data": {)"
                 R"("orderMargin": 5923,)"
                 R"("currency":"USDT",)"
                 R"("timestamp": 1553842862614)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
    CHECK(obj.topic == "/contractAccount/wallet"sv);
    CHECK(obj.subject == json::Subject::ORDER_MARGIN_CHANGE);
    auto &data = obj.data;
    CHECK(data.order_margin == 5923.0_a);
    CHECK(data.currency == "USDT"sv);
    CHECK(data.timestamp == 1553842862614ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
  */
}
