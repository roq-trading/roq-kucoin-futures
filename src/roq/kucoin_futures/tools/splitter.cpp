/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/splitter.hpp"

#include "roq/utils/charconv/from_chars.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace tools {

namespace {
Side to_side(std::string_view const &text) {
  if (text.compare("buy"sv) == 0) {
    return Side::BUY;
  }
  if (text.compare("sell"sv) == 0) {
    return Side::SELL;
  }
  return {};
}
}  // namespace

std::tuple<Side, double, double> split(std::string_view const &change) {
  auto sep_1 = change.find_first_of(',');
  if (sep_1 != change.npos) {
    auto sep_2 = change.find_first_of(',', sep_1 + 1);
    if (sep_2 != change.npos) {
      auto price = utils::charconv::from_chars<double>(change.substr(0, sep_1));
      auto side = to_side(change.substr(sep_1 + 1, sep_2 - sep_1 - 1));
      auto quantity = utils::charconv::from_chars<double>(change.substr(sep_2 + 1));
      return {side, price, quantity};
    }
  }
  return {};
}

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
