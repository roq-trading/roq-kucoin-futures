/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/enum.hpp"
#include "roq/utils/update.hpp"

namespace roq {
namespace kucoin_futures {

// === HELPERS ===

namespace {
auto create_margin_mode(auto &margin_mode) {
  return utils::parse_enum<MarginMode>(margin_mode);
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, margin_mode{create_margin_mode(settings.margin_mode)}, dispatcher_{dispatcher}, settings{settings},
      rate_limiter{settings.request.limit, settings.request.limit_interval}, symbols{settings.ws.max_subscriptions_per_stream},
      depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace kucoin_futures
}  // namespace roq
