/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/fills_ack.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using namespace Catch::literals;

using value_type = json::FillsAck;

TEST_CASE("empty", "[json_fills_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[])"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    CHECK(std::empty(obj.data));
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}

TEST_CASE("simple", "[json_fills_ack]") {
  auto const message = R"({)"
                       R"("code":"200000",)"
                       R"("data":[{)"
                       R"("symbol":"XBTUSDTM",)"
                       R"("tradeId":"1873921404162",)"
                       R"("orderId":"359065448081260544",)"
                       R"("side":"buy",)"
                       R"("liquidity":"taker",)"
                       R"("forceTaker":false,)"
                       R"("price":"115549.8",)"
                       R"("size":1,)"
                       R"("value":"115.5498",)"
                       R"("openFeePay":"0.055463904",)"
                       R"("closeFeePay":"0",)"
                       R"("stop":"",)"
                       R"("feeRate":"0.00048",)"
                       R"("fixFee":"0",)"
                       R"("feeCurrency":"USDT",)"
                       R"("tradeTime":1758421075974000000,)"
                       R"("subTradeType":null,)"
                       R"("marginMode":"CROSS",)"
                       R"("openFeeTaxPay":"0",)"
                       R"("closeFeeTaxPay":"0",)"
                       R"("positionSide":"BOTH",)"
                       R"("settleCurrency":"USDT",)"
                       R"("displayType":"limit",)"
                       R"("fee":"0.055463904",)"
                       R"("orderType":"limit",)"
                       R"("tradeType":"trade",)"
                       R"("createdAt":1758421075987)"
                       R"(})"
                       R"(])"
                       R"(})";
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    REQUIRE(std::size(obj.data) == 1);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
