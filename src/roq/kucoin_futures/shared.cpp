/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/update.hpp"

#include "roq/kucoin_futures/flags.hpp"

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create()}, dispatcher_{dispatcher}, settings{settings},
      rate_limiter{Flags::request_limit(), Flags::request_limit_interval()},
      symbols{Flags::ws_max_subscriptions_per_stream()}, depth_request_queue{Flags::ws_mbp_request_delay()} {
}

}  // namespace kucoin_futures
}  // namespace roq
