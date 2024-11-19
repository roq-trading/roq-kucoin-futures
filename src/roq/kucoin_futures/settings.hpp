/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/kucoin_futures/flags/flags.hpp"
#include "roq/kucoin_futures/flags/mbp.hpp"
#include "roq/kucoin_futures/flags/misc.hpp"
#include "roq/kucoin_futures/flags/request.hpp"
#include "roq/kucoin_futures/flags/rest.hpp"
#include "roq/kucoin_futures/flags/ws.hpp"

namespace roq {
namespace kucoin_futures {

struct Settings final : public server::flags::Settings {
  explicit Settings(args::Parser const &);

  std::string_view exchange;

  flags::Misc misc;
  flags::REST rest;
  flags::WS ws;
  flags::MBP mbp;
  flags::Request request;

 private:
  Settings(args::Parser const &, flags::Flags const &);
};

}  // namespace kucoin_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::kucoin_futures::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::kucoin_futures::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(misc={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(mbp={}, )"
        R"(request={}, )"
        R"(server={})"
        R"(}})"sv,
        value.exchange,
        value.misc,
        value.rest,
        value.ws,
        value.mbp,
        value.request,
        static_cast<roq::server::Settings const &>(value));
  }
};
