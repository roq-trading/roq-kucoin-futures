/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>
#include <tuple>

#include "roq/api.hpp"

namespace roq {
namespace kucoin_futures {
namespace tools {

extern std::tuple<Side, double, double> split(std::string_view const &change);

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
