/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/security.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/core/clock.hpp"

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

Security::Security(Config const &config, std::string_view const &account)
    : account_(account),
      hasher_(config.get_api_key(account_), config.get_secret(account_), config.get_passphrase(account_)) {
}

std::string Security::create_signature_api_v1(
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body) {
  auto now = core::clock::GetRealTime();
  return hasher_.create_headers_v1(method, path, query, body, utils::safe_cast(now));
}

std::string Security::create_signature_api_v2(
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body) {
  auto now = core::clock::GetRealTime();
  return hasher_.create_headers_v2(method, path, query, body, utils::safe_cast(now));
}

}  // namespace kucoin_futures
}  // namespace roq
