/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/rate_limit.hpp"

#include "roq/utils/compare.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/utils/charconv/from_chars.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace tools {

// === CONSTANTS ===

namespace {
auto const DEFAULT_BACKOFF = 5s;     // note! maybe as low as 1 second
auto const BLOCKED_BACKOFF = 10min;  // note! very serious
}  // namespace

// === HELPERS ===

namespace {
// note! std::tolower is not constexpr gcc16 + clang23
constexpr auto lower(auto value) {
  return utils::detail::ascii_to_lower(value);
}

enum class Header {
  UNKNOWN,
  GW_RATELIMIT_REMAINING,
  GW_RATELIMIT_LIMIT,
  GW_RATELIMIT_RESET,
};

constexpr auto parse_header(std::string_view const &text) {
  std::string value;
  value.reserve(std::size(text));
  std::transform(std::begin(text), std::end(text), std::back_inserter(value), [](auto c) { return lower(c); });
  auto key = utils::hash::FNV::compute(value);
  switch (key) {
    case utils::hash::FNV::compute("gw-ratelimit-remaining"sv):
      return Header::GW_RATELIMIT_REMAINING;
    case utils::hash::FNV::compute("gw-ratelimit-limit"sv):
      return Header::GW_RATELIMIT_LIMIT;
    case utils::hash::FNV::compute("gw-ratelimit-reset"sv):
      return Header::GW_RATELIMIT_RESET;
  }
  return Header::UNKNOWN;
}

static_assert(parse_header("gw-ratelimit-remaining"sv) == Header::GW_RATELIMIT_REMAINING);
static_assert(parse_header("gw-ratelimit-limit"sv) == Header::GW_RATELIMIT_LIMIT);
static_assert(parse_header("gw-ratelimit-reset"sv) == Header::GW_RATELIMIT_RESET);
}  // namespace

// === IMPLEMENTATION ===

RateLimit::RateLimit(flags::Settings const &settings) : suspend_on_rate_limit_{settings.experimental.suspend_on_rate_limit} {
}

// web::rest::Interceptor

void RateLimit::operator()(Trace<web::rest::MessageBegin> const &) {
}

void RateLimit::operator()(Trace<web::rest::MessageHeader> const &event) {
  auto &[trace_info, header] = event;
  auto update_value = [&](auto &result) {
    using value_type = std::remove_cvref_t<decltype(result)>;
    auto value = utils::charconv::from_chars<value_type>(header.value);
    return utils::update(result, value);
  };
  auto update_suspend_until = [&]() {
    if (!suspend_on_rate_limit_) {
      return;
    }
    /*
    auto now = clock::get_system();
    auto now_utc = clock::get_realtime();
    auto period = std::chrono::milliseconds{params_.reset};
    suspend_until_ = std::max(suspend_until_, now + period);
    */
  };
  auto key = parse_header(header.name);
  switch (key) {
    using enum Header;
    [[likely]] case UNKNOWN:
      return;
    case GW_RATELIMIT_REMAINING:
      update_value(params_.remaining);
      break;
    case GW_RATELIMIT_LIMIT:
      if (update_value(params_.limit)) {
        update_suspend_until();
      }
      break;
    case GW_RATELIMIT_RESET:
      if (update_value(params_.reset)) {
        update_suspend_until();
      }
      break;
  }
}

void RateLimit::operator()(Trace<web::rest::MessageEnd> const &event) {
  auto &[trace_info, message_end] = event;
  if (!suspend_on_rate_limit_) {
    return;
  }
  switch (message_end.status) {
    using enum web::http::Status;
    [[unlikely]] case FORBIDDEN: {  // 403
      auto now = clock::get_system();
      suspend_until_ = std::max(suspend_until_, now + BLOCKED_BACKOFF);
      break;
    }
    [[unlikely]] case TOO_MANY_REQUESTS: {  // 429
      if (suspend_until_.count() == 0) {
        auto now = clock::get_system();
        suspend_until_ = now + DEFAULT_BACKOFF;
      }
      break;
    }
    default:
      break;
  }
}

// web::socket::Interceptor

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
