/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// kucoin_futures::json => roq

// kucoin_futures::json::Liquidity => roq::Liquidity

template <>
template <>
constexpr Helper<kucoin_futures::json::Liquidity>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::Liquidity::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case TAKER:
      return roq::Liquidity::TAKER;
    case MAKER:
      return roq::Liquidity::MAKER;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::Liquidity{kucoin_futures::json::Liquidity::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kucoin_futures::json::Liquidity{kucoin_futures::json::Liquidity::TAKER}} == roq::Liquidity::TAKER);
static_assert(Helper{kucoin_futures::json::Liquidity{kucoin_futures::json::Liquidity::MAKER}} == roq::Liquidity::MAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<kucoin_futures::json::Liquidity>::helper() const {
  return Helper{args_};
}

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

// kucoin_futures::json::OrderStatus => roq::OrderStatus

template <>
template <>
constexpr Helper<kucoin_futures::json::OrderStatus>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::OrderStatus::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case OPEN:
      return roq::OrderStatus::WORKING;
    case MATCH:
      return roq::OrderStatus::WORKING;
    case DONE:
      return roq::OrderStatus::COMPLETED;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::OrderStatus{kucoin_futures::json::OrderStatus::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{kucoin_futures::json::OrderStatus{kucoin_futures::json::OrderStatus::OPEN}} == roq::OrderStatus::WORKING);
static_assert(Helper{kucoin_futures::json::OrderStatus{kucoin_futures::json::OrderStatus::MATCH}} == roq::OrderStatus::WORKING);
static_assert(Helper{kucoin_futures::json::OrderStatus{kucoin_futures::json::OrderStatus::DONE}} == roq::OrderStatus::COMPLETED);

template <>
template <>
std::optional<roq::OrderStatus> Map<kucoin_futures::json::OrderStatus>::helper() const {
  return Helper{args_};
}

// kucoin_futures::json::OrderType => roq::OrderType

template <>
template <>
constexpr Helper<kucoin_futures::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case MARKET:
      return roq::OrderType::MARKET;
    case LIMIT:
      return roq::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::MARKET}} == roq::OrderType::MARKET);
static_assert(Helper{kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<kucoin_futures::json::OrderType>::helper() const {
  return Helper{args_};
}

// {kucoin_futures::json::PositionSide, kucoin_futures::json::Side} => roq::PositionEffect

template <>
template <>
constexpr Helper<kucoin_futures::json::PositionSide, kucoin_futures::json::Side>::operator std::optional<roq::PositionEffect>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::PositionSide::type_t;
    case UNDEFINED_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::PositionEffect::UNDEFINED;
    case BOTH:
      return roq::PositionEffect::UNDEFINED;
    case LONG:
      switch (std::get<1>(args_)) {
        using enum kucoin_futures::json::Side::type_t;
        case UNDEFINED_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case UNKNOWN_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case BUY:
          return roq::PositionEffect::OPEN;
        case SELL:
          return roq::PositionEffect::CLOSE;
      }
      break;
    case SHORT:
      switch (std::get<1>(args_)) {
        using enum kucoin_futures::json::Side::type_t;
        case UNDEFINED_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case UNKNOWN_INTERNAL:
          return roq::PositionEffect::UNDEFINED;
        case BUY:
          return roq::PositionEffect::CLOSE;
        case SELL:
          return roq::PositionEffect::OPEN;
      }
      break;
  }
  return {};
}

static_assert(
    Helper{
        kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::UNDEFINED_INTERNAL},
        kucoin_futures::json::Side{kucoin_futures::json::Side::UNDEFINED_INTERNAL}} == roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{
        kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::BOTH},
        kucoin_futures::json::Side{kucoin_futures::json::Side::UNDEFINED_INTERNAL}} == roq::PositionEffect::UNDEFINED);
static_assert(
    Helper{kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::LONG}, kucoin_futures::json::Side{kucoin_futures::json::Side::BUY}} ==
    roq::PositionEffect::OPEN);
static_assert(
    Helper{kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::LONG}, kucoin_futures::json::Side{kucoin_futures::json::Side::SELL}} ==
    roq::PositionEffect::CLOSE);
static_assert(
    Helper{kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::SHORT}, kucoin_futures::json::Side{kucoin_futures::json::Side::BUY}} ==
    roq::PositionEffect::CLOSE);
static_assert(
    Helper{kucoin_futures::json::PositionSide{kucoin_futures::json::PositionSide::SHORT}, kucoin_futures::json::Side{kucoin_futures::json::Side::SELL}} ==
    roq::PositionEffect::OPEN);

