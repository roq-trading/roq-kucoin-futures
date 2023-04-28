/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/kucoin_futures/config.hpp"

#include "roq/kucoin_futures/tools/crypto.hpp"

namespace roq {
namespace kucoin_futures {

struct Account final {
  Account(Config const &, std::string_view const &name);

  Account(Account &&) = delete;
  Account(Account const &) = delete;

  std::string_view get_name() const { return name_; }

  std::string create_signature_api_v1(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);
  std::string create_signature_api_v2(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

 private:
  std::string const name_;
  tools::Crypto crypto_;
};

}  // namespace kucoin_futures
}  // namespace roq
