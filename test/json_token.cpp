/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/token.h"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_token, simple) {
  const auto message =
      R"({)"
      R"("code":"200000",)"
      R"("data":{)"
      R"("token":"2neAiuYvAU61ZDXANAGAsiL4-iAExhsBXZxftpOeh_55i3Ysy2q2LEsEWU64mdzUOPusi34M_wGoSf7iNyEWJ85XuAlIeRt8svl5W6NWaJ6QT7eIx7nvxtiYB9J6i9GjsxUuhPw3BlrzazF6ghq4L_Hs1E-1pGVsVNxvpblIt0c=.sb29ayXiBOmPXNoHkkMaUA==",)"
      R"("instanceServers":[{)"
      R"("endpoint":"wss://push-websocket-sandbox.kucoin.com/endpoint",)"
      R"("encrypt":true,)"
      R"("protocol":"websocket",)"
      R"("pingInterval":50000,)"
      R"("pingTimeout":10000)"
      R"(})"
      R"(])"
      R"(})"
      R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Token>(message, buffer_);
  EXPECT_EQ(obj.code, 200000);
  auto &data = obj.data;
  EXPECT_EQ(
      data.token,
      "2neAiuYvAU61ZDXANAGAsiL4-iAExhsBXZxftpOeh_55i3Ysy2q2LEsEWU64mdzUOPusi34M_"
      "wGoSf7iNyEWJ85XuAlIeRt8svl5W6NWaJ6QT7eIx7nvxtiYB9J6i9GjsxUuhPw3BlrzazF6gh"
      "q4L_Hs1E-1pGVsVNxvpblIt0c=.sb29ayXiBOmPXNoHkkMaUA=="sv);
  auto &instance_servers = data.instance_servers;
  EXPECT_EQ(std::size(instance_servers), 1);
  auto &is0 = instance_servers[0];
  EXPECT_EQ(is0.endpoint, "wss://push-websocket-sandbox.kucoin.com/endpoint"sv);
  EXPECT_EQ(is0.encrypt, true);
  EXPECT_EQ(is0.protocol, "websocket"sv);
  EXPECT_EQ(is0.ping_interval, 50000ms);
  EXPECT_EQ(is0.ping_timeout, 10000ms);
}
