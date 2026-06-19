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

#include "roq/kucoin_futures/gateway/api.hpp"
#include "roq/kucoin_futures/gateway/settings.hpp"

namespace roq {
namespace kucoin_futures {
namespace gateway {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  server::Dispatcher &dispatcher;

  Settings const &settings;
  API const api;
  MarginMode const margin_mode = {};

  core::limit::RateLimiter rate_limiter;

  core::Symbols symbols;
  utils::unordered_set<std::string> all_symbols;

  core::TimerQueue<std::string> depth_request_queue;

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

  std::vector<MBPUpdate> final_bids, final_asks;

  utils::unordered_map<std::string, market::mbp::Sequencer> mbp_sequencer;
};

}  // namespace gateway
}  // namespace kucoin_futures
}  // namespace roq
