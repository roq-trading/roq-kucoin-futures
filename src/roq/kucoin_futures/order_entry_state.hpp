/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace kucoin_futures {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  PRIVATE_TOKEN,
  ACCOUNT,
  POSITIONS,
  ORDERS,
  FILLS,
  DONE,
};

}  // namespace kucoin_futures
}  // namespace roq
