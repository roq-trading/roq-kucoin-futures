/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/hasher.hpp"

#include <fmt/format.h>

#include <array>

#include "roq/core/binascii/base64.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace tools {

namespace {
auto create_hmac(std::string_view const &secret) {
  return core::crypto::HMAC_SHA256(secret);
}
auto create_signed_passphrase(core::crypto::HMAC_SHA256 &hmac, std::string_view const &passphrase) {
  hmac.clear();
  hmac.update(passphrase);
  std::array<char, 32> buffer;
  auto length = hmac.digest(buffer);
  assert(length == std::size(buffer));
  return core::binascii::Base64::encode(buffer, false);
}
}  // namespace

Hasher::Hasher(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase)
    : key_(key), hmac_(create_hmac(secret)), passphrase_(passphrase),
      signed_passphrase_(create_signed_passphrase(hmac_, passphrase)) {
}

std::string Hasher::create_headers_v1(
    core::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body,
    std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  hmac_.clear();
  hmac_.update(tmp);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == std::size(buffer));
  auto signature = core::binascii::Base64::encode(buffer, false);
  auto result = fmt::format(
      "KC-API-KEY: {}\r\n"
      "KC-API-SIGN: {}\r\n"
      "KC-API-TIMESTAMP: {}\r\n"
      "KC-API-PASSPHRASE: {}\r\n"
      "KC-API-KEY-VERSION: 1\r\n"sv,
      key_,
      signature,
      timestamp.count(),
      passphrase_);
  return result;
}

std::string Hasher::create_headers_v2(
    core::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body,
    std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  hmac_.clear();
  hmac_.update(tmp);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == std::size(buffer));
  auto signature = core::binascii::Base64::encode(buffer, false);
  auto result = fmt::format(
      "KC-API-KEY: {}\r\n"
      "KC-API-SIGN: {}\r\n"
      "KC-API-TIMESTAMP: {}\r\n"
      "KC-API-PASSPHRASE: {}\r\n"
      "KC-API-KEY-VERSION: 2\r\n"sv,
      key_,
      signature,
      timestamp.count(),
      signed_passphrase_);
  return result;
}

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
