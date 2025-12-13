/* Copyright (c) 2017-2026, Hans Erik Thrane */

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

  Account(Account const &) = delete;

  std::string create_headers(web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

  std::string create_ws_query();
  std::string create_ws_auth(std::string_view const &message);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace kucoin_futures
}  // namespace roq
