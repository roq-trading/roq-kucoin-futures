/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::WithdrawHoldChange;

TEST_CASE("simple", "[json_withdraw_hold_change]") {
  /*
  auto message = R"({)"
                 R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                 R"("topic": "/contractAccount/wallet",)"
                 R"("subject": "withdrawHold.change",)"
                 R"("data": {)"
                 R"("withdrawHold": 5923,)"
                 R"("currency":"USDT",)"
                 R"("timestamp": 1553842862614)"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
    CHECK(obj.topic == "/contractAccount/wallet"sv);
    CHECK(obj.subject == json::Subject::WITHDRAW_HOLD_CHANGE);
    auto &data = obj.data;
    CHECK(data.withdraw_hold == 5923.0_a);
    CHECK(data.currency == "USDT"sv);
    CHECK(data.timestamp == 1553842862614ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
  */
}
