/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/datetime.hpp"

#include "roq/kucoin_futures/json/mark_index_price.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_v2_mark_index_price", "[json_mark_price]") {
  auto const message = R"({)"
                       R"("type":"message",)"
                       R"("topic":"/futuresContract/markPrice",)"
                       R"("subject":"mark.index.price",)"
                       R"("data":{)"
                       R"("symbol":"XRPUSDM",)"
                       R"("markPrice":0.3162,)"
                       R"("indexPrice":0.3165,)"
                       R"("granularity":1000,)"
                       R"("timestamp":1656668191000)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::MarkIndexPrice::create(message, buffer);
  CHECK(obj.type == json::Type::MESSAGE);
  CHECK(obj.topic == "/futuresContract/markPrice"sv);
  CHECK(obj.subject == json::Subject::MARK_INDEX_PRICE);
  auto &data = obj.data;
  CHECK(data.symbol == "XRPUSDM"sv);
  CHECK(data.mark_price == 0.3162_a);
  CHECK(data.index_price == 0.3165_a);
  CHECK(data.granularity == 1000.0_a);
  CHECK(data.timestamp == 1656668191000ms);
}
