/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/hasher.h"

#include <fmt/format.h>

#include <array>

#include "roq/literals.h"

#include "roq/core/binascii/base64.h"

#include "roq/core/crypto/sha.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {
namespace tools {

namespace {
static auto create_hmac(const std::string_view &secret) {
  auto raw_secret = core::binascii::Base64::decode(secret, true);
  return core::crypto::HMAC_SHA512(raw_secret);
}
}  // namespace

Hasher::Hasher(const std::string_view &secret) : hmac_(create_hmac(secret)) {
}

std::string Hasher::create_headers(
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &query,
    const std::string_view &body,
    const std::string_view &key,
    const std::string_view &passphrase,
    std::chrono::milliseconds timestamp) {
  assert(!path.empty());
  auto tmp = fmt::format("{}{}{}{}{}"_sv, timestamp.count(), method, path, query, body);
  hmac_.clear();
  hmac_.update(tmp);
  std::array<char, 64> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == std::size(buffer));
  auto signature = core::binascii::Base64::encode(buffer, false);
  auto result = fmt::format(
      "KC-API-KEY: {}\r\n"
      "KC-API-SIGN: {}\r\n"
      "KC-API-TIMESTAMP: {}\r\n"
      "KC-API-PASSPHRASE: {}\r\n"
      "KC-API-VERSION: 1\r\n"_sv,
      key,
      signature,
      timestamp.count(),
      passphrase);
  return result;
}

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
