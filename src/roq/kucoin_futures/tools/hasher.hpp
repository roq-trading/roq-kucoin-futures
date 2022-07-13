/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/core/crypto/hmac_sha256.hpp"

namespace roq {
namespace kucoin_futures {
namespace tools {

class Hasher final {
 public:
  Hasher(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase);

  Hasher(Hasher &&) = delete;
  Hasher(Hasher const &) = delete;

  std::string create_headers_v1(
      web::http::Method,
      std::string_view const &path,
      std::string_view const &query,
      std::string_view const &body,
      std::chrono::milliseconds now);

  std::string create_headers_v2(
      web::http::Method,
      std::string_view const &path,
      std::string_view const &query,
      std::string_view const &body,
      std::chrono::milliseconds now);

 private:
  const std::string key_;
  core::crypto::HMAC_SHA256 hmac_;
  const std::string passphrase_;
  const std::string signed_passphrase_;
};

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