template <>
template <>
std::optional<roq::PositionEffect> Map<kucoin_futures::json::PositionSide, kucoin_futures::json::Side>::helper() const {
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
    case IOC:
      return roq::TimeInForce::IOC;
    case FOK:
      return roq::TimeInForce::FOK;
    case GTT:
      return roq::TimeInForce::UNDEFINED;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::IOC}} == roq::TimeInForce::IOC);
static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::GTT}} == roq::TimeInForce::UNDEFINED);

template <>
template <>
std::optional<roq::TimeInForce> Map<kucoin_futures::json::TimeInForce>::helper() const {
  return Helper{args_};
}

// roq => kucoin_futures::json

// roq::MarginMode ==> kucoin_futures::json::MarginMode

template <>
template <>
constexpr Helper<roq::MarginMode>::operator std::optional<kucoin_futures::json::MarginMode>() const {
  switch (std::get<0>(args_)) {
    using enum roq::MarginMode;
    case UNDEFINED:
      return kucoin_futures::json::MarginMode::ISOLATED;  // note! default
    case ISOLATED:
      return kucoin_futures::json::MarginMode::ISOLATED;
    case CROSS:
      return kucoin_futures::json::MarginMode::CROSS;
    case PORTFOLIO:
      return kucoin_futures::json::MarginMode::ISOLATED;  // note! default
  }
  return {};
}

static_assert(Helper{roq::MarginMode::UNDEFINED} == kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::ISOLATED});
static_assert(Helper{roq::MarginMode::ISOLATED} == kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::ISOLATED});
static_assert(Helper{roq::MarginMode::CROSS} == kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::CROSS});
static_assert(Helper{roq::MarginMode::PORTFOLIO} == kucoin_futures::json::MarginMode{kucoin_futures::json::MarginMode::ISOLATED});

template <>
template <>
std::optional<kucoin_futures::json::MarginMode> Map<roq::MarginMode>::helper() const {
  return Helper{args_};
}

// roq::OrderType ==> kucoin_futures::json::OrderType

template <>
template <>
constexpr Helper<roq::OrderType>::operator std::optional<kucoin_futures::json::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum roq::OrderType;
    case UNDEFINED:
      return kucoin_futures::json::OrderType::UNDEFINED_INTERNAL;
    case MARKET:
      return kucoin_futures::json::OrderType::MARKET;
    case LIMIT:
      return kucoin_futures::json::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{roq::OrderType::UNDEFINED} == kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::UNDEFINED_INTERNAL});
static_assert(Helper{roq::OrderType::MARKET} == kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::MARKET});
static_assert(Helper{roq::OrderType::LIMIT} == kucoin_futures::json::OrderType{kucoin_futures::json::OrderType::LIMIT});

template <>
template <>
std::optional<kucoin_futures::json::OrderType> Map<roq::OrderType>::helper() const {
  return Helper{args_};
}

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

// roq::TimeInForce ==> kucoin_futures::json::TimeInForce

template <>
template <>
constexpr Helper<roq::TimeInForce>::operator std::optional<kucoin_futures::json::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum roq::TimeInForce;
    case UNDEFINED:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFD:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTC:
      return kucoin_futures::json::TimeInForce::GTC;
    case OPG:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case IOC:
      return kucoin_futures::json::TimeInForce::IOC;
    case FOK:
      return kucoin_futures::json::TimeInForce::FOK;
    case GTX:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GTD:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_THE_CLOSE:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_THROUGH_CROSSING:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case AT_CROSSING:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GOOD_FOR_TIME:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFA:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
    case GFM:
      return kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL;
  }
  return {};
}

static_assert(Helper{roq::TimeInForce::UNDEFINED} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFD} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTC} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::GTC});
static_assert(Helper{roq::TimeInForce::OPG} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::IOC} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::IOC});
static_assert(Helper{roq::TimeInForce::FOK} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::FOK});
static_assert(Helper{roq::TimeInForce::GTX} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GTD} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_THE_CLOSE} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_THROUGH_CROSSING} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::AT_CROSSING} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GOOD_FOR_TIME} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFA} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});
static_assert(Helper{roq::TimeInForce::GFM} == kucoin_futures::json::TimeInForce{kucoin_futures::json::TimeInForce::UNDEFINED_INTERNAL});

template <>
template <>
std::optional<kucoin_futures::json::TimeInForce> Map<roq::TimeInForce>::helper() const {
  return Helper{args_};
}

}  // namespace roq
