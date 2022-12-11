/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/kucoin_futures/json/funding_rate.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_v2_funding_rate", "[json_funding_rate]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/futuresContract/fundingRate:BTCUSDTM",)"
                       R"("subject":"funding.rate",)"
                       R"("data":{)"
                       R"( "granularity":60000,)"
                       R"("fundingRate":0.000100,)"
                       R"("timestamp":1656678420000)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::FundingRate>(message, buffer_);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/futuresContract/fundingRate:BTCUSDTM"sv);
  CHECK(obj.subject == json::Subject::FUNDING_RATE);
  auto &data = obj.data;
  CHECK(data.granularity == 60000_a);
  CHECK(data.funding_rate == 0.000100_a);
  CHECK(data.timestamp == 1656678420000ms);
}

TEST_CASE("json_v1_funding_rate", "[json_funding_rate]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/contract/instrument:ETHUSDTM",)"
                       R"("subject":"funding.rate",)"
                       R"("data":{)"
                       R"("granularity":60000,)"
                       R"("fundingRate":-0.000248,)"
                       R"("timestamp":1656678720000)"
                       R"(})"
                       R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::FundingRate>(message, buffer_);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/contract/instrument:ETHUSDTM"sv);
  CHECK(obj.subject == json::Subject::FUNDING_RATE);
  auto &data = obj.data;
  CHECK(data.granularity == 60000_a);
  CHECK(data.funding_rate == -0.000248_a);
  CHECK(data.timestamp == 1656678720000ms);
}
