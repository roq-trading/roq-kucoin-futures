/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/http/method.hpp"

#include "roq/kucoin_futures/config.hpp"

#include "roq/kucoin_futures/tools/hasher.hpp"

namespace roq {
namespace kucoin_futures {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }

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
  tools::Hasher hasher_;
};

}  // namespace kucoin_futures
}  // namespace roq
