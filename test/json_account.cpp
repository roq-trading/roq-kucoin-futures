/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/kucoin_futures/json/account.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_account_simple", "[json_account]") {
  const auto message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("accountEquity":0.00000000,)"
                       R"("unrealisedPNL":0.00000000,)"
                       R"("marginBalance":0.00000000,)"
                       R"("positionMargin":0.00000000,)"
                       R"("orderMargin":0.00000000,)"
                       R"("frozenFunds":0.00000000,)"
                       R"("availableBalance":0.00000000,)"
                       R"("currency":"XBT")"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Account>(message, buffer_);
  CHECK(obj.code == 200000);
  auto &data = obj.data;
  CHECK(data.account_equity == 0.0_a);
  CHECK(data.unrealised_pnl == 0.0_a);
  CHECK(data.margin_balance == 0.0_a);
  CHECK(data.position_margin == 0.0_a);
  CHECK(data.order_margin == 0.0_a);
  CHECK(data.frozen_funds == 0.0_a);
  CHECK(data.available_balance == 0.0_a);
  CHECK(data.currency == "XBT"sv);
}
