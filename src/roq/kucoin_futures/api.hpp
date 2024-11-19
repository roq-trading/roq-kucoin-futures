/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/kucoin_futures/settings.hpp"

namespace roq {
namespace kucoin_futures {

struct API final {
  int version = {};
  struct {
    std::string_view bullet_public;
    std::string_view contracts_active;
    std::string_view level2_snapshot;
  } rest_public;
  struct {
    std::string_view bullet_private;
    std::string_view account_overview;
    std::string_view all_position;
    std::string_view orders_all_active;
    std::string_view orders_historical_trades;
    std::string_view order;
    std::string_view orders;
  } rest_private;
  // ws
  // -- public
  std::string_view ticker;
  std::string_view execution;
  std::string_view level2;
  std::string_view mark_price;
  std::string_view funding_rate;
  // -- account
  std::string_view get_balance;
  std::string_view get_account;
  std::string_view get_open_orders;
  std::string_view order;
  std::string_view all_open_orders;
  std::string_view countdown_cancel_all;

  // factory
  static API create(Settings const &);
};

}  // namespace kucoin_futures
}  // namespace roq
