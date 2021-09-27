/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <fmt/format.h>

#include <chrono>
#include <string>
#include <string_view>
#include <utility>

#include "roq/core/http/method.h"

#include "roq/kucoin_futures/config.h"

#include "roq/kucoin_futures/tools/hasher.h"

namespace roq {
namespace kucoin_futures {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }

  std::string_view get_api_key() const { return key_; }  // XXX HANS remove

  // std::pair<std::string, std::string> create_signature(const std::chrono::nanoseconds &now);

  std::string create_signature_api_v1(
      core::http::Method,
      const std::string_view &path,
      const std::string_view &query,
      const std::string_view &body);
  std::string create_signature_api_v2(
      core::http::Method,
      const std::string_view &path,
      const std::string_view &query,
      const std::string_view &body);

 private:
  const std::string account_;
  const std::string key_;
  const std::string passphrase_;
  tools::Hasher hasher_;
};

}  // namespace kucoin_futures
}  // namespace roq
