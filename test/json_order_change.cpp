/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/order_change.hpp"
#include "roq/kucoin_futures/json/parser.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

namespace {
auto const MESSAGE_OPEN = R"({)"
                          R"("type": "message",)"
                          R"("topic": "/contractMarket/tradeOrders",)"
                          R"("subject": "orderChange",)"
                          R"("channelType": "private",)"
                          R"("data": {)"
                          R"("orderId": "5cdfc138b21023a909e5ad55",)"
                          R"("symbol": "XBTUSDM",)"
                          R"("type": "match",)"
                          R"("status": "open",)"
                          R"("matchSize": "",)"
                          R"("matchPrice": "",)"
                          R"("orderType": "limit",)"
                          R"("side": "buy",)"
                          R"("price": "3600",)"
                          R"("size": "20000",)"
                          R"("remainSize": "20001",)"
                          R"("filledSize":"20000",)"
                          R"("canceledSize": "0",)"
                          R"("tradeId": "5ce24c16b210233c36eexxxx",)"
                          R"("clientOid": "5ce24c16b210233c36ee321d",)"
                          R"("orderTime": 1545914149935808589,)"
                          R"("oldSize": "15000",)"
                          R"("liquidity": "maker",)"
                          R"("ts": 1545914149935808589)"
                          R"(})"
                          R"(})"sv;
}

TEST_CASE("open", "[json_order_change]") {
  core::json::BufferStack buffer{8192, 1};
  json::OrderChange obj{MESSAGE_OPEN, buffer};
  /*
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
  CHECK(obj.subject == json::Subject::ORDER_CHANGE);
  CHECK(obj.channel_type == "private"sv);
  auto &data = obj.data;
  CHECK(data.order_id == "5cdfc138b21023a909e5ad55"sv);
  CHECK(data.symbol == "XBTUSDM"sv);
  CHECK(data.type == json::OrderUpdateType::MATCH);
  CHECK(data.status == json::OrderStatus::OPEN);
  CHECK(std::isnan(data.match_size) == true);
  CHECK(std::isnan(data.match_price) == true);
  CHECK(data.order_type == json::OrderType::LIMIT);
  CHECK(data.side == json::Side::BUY);
  CHECK(data.price == 3600.0_a);
  CHECK(data.size == 20000.0_a);
  CHECK(data.remain_size == 20001.0_a);
  CHECK(data.filled_size == 20000.0_a);
  CHECK(data.canceled_size == 0.0_a);
  CHECK(data.trade_id == "5ce24c16b210233c36eexxxx"sv);
  CHECK(data.client_oid == "5ce24c16b210233c36ee321d"sv);
  CHECK(data.order_time == 1545914149935808589ns);
  CHECK(data.old_size == 15000.0_a);
  CHECK(data.liquidity == json::Liquidity::MAKER);
  CHECK(data.ts == 1545914149935808589ns);
  */
}

TEST_CASE("open_parser", "[json_order_change]") {
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

    void operator()(Trace<json::WalletBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderMarginChange> const &) override { FAIL(); }
    void operator()(Trace<json::AvailableBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::WithdrawHoldChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionSettlement> const &) override { FAIL(); }
    void operator()(Trace<json::PositionAdjustRiskLimit> const &) override { FAIL(); }
    void operator()(Trace<json::SymbolOrderChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderChange> const &event) override {
      ++count;
      auto &[trace_info, order_change] = event;
    }

  } handler;
  core::json::BufferStack buffer{8192, 1};
  auto res = json::Parser::dispatch(handler, MESSAGE_OPEN, buffer, {}, false);
  CHECK(res == true);
  CHECK(handler.count == 1);
}

namespace {
auto const MESSAGE_CANCELED = R"({)"
                              R"("topic":"/contractMarket/tradeOrders",)"
                              R"("type":"message",)"
                              R"("subject":"orderChange",)"
                              R"("userId":"67f914adc8d0110001ca099e",)"
                              R"("channelType":"private",)"
                              R"("data":{)"
                              R"("symbol":"XBTUSDTM",)"
                              R"("orderType":"limit",)"
                              R"("side":"buy",)"
                              R"("canceledSize":"1",)"
                              R"("orderId":"358721557486432258",)"
                              R"("positionSide":"BOTH",)"
                              R"("marginMode":"CROSS",)"
                              R"("type":"canceled",)"
                              R"("orderTime":1758339086029193392,)"
                              R"("size":"1",)"
                              R"("filledSize":"0",)"
                              R"("price":"32000",)"
                              R"("remainSize":"0",)"
                              R"("clientOid":"MwACgFskvDQAAQAAAAAA",)"
                              R"("tradeType":"trade",)"
                              R"("status":"done",)"
                              R"("ts":1758339087398000000)"
                              R"(})"
                              R"(})"sv;
}  // namespace

TEST_CASE("canceled", "[json_order_change]") {
  core::json::BufferStack buffer{8192, 1};
  json::OrderChange obj{MESSAGE_CANCELED, buffer};
  /*
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
  CHECK(obj.subject == json::Subject::ORDER_CHANGE);
  CHECK(obj.channel_type == "private"sv);
  auto &data = obj.data;
  CHECK(data.order_id == "5cdfc138b21023a909e5ad55"sv);
  CHECK(data.symbol == "XBTUSDM"sv);
  CHECK(data.type == json::OrderUpdateType::MATCH);
  CHECK(data.status == json::OrderStatus::OPEN);
  CHECK(std::isnan(data.match_size) == true);
  CHECK(std::isnan(data.match_price) == true);
  CHECK(data.order_type == json::OrderType::LIMIT);
  CHECK(data.side == json::Side::BUY);
  CHECK(data.price == 3600.0_a);
  CHECK(data.size == 20000.0_a);
  CHECK(data.remain_size == 20001.0_a);
  CHECK(data.filled_size == 20000.0_a);
  CHECK(data.canceled_size == 0.0_a);
  CHECK(data.trade_id == "5ce24c16b210233c36eexxxx"sv);
  CHECK(data.client_oid == "5ce24c16b210233c36ee321d"sv);
  CHECK(data.order_time == 1545914149935808589ns);
  CHECK(data.old_size == 15000.0_a);
  CHECK(data.liquidity == json::Liquidity::MAKER);
  CHECK(data.ts == 1545914149935808589ns);
  */
}

TEST_CASE("canceled_parser", "[json_order_change]") {
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

    void operator()(Trace<json::WalletBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderMarginChange> const &) override { FAIL(); }
    void operator()(Trace<json::AvailableBalanceChange> const &) override { FAIL(); }
    void operator()(Trace<json::WithdrawHoldChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionChange> const &) override { FAIL(); }
    void operator()(Trace<json::PositionSettlement> const &) override { FAIL(); }
    void operator()(Trace<json::PositionAdjustRiskLimit> const &) override { FAIL(); }
    void operator()(Trace<json::SymbolOrderChange> const &) override { FAIL(); }
    void operator()(Trace<json::OrderChange> const &event) override {
      ++count;
      auto &[trace_info, order_change] = event;
    }

  } handler;
  core::json::BufferStack buffer{8192, 1};
  auto res = json::Parser::dispatch(handler, MESSAGE_CANCELED, buffer, {}, false);
  CHECK(res == true);
  CHECK(handler.count == 1);
}
