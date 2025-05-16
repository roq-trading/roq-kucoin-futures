/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// kucoin_futures::json => roq

// kucoin_futures::json::Side => roq::Side

template <>
template <>
constexpr Helper<kucoin_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum kucoin_futures::json::Side::type_t;
    case _UNDEFINED:
      return roq::Side::UNDEFINED;
    case _UNKNOWN:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::_UNDEFINED}} == roq::Side::UNDEFINED);
static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{kucoin_futures::json::Side{kucoin_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<kucoin_futures::json::Side>::helper() const {
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
      return kucoin_futures::json::Side::_UNDEFINED;
    case BUY:
      return kucoin_futures::json::Side::BUY;
    case SELL:
      return kucoin_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == kucoin_futures::json::Side{kucoin_futures::json::Side::_UNDEFINED});
static_assert(Helper{roq::Side::BUY} == kucoin_futures::json::Side{kucoin_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == kucoin_futures::json::Side{kucoin_futures::json::Side::SELL});

template <>
template <>
std::optional<kucoin_futures::json::Side> Map<roq::Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq
