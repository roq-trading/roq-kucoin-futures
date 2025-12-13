/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/token.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Token;

TEST_CASE("simple", "[json_token]") {
  auto message =
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
  auto helper = [&](value_type &obj) {
    CHECK(obj.code == 200000);
    auto &data = obj.data;
    CHECK(
        data.token ==
        "2neAiuYvAU61ZDXANAGAsiL4-iAExhsBXZxftpOeh_55i3Ysy2q2LEsEWU64mdzUOPusi34M_"
        "wGoSf7iNyEWJ85XuAlIeRt8svl5W6NWaJ6QT7eIx7nvxtiYB9J6i9GjsxUuhPw3BlrzazF6gh"
        "q4L_Hs1E-1pGVsVNxvpblIt0c=.sb29ayXiBOmPXNoHkkMaUA=="sv);
    auto &instance_servers = data.instance_servers;
    CHECK(std::size(instance_servers) == 1);
    auto &is0 = instance_servers[0];
    CHECK(is0.endpoint == "wss://push-websocket-sandbox.kucoin.com/endpoint"sv);
    CHECK(is0.encrypt == true);
    CHECK(is0.protocol == "websocket"sv);
    CHECK(is0.ping_interval == 50000ms);
    CHECK(is0.ping_timeout == 10000ms);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
