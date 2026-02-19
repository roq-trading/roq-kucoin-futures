/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace roq {
namespace kucoin_futures {

struct PrivateToken final {
  std::string_view account;
  std::string_view uri;
  std::string_view query;
  std::chrono::nanoseconds ping_frequency = {};
};

}  // namespace kucoin_futures
}  // namespace roq
