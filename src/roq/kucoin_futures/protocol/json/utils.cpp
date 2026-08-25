/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/protocol/json/utils.hpp"

#include "roq/kucoin_futures/protocol/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace protocol {
namespace json {

Error guess_error([[maybe_unused]] int32_t code) {
  return Error::UNKNOWN;
}

bool is_auth_error(int32_t code) {
  switch (code) {
    case 400001:  // Any of KC-API-KEY, KC-API-SIGN, KC-API-TIMESTAMP, KC-API-PASSPHRASE is missing in your request header.
    case 400002:  // KC-API-TIMESTAMP Invalid -- Time differs from server time by more than 5 seconds
    case 400003:  // KC-API-KEY does not exist
    case 400004:  // KC-API-PASSPHRASE error
    case 400005:  // Signature error -- Please check your signature
    case 400006:  // The IP address is not on the API whitelist
    case 400007:  // Access Denied -- Your API key does not have sufficient permissions to access the URI
      return true;
  }
  return false;
}

}  // namespace json
}  // namespace protocol
}  // namespace kucoin_futures
}  // namespace roq
