/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/map.hpp"

#include "roq/liquidity.hpp"
#include "roq/margin_mode.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/position_effect.hpp"
#include "roq/side.hpp"
#include "roq/time_in_force.hpp"

#include "roq/kucoin_futures/protocol/json/liquidity.hpp"
#include "roq/kucoin_futures/protocol/json/margin_mode.hpp"
#include "roq/kucoin_futures/protocol/json/order_status.hpp"
#include "roq/kucoin_futures/protocol/json/order_type.hpp"
#include "roq/kucoin_futures/protocol/json/position_side.hpp"
#include "roq/kucoin_futures/protocol/json/side.hpp"
#include "roq/kucoin_futures/protocol/json/time_in_force.hpp"

namespace roq {

template <>
template <>
std::optional<Liquidity> Map<kucoin_futures::protocol::json::Liquidity>::helper() const;

template <>
template <>
std::optional<MarginMode> Map<kucoin_futures::protocol::json::MarginMode>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<kucoin_futures::protocol::json::OrderStatus>::helper() const;

template <>
template <>
std::optional<OrderType> Map<kucoin_futures::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<PositionEffect> Map<kucoin_futures::protocol::json::PositionSide, kucoin_futures::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<Side> Map<kucoin_futures::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<kucoin_futures::protocol::json::TimeInForce>::helper() const;

// ===

template <>
template <>
std::optional<kucoin_futures::protocol::json::MarginMode> Map<MarginMode>::helper() const;

template <>
template <>
std::optional<kucoin_futures::protocol::json::OrderType> Map<OrderType>::helper() const;

template <>
template <>
std::optional<kucoin_futures::protocol::json::Side> Map<Side>::helper() const;

template <>
template <>
std::optional<kucoin_futures::protocol::json::TimeInForce> Map<TimeInForce>::helper() const;

}  // namespace roq
