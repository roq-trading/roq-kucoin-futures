/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/server.hpp"

namespace roq {
namespace kucoin_futures {

struct OrderEntry {
  OrderEntry() = default;

  OrderEntry(OrderEntry const &) = delete;

  virtual ~OrderEntry() = default;

  virtual bool ready() const = 0;

  virtual uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) = 0;
  virtual uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) = 0;
  virtual uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) = 0;

  virtual uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) = 0;
};

}  // namespace kucoin_futures
}  // namespace roq
