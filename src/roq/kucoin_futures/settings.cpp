/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/settings.hpp"

#include "roq/logging.hpp"

#include "roq/kucoin_futures/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

Settings::Settings(args::Parser const &args, server::Type type)
    : server::flags::Settings{args, type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, flags::Flags::api()},
      exchange{flags::Flags::exchange()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace kucoin_futures
}  // namespace roq
