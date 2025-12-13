/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::OrderChange;

TEST_CASE("open", "[json_order_change]") {
  auto message = R"({)"
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.type == json::Type::MESSAGE);
    CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
    CHECK(obj.subject == json::Subject::ORDER_CHANGE);
    CHECK(obj.channel_type == "private"sv);
    /*
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
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("canceled", "[json_order_change]") {
  auto message = R"({)"
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.type == json::Type::MESSAGE);
    CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
    CHECK(obj.subject == json::Subject::ORDER_CHANGE);
    CHECK(obj.channel_type == "private"sv);
    /*
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
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("match", "[json_order_change]") {
  auto message = R"({)"
                 R"("topic":"/contractMarket/tradeOrders",)"
                 R"("type":"message",)"
                 R"("subject":"orderChange",)"
                 R"("userId":"67f914adc8d0110001ca099e",)"
                 R"("channelType":"private",)"
                 R"("data":{)"
                 R"("symbol":"XBTUSDTM",)"
                 R"("orderType":"limit",)"
                 R"("side":"buy",)"
                 R"("canceledSize":"0",)"
                 R"("orderId":"359065448081260544",)"
                 R"("positionSide":"BOTH",)"
                 R"("liquidity":"taker",)"
                 R"("marginMode":"CROSS",)"
                 R"("type":"match",)"
                 R"("feeType":"takerFee",)"
                 R"("orderTime":1758421075935374001,)"
                 R"("size":"1",)"
                 R"("filledSize":"1",)"
                 R"("price":"115600",)"
                 R"("matchPrice":"115549.8",)"
                 R"("matchSize":"1",)"
                 R"("remainSize":"0",)"
                 R"("tradeId":"1873921404162",)"
                 R"("clientOid":"hQACydAB7TQAAQAAAAAA",)"
                 R"("tradeType":"trade",)"
                 R"("status":"match",)"
                 R"("ts":1758421075974000000)"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.type == json::Type::MESSAGE);
    CHECK(obj.topic == "/contractMarket/tradeOrders"sv);
    CHECK(obj.subject == json::Subject::ORDER_CHANGE);
    CHECK(obj.channel_type == "private"sv);
    /*
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
    CHECK(data.match_size == 0.0_a);
    CHECK(data.trade_id == "5ce24c16b210233c36eexxxx"sv);
    CHECK(data.client_oid == "5ce24c16b210233c36ee321d"sv);
    CHECK(data.order_time == 1545914149935808589ns);
    CHECK(data.old_size == 15000.0_a);
    CHECK(data.liquidity == json::Liquidity::MAKER);
    CHECK(data.ts == 1545914149935808589ns);
    */
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
