/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// kucoin_futures::json => roq

// kucoin_futures::json::MarginMode => roq::MarginMode

template <>
template <>
constexpr Helper<kucoin_futures::json::MarginMode>::operator std::optional<roq::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::MarginMode::type_t;
    case UNDEFINED_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::MarginMode::UNDEFINED;
    case CROSS:
      return roq::MarginMode::CROSS;
    case ISOLATED:
      return roq::MarginMode::ISOLATED;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::UNDEFINED_INTERNAL}} == roq::MarginMode::UNDEFINED);
static_assert(Helper{kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::CROSS}} == roq::MarginMode::CROSS);
static_assert(Helper{kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::ISOLATED}} == roq::MarginMode::ISOLATED);

template <>
template <>
std::optional<roq::MarginMode> Map<kucoin_futures::json::MarginMode>::helper() const {
  return Helper{args_};
}

// kucoin_futures::json::Side => roq::Side

template <>
template <>
constexpr Helper<kucoin_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<kucoin_futures::json::Side>::helper() const {
  return Helper{args_};
}

// kucoin_futures::json::TimeInForce => roq::TimeInForce

template <>
template <>
constexpr Helper<kucoin_futures::json::TimeInForce>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::TimeInForce::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case GTC:
      return roq::TimeInForce::GTC;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);

template <>
template <>
std::optional<roq::TimeInForce> Map<kucoin_futures::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq => kucoin_futures::json

// roq::Side ==> kucoin_futures::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<kucoin_futures::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return kucoin_futures::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return kucoin_futures::json::Side::BUY;
    case SELL:
      return kucoin_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == kucoin_futures::json::Side{kucoin_futures::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == kucoin_futures::json::Side{kucoin_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == kucoin_futures::json::Side{kucoin_futures::json::Side::SELL});

template <>
template <>
std::optional<kucoin_futures::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq
