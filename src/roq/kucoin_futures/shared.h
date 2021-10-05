/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "roq/api.h"
#include "roq/server.h"

#include "roq/core/memory.h"

#include "roq/kucoin_futures/collector.h"

namespace roq {
namespace kucoin_futures {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(const Shared &) = delete;

  auto discard_symbol(const std::string_view &name) const {
    return dispatcher_.discard_symbol(name);
  }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  template <typename F>
  bool can_request(std::chrono::nanoseconds now, F callback) {
    auto result = can_request_helper(now);
    if (result)
      callback();
    return result;
  }

 protected:
  bool can_request_helper(std::chrono::nanoseconds now);

 public:
  core::page_aligned_vector<MBPUpdate> bids, asks, final_bids, final_asks;

  absl::flat_hash_map<std::string, Collector> mbp_collector;

 private:
  server::Dispatcher &dispatcher_;

  std::deque<std::chrono::nanoseconds> request_history_;
  bool request_is_blocked_ = false;
};

}  // namespace kucoin_futures
}  // namespace roq
