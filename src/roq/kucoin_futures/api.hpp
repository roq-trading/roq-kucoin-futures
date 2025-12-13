/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/kucoin_futures/settings.hpp"

namespace roq {
namespace kucoin_futures {

struct API final {
  struct {
    std::string_view bullet_public;
    std::string_view contracts_active;
    std::string_view level2_snapshot;
  } rest_public;
  struct {
    std::string_view bullet_private;
    std::string_view get_account_list;
    std::string_view get_position_list;
    std::string_view get_order_list;
    std::string_view get_recent_fills;
    std::string_view add_order;
    std::string_view cancel_order;
    std::string_view cancel_all_orders;
  } rest_private;
  // ws
  // -- public
  std::string_view ticker;
  std::string_view execution;
  std::string_view instrument;
  std::string_view snapshot;
  std::string_view announcement;
  std::string_view level2;
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
