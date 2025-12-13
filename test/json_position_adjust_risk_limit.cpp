/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::PositionAdjustRiskLimit;

TEST_CASE("simple", "[json_position_adjust_risk_limit]") {
  /*
  auto message = R"({)"
                 R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                 R"("topic": "/contract/position:XBTUSDM",)"
                 R"("subject": "position.adjustRiskLimit",)"
                 R"("data": {)"
                 R"(})"
                 R"(})";

  auto helper = [](value_type const &obj) {
    CHECK(obj.user_id == "xbc453tg732eba53a88ggyt8c"sv);
    CHECK(obj.topic == "/contract/position:XBTUSDM"sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
  */
}
