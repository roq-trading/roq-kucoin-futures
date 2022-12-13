/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/hasher.hpp"

#include <fmt/format.h>

#include <array>

#include "roq/core/binascii/base64.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace tools {

// === HELPERS ===

namespace {
auto create_signed_passphrase(auto &mac, auto &digest_buffer, auto const &passphrase) {
  mac.clear();
  mac.update(passphrase);
  auto digest = mac.final(digest_buffer);
  std::string result;
  core::binascii::Base64::encode(result, digest, false);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Hasher::Hasher(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase)
    : key_{key}, mac_{secret}, passphrase_{passphrase}, signed_passphrase_{
                                                            create_signed_passphrase(mac_, digest_, passphrase)} {
}

std::string Hasher::create_headers_v1(
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body,
    std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  core::binascii::Base64::encode(signature, digest, false);
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
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &body,
    std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  core::binascii::Base64::encode(signature, digest, false);
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
