/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <cmath>

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/wallet_balance_change.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_wallet_balance_change", "[json_wallet_balance_change]") {
  auto const message = R"({)"
                       R"("userId": "xbc453tg732eba53a88ggyt8c",)"
                       R"("topic": "/contractAccount/wallet",)"
                       R"("subject": "orderMargin.change",)"
                       R"("data": {)"
                       R"(})"
                       R"(})";
  std::vector<std::byte> buffer(8192);
  json::WalletBalanceChange obj{message, buffer};
}
