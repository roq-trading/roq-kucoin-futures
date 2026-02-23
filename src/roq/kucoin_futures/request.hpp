/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

namespace roq {
namespace kucoin_futures {

struct Request final {
  // private-token
  std::chrono::nanoseconds request_private_token = {};
  std::chrono::nanoseconds respond_private_token = {};
};

}  // namespace kucoin_futures
}  // namespace roq
