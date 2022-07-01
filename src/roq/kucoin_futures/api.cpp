/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/api.hpp"

#include "roq/exceptions.hpp"

#include "roq/kucoin_futures/flags.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

API API::create() {
  auto api = Flags::api();
  if (api.compare("v1"sv) == 0) {
    return {
        .version = 1,
        // rest
        // -- public
        .get_contracts_active = "/api/v1/contracts/active"sv,
        .get_order_book = "/api/v1/level2/snapshot"sv,
        // -- account
        .get_account_overview = "/api/v1/account-overview"sv,
        .get_all_position = "/api/v1/positions"sv,
        .get_orders_all_active = "/api/v1/orders"sv,  // note! query params
        .get_orders_historical_trades = "/api/v1/fills"sv,
        .post_order = "/api/v1/orders"sv,
        .delete_order = "/api/v1/orders"sv,  // note! append order id
        .delete_orders = "/api/v1/orders"sv,
        // ws
        .get_listen_key = "/dapi/v1/listenKey"sv,
        .get_balance = "/dapi/v1/balance"sv,
        .get_account = "/dapi/v1/account"sv,
        .get_open_orders = "/dapi/v1/openOrders"sv,
        .order = "/dapi/v1/order"sv,
        .all_open_orders = "/dapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/dapi/v1/countdownCancelAll"sv,
    };
  }
  if (api.compare("v2"sv) == 0) {
    return {
        .version = 2,
        // rest
        // -- public
        .get_contracts_active = "/api/v2/contracts/active"sv,
        .get_order_book = "/api/v2/order-book"sv,
        // -- account
        .get_account_overview = "/api/v2/account-overview"sv,
        .get_all_position = "/api/v2/all-position"sv,
        .get_orders_all_active = "/api/v2/orders/all-active"sv,
        .get_orders_historical_trades = "/api/v2/orders/historical-trades"sv,
        .post_order = "/api/v2/order"sv,
        .delete_order = "/api/v2/order"sv,
        .delete_orders = "/api/v2/orders"sv,
        // ws
        .get_listen_key = "/fapi/v1/listenKey"sv,
        .get_balance = "/fapi/v2/balance"sv,
        .get_account = "/fapi/v2/account"sv,
        .get_open_orders = "/fapi/v1/openOrders"sv,
        .order = "/fapi/v1/order"sv,
        .all_open_orders = "/fapi/v1/allOpenOrders"sv,
        .countdown_cancel_all = "/fapi/v1/countdownCancelAll"sv,
    };
  }
  throw RuntimeError(R"(Unknown api="{}")"sv, api);
}

}  // namespace kucoin_futures
}  // namespace roq
