/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/http/method.h"

#include "roq/core/crypto/hmac.h"

namespace roq {
namespace kucoin_futures {
namespace tools {

class Hasher final {
 public:
  explicit Hasher(const std::string_view &secret);

  Hasher(Hasher &&) = delete;
  Hasher(const Hasher &) = delete;

  std::string create_headers(
      core::http::Method,
      const std::string_view &path,
      const std::string_view &query,
      const std::string_view &body,
      const std::string_view &key,
      const std::string_view &passphrase,
      std::chrono::milliseconds now);

 private:
  core::crypto::HMAC_SHA512 hmac_;
};

}  // namespace tools
}  // namespace kucoin_futures
}  // namespace roq
