/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/available_balance_change.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_available_balance_change_example", "json_available_balance_change") {
  const auto message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "availableBalance.change",)"
                       R"("data": {)"
                       R"("availableBalance": 5923,)"
                       R"("holdBalance": 2312,)"
                       R"("currency":"USDT",)"
                       R"("timestamp": 1553842862614)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::AvailableBalanceChange>(message, buffer_);
  CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
  CHECK(obj.topic == "/contractAccount/wallet"sv);
  CHECK(obj.subject == json::Subject::AVAILABLE_BALANCE_CHANGE);
  auto &data = obj.data;
  CHECK(data.available_balance == 5923.0_a);
  CHECK(data.hold_balance == 2312.0_a);
  CHECK(data.currency == "USDT"sv);
  CHECK(data.timestamp == 1553842862614ms);
}
