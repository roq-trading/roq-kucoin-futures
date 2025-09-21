/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/parser.hpp"
#include "roq/kucoin_futures/json/wallet_balance_change.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

namespace {
auto const MESSAGE = R"({)"
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
}

TEST_CASE("simple", "[json_wallet_balance_change]") {
  core::json::BufferStack buffer{8192, 1};
  json::WalletBalanceChange obj{MESSAGE, buffer};
}

TEST_CASE("simple_parser", "[json_wallet_balance_change]") {
  struct MyHandler final : public json::Parser::Handler {
    size_t count = {};

   protected:
    void operator()(Trace<json::Welcome> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Pong> const &) override { FAIL(); }
    void operator()(Trace<json::Ack> const &) override { FAIL(); }

    void operator()(Trace<json::TickerV2> const &) override { FAIL(); }
    void operator()(Trace<json::Match> const &) override { FAIL(); }
    void operator()(Trace<json::Execution> const &) override { FAIL(); }
    void operator()(Trace<json::MarkIndexPrice> const &) override { FAIL(); }
    void operator()(Trace<json::FundingRate> const &) override { FAIL(); }
    void operator()(Trace<json::Level2> const &) override { FAIL(); }
    void operator()(Trace<json::FundingBegin> const &) override { FAIL(); }
    void operator()(Trace<json::FundingEnd> const &) override { FAIL(); }
    void operator()(Trace<json::Snapshot24h> const &) override { FAIL(); }

    void operator()(Trace<json::WalletBalanceChange> const &event) override {
      ++count;
      auto &[trace_info, wallet_balance_change] = event;
      CHECK(wallet_balance_change.id == "68ce1cdd35423c000114ec16"sv);
    }
    void operator()(Trace<json::OrderMarginChange> const &) override { FAIL(); }
    void operator()(Trace<json::AvailableBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::WithdrawHoldChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionSettlement> const &) override { FAIL(); }
    void operator()(Trace<json::PositionAdjustRiskLimit> const &) override { FAIL(); }
    void operator()(Trace<json::SymbolOrderChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderChange> const &) override { FAIL(); }

  } handler;
  core::json::BufferStack buffer{8192, 1};
  auto res = json::Parser::dispatch(handler, MESSAGE, buffer, {}, false);
  CHECK(res == true);
  CHECK(handler.count == 1);
}

namespace {
auto const MESSAGE_2 = R"({)"
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
}

TEST_CASE("simple_2", "[json_wallet_balance_change]") {
  core::json::BufferStack buffer{8192, 1};
  json::WalletBalanceChange obj{MESSAGE_2, buffer};
}

TEST_CASE("simple_2_parser", "[json_wallet_balance_change]") {
  struct MyHandler final : public json::Parser::Handler {
    size_t count = {};

   protected:
    void operator()(Trace<json::Welcome> const &) override { FAIL(); }
    void operator()(Trace<json::Error> const &) override { FAIL(); }
    void operator()(Trace<json::Pong> const &) override { FAIL(); }
    void operator()(Trace<json::Ack> const &) override { FAIL(); }

    void operator()(Trace<json::TickerV2> const &) override { FAIL(); }
    void operator()(Trace<json::Match> const &) override { FAIL(); }
    void operator()(Trace<json::Execution> const &) override { FAIL(); }
    void operator()(Trace<json::MarkIndexPrice> const &) override { FAIL(); }
    void operator()(Trace<json::FundingRate> const &) override { FAIL(); }
    void operator()(Trace<json::Level2> const &) override { FAIL(); }
    void operator()(Trace<json::FundingBegin> const &) override { FAIL(); }
    void operator()(Trace<json::FundingEnd> const &) override { FAIL(); }
    void operator()(Trace<json::Snapshot24h> const &) override { FAIL(); }

    void operator()(Trace<json::WalletBalanceChange> const &event) override {
      ++count;
      auto &[trace_info, wallet_balance_change] = event;
      CHECK(wallet_balance_change.id == "68cf605335423c000151c82d"sv);
    }
    void operator()(Trace<json::OrderMarginChange> const &) override { FAIL(); }
    void operator()(Trace<json::AvailableBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::WithdrawHoldChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionSettlement> const &) override { FAIL(); }
    void operator()(Trace<json::PositionAdjustRiskLimit> const &) override { FAIL(); }
    void operator()(Trace<json::SymbolOrderChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderChange> const &) override { FAIL(); }

  } handler;
  core::json::BufferStack buffer{8192, 1};
  auto res = json::Parser::dispatch(handler, MESSAGE_2, buffer, {}, false);
  CHECK(res == true);
  CHECK(handler.count == 1);
}
