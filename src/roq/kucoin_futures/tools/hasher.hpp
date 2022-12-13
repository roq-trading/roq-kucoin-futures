/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/core/mac/hmac.hpp"

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
  using MAC = core::mac::HMAC<core::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  MAC mac_;
  Digest digest_;
  std::string const passphrase_;
  std::string const signed_passphrase_;
};

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
