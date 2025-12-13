/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/tools/crypto.hpp"

#include <fmt/format.h>

#include <array>

#include "roq/logging.hpp"

#include "roq/utils/codec/base64.hpp"
#include "roq/utils/codec/url.hpp"

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
  utils::codec::Base64::encode(result, digest, false, false);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase)
    : key_{key}, mac_{secret}, passphrase_{passphrase}, signed_passphrase_{create_signed_passphrase(mac_, digest_, passphrase)} {
}

std::string Crypto::create_headers(
    web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::milliseconds timestamp) {
  assert(!std::empty(path));
  auto tmp = fmt::format("{}{}{}{}{}"sv, timestamp.count(), method, path, query, body);
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature;
  utils::codec::Base64::encode(signature, digest, false, false);
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

std::string Crypto::create_ws_query(std::chrono::milliseconds now) {
  auto timestamp_2 = fmt::format("{}"sv, now.count());
  // sign
  mac_.clear();
  mac_.update(key_);
  mac_.update(timestamp_2);
  auto digest = mac_.final(digest_);
  std::string sign;
  utils::codec::Base64::encode(sign, digest, false, false);
  std::string buffer_sign;
  auto sign_url = utils::codec::URL::encode(buffer_sign, sign);
  // passphrase
  std::string buffer_passphrase;
  auto passphrase_url = utils::codec::URL::encode(buffer_passphrase, signed_passphrase_);
  // query
  auto result = fmt::format("?apikey={}&sign={}&passphrase={}&timestamp={}"sv, key_, sign_url, passphrase_url, timestamp_2);
  return result;
}

std::string Crypto::create_ws_auth(std::string_view const &message) {
  mac_.clear();
  mac_.update(message);
  auto digest = mac_.final(digest_);
  std::string result;
  utils::codec::Base64::encode(result, digest, false, false);
  return result;
}

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
