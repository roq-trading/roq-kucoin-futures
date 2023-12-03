/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/kucoin_futures/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/update.hpp"

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, dispatcher_{dispatcher}, settings{settings},
      rate_limiter{settings.common.request_limit, settings.common.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream}, depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace kucoin_futures
}  // namespace roq
