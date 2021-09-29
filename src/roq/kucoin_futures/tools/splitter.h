/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>
#include <tuple>

#include "roq/api.h"

namespace roq {
namespace kucoin_futures {
namespace tools {

extern std::tuple<Side, double, double> split(const std::string_view &change);

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
