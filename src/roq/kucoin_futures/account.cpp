/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/account.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/clock.hpp"

namespace roq {
namespace kucoin_futures {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name)
    : name{name}, crypto_{config.get_api_key(name), config.get_secret(name), config.get_passphrase(name)} {
}

std::string Account::create_headers(web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body) {
  auto now = clock::get_realtime();
  return crypto_.create_headers(method, path, query, body, utils::safe_cast(now));
}

std::string Account::create_ws_query() {
  auto now = clock::get_realtime();
  return crypto_.create_ws_query(utils::safe_cast(now));
}

std::string Account::create_ws_auth(std::string_view const &message) {
  return crypto_.create_ws_auth(message);
}

}  // namespace kucoin_futures
}  // namespace roq
