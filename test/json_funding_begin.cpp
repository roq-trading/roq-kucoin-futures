/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::kucoin_futures;

using namespace std::literals;

using value_type = json::FundingBegin;

TEST_CASE("simple", "[json_funding_begin]") {
  /*
  auto message = R"({)"
                 // XXX FIXME don't have
                 R"(})";
  auto helper = [](value_type const &obj) {
    // XXX
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
  */
}
