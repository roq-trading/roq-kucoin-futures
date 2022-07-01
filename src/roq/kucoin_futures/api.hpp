/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

namespace roq {
namespace kucoin_futures {

struct API final {
  int version = {};
  // rest
  // -- public
  std::string_view get_contracts_active;
  std::string_view get_order_book;
  // -- account
  std::string_view get_account_overview;
  std::string_view get_all_position;
  std::string_view get_orders_all_active;
  std::string_view get_orders_historical_trades;
  std::string_view post_order;
  std::string_view delete_order;
  std::string_view delete_orders;
  // ws
  std::string_view get_listen_key;
  std::string_view get_balance;
  std::string_view get_account;
  std::string_view get_open_orders;
  std::string_view order;
  std::string_view all_open_orders;
  std::string_view countdown_cancel_all;
  // factory
  static API create();
};

}  // namespace kucoin_futures
}  // namespace roq
