/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/create_order.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Encoder final {
  static std::string_view add_order(std::string &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id, roq::MarginMode);
};

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
