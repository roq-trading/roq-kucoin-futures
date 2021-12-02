/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/application.h"

#include "roq/kucoin_futures/config.h"
#include "roq/kucoin_futures/flags.h"
#include "roq/kucoin_futures/gateway.h"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway"sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kucoin_futures
}  // namespace roq
