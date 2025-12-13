/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace roq {
namespace kucoin_futures {

struct Collector final {
  std::chrono::nanoseconds created = {};
  bool ready = false;
  std::vector<std::pair<int64_t, std::string>> history;
  uint32_t retries = {};
  int64_t last_sequence = {};
};

}  // namespace kucoin_futures
}  // namespace roq
