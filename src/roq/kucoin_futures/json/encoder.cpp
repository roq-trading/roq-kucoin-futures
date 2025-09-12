/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kucoin_futures/json/encoder.hpp"

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

std::string_view Encoder::add_order(std::string &buffer, CreateOrder const &create_order, server::oms::Order const &, std::string_view const &request_id) {
  buffer.clear();
  auto side = map(create_order.side).template get<json::Side>();
  auto type = "limit"sv;  // limit or market
  auto leverage = 1;
  auto remark = ""sv;
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  auto time_in_force = "GTC"sv;  // GTC, IOC
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("clientOid":"{}",)"
      R"("side":"{}",)"
      R"("symbol":"{}",)"
      R"("type":"{}",)"
      R"("leverage":{},)"
      R"("remark":"{}",)"
      R"("reduceOnly":{},)"
      R"("price":{},)"
      R"("size":{},)"
      R"("timeInForce":"{}")"
      R"(}})"sv,
      request_id,
      side.as_raw_text(),
      create_order.symbol,
      type,
      leverage,
      remark,
      reduce_only,
      create_order.price,
      create_order.quantity,
      time_in_force);
  return buffer;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
