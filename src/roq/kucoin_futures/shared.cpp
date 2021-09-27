/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/shared.h"

#include "roq/kucoin_futures/flags.h"

namespace roq {
namespace kucoin_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), dispatcher_(dispatcher) {
}

}  // namespace kucoin_futures
}  // namespace roq
