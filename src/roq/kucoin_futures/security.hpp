/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/kucoin_futures/config.hpp"

#include "roq/kucoin_futures/tools/hasher.hpp"

namespace roq {
namespace kucoin_futures {

class Security final {
 public:
  Security(Config const &, std::string_view const &account);

  Security(Security &&) = delete;
  Security(Security const &) = delete;

  std::string_view get_account() const { return account_; }

  std::string create_signature_api_v1(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);
  std::string create_signature_api_v2(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

 private:
  const std::string account_;
  tools::Hasher hasher_;
};

}  // namespace kucoin_futures
}  // namespace roq
