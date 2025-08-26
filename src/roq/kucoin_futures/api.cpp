/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kucoin_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  return {
      .rest_public{
          .bullet_public = "/api/v1/bullet-public"sv,
          .contracts_active = "/api/v1/contracts/active"sv,
          .level2_snapshot = "/api/v1/level2/snapshot"sv,
      },
      .rest_private{
          .bullet_private = "/api/v1/bullet-private"sv,
          .account_overview = "/api/v2/account-overview"sv,
          .all_position = "/api/v2/all-position"sv,
          .orders_all_active = "/api/v2/orders/all-active"sv,
          .orders_historical_trades = "/api/v2/orders/historical-trades"sv,
          .order = "/api/v2/order"sv,
          .orders = "/api/v2/orders"sv,
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
