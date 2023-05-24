/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/position_settlement.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_position_settlement_example", "[json_position_settlement]") {
  auto const message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contract/position:XBTUSDM",)"
                       R"("subject": "position.settlement",)"
                       R"("data": {)"
                       R"("fundingTime": 1551770400000,)"
                       R"("qty": 100,)"
                       R"("markPrice": 3610.85,)"
                       R"("fundingRate": -0.002966,)"
                       R"("fundingFee": -296,)"
                       R"("ts": 1547697294838004923,)"
                       R"("settleCurrency": "XBT")"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::PositionSettlement::create(message, buffer);
  CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
  CHECK(obj.topic == "/contract/position:XBTUSDM"sv);
  CHECK(obj.subject == json::Subject::POSITION_SETTLEMENT);
  auto &data = obj.data;
  CHECK(data.funding_time == 1551770400000ms);
  CHECK(data.qty == 100.0_a);
  CHECK(data.mark_price == 3610.85_a);
  CHECK(data.funding_rate == -0.002966_a);
  CHECK(data.funding_fee == -296.0_a);
  CHECK(data.ts == 1547697294838004923ns);
  CHECK(data.settle_currency == "XBT"sv);
}
