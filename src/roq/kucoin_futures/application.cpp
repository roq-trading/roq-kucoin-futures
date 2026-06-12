/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/application.hpp"

#include "roq/kucoin_futures/flags/settings.hpp"

#include "roq/kucoin_futures/gateway/config.hpp"
#include "roq/kucoin_futures/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<gateway::Controller>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kucoin_futures
}  // namespace roq
