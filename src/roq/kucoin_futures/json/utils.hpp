/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/charconv/datetime.hpp"

#include "roq/kucoin_futures/json/side.hpp"

namespace roq {
namespace kucoin_futures {
namespace json {

template <typename T>
inline void update(T &result, const core::json::Value &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, const core::json::Value &value) {
  return std::visit(
      overloaded{
          [&](const core::json::Null &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::milliseconds{value}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<int64_t>(value)}; },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::Object &) { throw std::bad_cast(); },
          [](const core::json::Array &) { throw std::bad_cast(); },
      },
      value);
}

template <>
inline void update(std::chrono::nanoseconds &result, const core::json::Value &value) {
  return std::visit(
      overloaded{
          [&](const core::json::Null &) { result = std::chrono::nanoseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::nanoseconds{value}; },
          [&](double value) { result = std::chrono::nanoseconds{static_cast<int64_t>(value)}; },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::Object &) { throw std::bad_cast(); },
          [](const core::json::Array &) { throw std::bad_cast(); },
      },
      value);
}

inline std::string_view strip_symbol_from_topic(const std::string_view &topic) {
  auto pos = topic.find_last_of(':');
  return pos == topic.npos ? topic : topic.substr(pos + 1);
}

inline roq::Side map(json::Side value) {
  switch (value) {
    using enum json::Side::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

inline json::Side map(roq::Side value) {
  switch (value) {
    using enum roq::Side;
    case UNDEFINED:
      break;
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  return {};
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
