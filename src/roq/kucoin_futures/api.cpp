/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .rest_public{
          .bullet_public = "/api/v1/bullet-public"sv,
          .contracts_active = "/api/v1/contracts/active"sv,
          .level2_snapshot = "/api/v1/level2/snapshot"sv,
      },
      .rest_private{
          .bullet_private = "/api/v1/bullet-private"sv,
          .get_account_list = "/api/v1/account-overview"sv,
          .get_position_list = "/api/v1/positions"sv,
          .get_order_list = "/api/v1/orders"sv,
          .get_recent_fills = "/api/v1/recentFills"sv,
          .add_order = "/api/v1/orders"sv,
          .cancel_order = "/api/v1/orders"sv,
          .cancel_all_orders = "/api/v3/orders"sv,
      },
      // ws
      // -- public
      .ticker = "/contractMarket/tickerV2"sv,
      .execution = "/contractMarket/execution"sv,
      .instrument = "/contract/instrument"sv,
      .snapshot = "/contractMarket/snapshot"sv,
      .announcement = "/contract/announcement"sv,
      .level2 = "/contractMarket/level2"sv,
      // account
      .get_balance = "/fapi/v2/balance"sv,
      .get_account = "/fapi/v2/account"sv,
      .get_open_orders = "/fapi/v1/openOrders"sv,
      .order = "/fapi/v1/order"sv,
      .all_open_orders = "/fapi/v1/allOpenOrders"sv,
      .countdown_cancel_all = "/fapi/v1/countdownCancelAll"sv,
  };
}

}  // namespace kucoin_futures
}  // namespace roq
