/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/margin_mode.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"

#include "roq/kucoin_futures/json/margin_mode.hpp"
#include "roq/kucoin_futures/json/side.hpp"
#include "roq/kucoin_futures/json/time_in_force.hpp"

namespace roq {

template <>
template <>
std::optional<MarginMode> Map<kucoin_futures::json::MarginMode>::helper() const;

template <>
template <>
std::optional<Side> Map<kucoin_futures::json::Side>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<kucoin_futures::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<kucoin_futures::json::Side> Map<Side>::helper() const;

}  // namespace roq
