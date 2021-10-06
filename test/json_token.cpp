/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include <cmath>

#include "roq/core/datetime.h"

#include "roq/core/json/parser.h"

#include "roq/kucoin_futures/json/token.h"

using namespace roq;
using namespace roq::kucoin_futures;

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
}
