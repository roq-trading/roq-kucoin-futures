/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/account_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::AccountAck;

TEST_CASE("simple", "[json_account_ack]") {
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
  auto helper = [&](value_type &obj) {
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
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
