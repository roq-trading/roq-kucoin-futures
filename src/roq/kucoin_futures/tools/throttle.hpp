/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

#include <fmt/format.h>

#include "roq/web/rest/interceptor.hpp"

#include "roq/web/socket/interceptor.hpp"

#include "roq/server/settings.hpp"

namespace roq {
namespace kucoin_futures {
namespace tools {

struct Throttle final : public web::rest::Interceptor, public web::socket::Interceptor {
  explicit Throttle(server::Settings const &);

  struct Params {
    int32_t remaining = {};
    int32_t limit = {};
    int64_t reset = {};  // msec / relative
  };

 protected:
  // web::Interceptor

  operator std::chrono::nanoseconds() const override { return suspend_until_; }

  // web::rest::Interceptor

  void operator()(Trace<web::rest::MessageBegin> const &) override;
  void operator()(Trace<web::rest::MessageHeader> const &) override;
  void operator()(Trace<web::rest::MessageEnd> const &) override;

  // web::socket::Interceptor

 private:
  bool const enabled_;

  Params params_;

  std::chrono::nanoseconds suspend_until_ = {};
};

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::kucoin_futures::tools::Throttle::Params> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::kucoin_futures::tools::Throttle::Params const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(remaining={}, )"
        R"(limit={}, )"
        R"(reset={})"
        R"(}})"sv,
        value.remaining,
        value.limit,
        value.reset);
  }
};
