/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/utils/mac/hmac.hpp"

namespace roq {
namespace kucoin_futures {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret, std::string_view const &passphrase);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_headers(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::milliseconds now);

  std::string create_ws_query(std::chrono::milliseconds now);
  std::string create_ws_auth(std::string_view const &message);

 private:
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;
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
