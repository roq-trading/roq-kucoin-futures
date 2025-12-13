/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/json/utils.hpp"

#include "roq/kucoin_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

Error guess_error([[maybe_unused]] int32_t code) {
  return Error::UNKNOWN;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
