/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/kucoin_futures/json/side.hpp"

#include "roq/side.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Side> Map<kucoin_futures::json::Side>::helper() const;

// ===

template <>
template <>
std::optional<kucoin_futures::json::Side> Map<Side>::helper() const;

}  // namespace roq
