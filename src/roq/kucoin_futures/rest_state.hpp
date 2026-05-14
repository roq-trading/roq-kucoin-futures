/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace kucoin_futures {

enum class RestState : uint8_t {
  UNDEFINED = 0,
  PUBLIC_TOKEN,
  CONTRACTS,
  DONE,
};

}  // namespace kucoin_futures
}  // namespace roq
