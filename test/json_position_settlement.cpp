/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::PositionSettlement;

TEST_CASE("simple", "[json_position_settlement]") {
  /*
  auto message = R"({)"
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
  auto helper = [](value_type const &obj) {
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
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
  */
}
