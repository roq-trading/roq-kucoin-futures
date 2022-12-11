/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/kucoin_futures/json/ticker.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_v2_ticker", "[json_ticker]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/futuresMarket/ticker:BTCUSDTM",)"
                       R"("subject":"ticker",)"
                       R"("sn":83831591,)"
                       R"("data":{)"
                       R"("symbol":"BTCUSDTM",)"
                       R"("bestBidSize":252485,)"
                       R"("bestBidPrice":19180.7,)"
                       R"("bestAskPrice":19187.0,)"
                       R"("bestAskSize":2028,)"
                       R"("ts":1656676964760)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Ticker>(message, buffer_);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/futuresMarket/ticker:BTCUSDTM"sv);
  CHECK(obj.subject == json::Subject::TICKER);
  CHECK(obj.sn == 83831591);
  auto &data = obj.data;
  CHECK(data.symbol == "BTCUSDTM"sv);
  CHECK(data.best_bid_size == 252485_a);
  CHECK(data.best_bid_price == 19180.7_a);
  CHECK(data.best_ask_price == 19187.0_a);
  CHECK(data.best_ask_size == 2028_a);
  // XXX
  auto ts = data.ts * 1000000;
  CHECK(ts == 1656676964760ms);
}

TEST_CASE("json_v1_ticker", "[json_ticker]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/contractMarket/ticker:ETHUSDTM",)"
                       R"("subject":"ticker",)"
                       R"("data":{)"
                       R"("symbol":"ETHUSDTM",)"
                       R"("sequence":1643106002725,)"
                       R"("side":"buy",)"
                       R"("size":1,)"
                       R"("price":1048.50,)"
                       R"("bestBidSize":13559,)"
                       R"("bestBidPrice":"1048.45",)"
                       R"("bestAskPrice":"1048.5",)"
                       R"("tradeId":"62bee48d77a0c43b69994d40",)"
                       R"("ts":1656677517224409126,)"
                       R"("bestAskSize":4892)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Ticker>(message, buffer_);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/contractMarket/ticker:ETHUSDTM"sv);
  CHECK(obj.subject == json::Subject::TICKER);
  CHECK(obj.sn == 0);  // note!
  auto &data = obj.data;
  CHECK(data.symbol == "ETHUSDTM"sv);
  CHECK(data.best_bid_size == 13559_a);
  CHECK(data.best_bid_price == 1048.45_a);
  CHECK(data.best_ask_price == 1048.5_a);
  CHECK(data.ts == 1656677517224409126ns);  // note!
  CHECK(data.best_ask_size == 4892_a);
}
