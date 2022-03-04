/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/withdraw_hold_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_withdraw_hold_change_example", "[json_withdraw_hold_change]") {
  const auto message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "withdrawHold.change",)"
                       R"("data": {)"
                       R"("withdrawHold": 5923,)"
                       R"("currency":"USDT",)"
                       R"("timestamp": 1553842862614)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::WithdrawHoldChange>(message, buffer_);
  CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
  CHECK(obj.topic == "/contractAccount/wallet"sv);
  CHECK(obj.subject == json::Subject::WITHDRAW_HOLD_CHANGE);
  auto &data = obj.data;
  CHECK(data.withdraw_hold == 5923.0);
  CHECK(data.currency == "USDT"sv);
  CHECK(data.timestamp == 1553842862614ms);
}
