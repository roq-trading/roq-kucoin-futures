/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/account.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("simple", "[json_account]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":{)"
                       R"("accountEquity":0,)"
                       R"("unrealisedPNL":0,)"
                       R"("marginBalance":0,)"
                       R"("positionMargin":0,)"
                       R"("orderMargin":0,)"
                       R"("frozenFunds":0,)"
                       R"("availableBalance":0,)"
                       R"("currency":"XBT",)"
                       R"("riskRatio":0,)"
                       R"("maxWithdrawAmount":0)"
                       R"(})"
                       R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Account obj{message, buffer};
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
  CHECK(data.risk_ratio == 0.0_a);
  CHECK(data.max_withdraw_amount == 0.0_a);
}
