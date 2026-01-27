/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/core/symbols.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/market/mbp/sequencer.hpp"

#include "roq/kucoin_futures/api.hpp"
#include "roq/kucoin_futures/settings.hpp"

namespace roq {
namespace kucoin_futures {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 public:
  API const api;
  MarginMode const margin_mode = {};

 private:
  struct {
    std::vector<MBPUpdate> bids, asks;
    auto &clear() {
      bids.clear();
      asks.clear();
      return *this;
    }
    bool empty() const { return std::empty(bids) && std::empty(asks); }
  } mbp;

 public:
  auto &get_mbp() { return mbp.clear(); }

  utils::unordered_map<std::string, market::mbp::Sequencer> mbp_sequencer;

 public:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
  utils::unordered_set<std::string> all_symbols;
  core::TimerQueue<std::string> depth_request_queue;
};

}  // namespace kucoin_futures
}  // namespace roq
