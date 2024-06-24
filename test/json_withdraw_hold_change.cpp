/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/withdraw_hold_change.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_withdraw_hold_change_example", "[json_withdraw_hold_change]") {
  auto const message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "withdrawHold.change",)"
                       R"("data": {)"
                       R"("withdrawHold": 5923,)"
                       R"("currency":"USDT",)"
                       R"("timestamp": 1553842862614)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  json::WithdrawHoldChange obj{message, buffer};
  CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
  CHECK(obj.topic == "/contractAccount/wallet"sv);
  CHECK(obj.subject == json::Subject::WITHDRAW_HOLD_CHANGE);
  auto &data = obj.data;
  CHECK(data.withdraw_hold == 5923.0_a);
  CHECK(data.currency == "USDT"sv);
  CHECK(data.timestamp == 1553842862614ms);
}
