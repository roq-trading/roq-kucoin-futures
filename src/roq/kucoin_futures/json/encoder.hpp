/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"

#include "roq/server/oms/order.hpp"
#include "roq/server/oms/ref_data.hpp"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Encoder final {
  static std::string_view add_order(
      std::string &buffer, CreateOrder const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id, roq::MarginMode);

  static std::string_view ws_add_order(
      std::string &buffer, CreateOrder const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id, roq::MarginMode);
  static std::string_view ws_cancel_order(
      std::string &buffer,
      CancelOrder const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
};

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
