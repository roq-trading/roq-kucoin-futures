/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::WalletBalanceChange;

TEST_CASE("simple", "[json_wallet_balance_change]") {
  auto message = R"({)"
                 R"("topic":"/contractAccount/wallet",)"
                 R"("type":"message",)"
                 R"("subject":"walletBalance.change",)"
                 R"("id":"68ce1cdd35423c000114ec16",)"
                 R"("userId":"67f914adc8d0110001ca099e",)"
                 R"("channelType":"private",)"
                 R"("data":{)"
                 R"("crossPosMargin":"0",)"
                 R"("isolatedOrderMargin":"0",)"
                 R"("holdBalance":"0",)"
                 R"("equity":"100",)"
                 R"("version":"10",)"
                 R"("availableBalance":"100",)"
                 R"("isolatedPosMargin":"0",)"
                 R"("maxWithdrawAmount":"100",)"
                 R"("walletBalance":"100",)"
                 R"("isolatedFundingFeeMargin":"0",)"
                 R"("crossUnPnl":"0",)"
                 R"("totalCrossMargin":"100",)"
                 R"("currency":"USDT",)"
                 R"("isolatedUnPnl":"0",)"
                 R"("crossOrderMargin":"0",)"
                 R"("timestamp":"1758277329658")"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractAccount/wallet"sv);
    CHECK(obj.id == "68ce1cdd35423c000114ec16"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("simple_2", "[json_wallet_balance_change]") {
  auto message = R"({)"
                 R"("topic":"/contractAccount/wallet",)"
                 R"("type":"message",)"
                 R"("subject":"walletBalance.change",)"
                 R"("id":"68cf605335423c000151c82d",)"
                 R"("userId":"67f914adc8d0110001ca099e",)"
                 R"("channelType":"private",)"
                 R"("data":{)"
                 R"("crossPosMargin":"38.5123399962",)"
                 R"("isolatedOrderMargin":"0",)"
                 R"("holdBalance":"0",)"
                 R"("equity":"99.931756096",)"
                 R"("version":"37",)"
                 R"("availableBalance":"61.4194160998",)"
                 R"("isolatedPosMargin":"0",)"
                 R"("maxWithdrawAmount":"61.4194160998",)"
                 R"("walletBalance":"99.944536096",)"
                 R"("isolatedFundingFeeMargin":"0",)"
                 R"("crossUnPnl":"-0.01278",)"
                 R"("totalCrossMargin":"99.931756096",)"
                 R"("currency":"USDT",)"
                 R"("isolatedUnPnl":"0",)"
                 R"("crossOrderMargin":"0",)"
                 R"("timestamp":"1758421075976")"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.topic == "/contractAccount/wallet"sv);
    CHECK(obj.id == "68cf605335423c000151c82d"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
