/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <utility>
#include <vector>

#include "roq/api.h"
#include "roq/server.h"

#include "roq/core/memory.h"

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

 public:
  core::page_aligned_vector<MBPUpdate> bids, asks, final_bids, final_asks;

  absl::flat_hash_map<std::string, std::pair<bool, std::vector<std::pair<int64_t, std::string>>>>
      mbp_collector;

 private:
  server::Dispatcher &dispatcher_;
};

}  // namespace kucoin_futures
}  // namespace roq
