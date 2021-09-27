/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/security.h"

#include <cassert>

#include "roq/utils/safe_cast.h"

#include "roq/core/clock.h"

#include "roq/core/binascii/hex.h"

#include "roq/core/crypto/hmac.h"

namespace roq {
namespace kucoin_futures {

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), key_(config.get_api_key(account_)),
      passphrase_(config.get_passphrase(account_)), hasher_(config.get_secret(account_)) {
}

std::string Security::create_signature_api_v1(
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &query,
    const std::string_view &body) {
  auto now = core::get_realtime_clock();
  return hasher_.create_headers(
      method, path, query, body, key_, passphrase_, utils::safe_cast(now));
}

std::string Security::create_signature_api_v2(
    [[maybe_unused]] core::http::Method method,
    [[maybe_unused]] const std::string_view &path,
    [[maybe_unused]] const std::string_view &query,
    [[maybe_unused]] const std::string_view &body) {
  auto now = core::get_realtime_clock();
  auto timestamp =
      fmt::format("{}"_sv, std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
  std::string sign;
  auto result = fmt::format(
      R"(KC-API-KEY: {}\r\n")"
      R"(KC-API-SIGN: {}\r\n")"
      R"(KC-API-TIMESTAMP: {}\r\n")"
      R"(KC-API-PASSPHRASE: {}\r\n")"
      R"(KC-API-VERSION: 2\r\n")"_sv,
      key_,
      sign,
      timestamp,
      passphrase_);  // must be encoded
  return result;
}

}  // namespace kucoin_futures
}  // namespace roq
