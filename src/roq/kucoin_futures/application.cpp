/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/application.hpp"

#include "roq/kucoin_futures/config.hpp"
#include "roq/kucoin_futures/gateway.hpp"
#include "roq/kucoin_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  auto settings = Settings::create(TYPE);
  Config config;
  auto context = server::create_io_context();
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kucoin_futures
}  // namespace roq
