/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kucoin_futures/json/ack.h"
#include "roq/kucoin_futures/json/error.h"
#include "roq/kucoin_futures/json/pong.h"
#include "roq/kucoin_futures/json/welcome.h"

#include "roq/kucoin_futures/json/level2.h"
#include "roq/kucoin_futures/json/snapshot.h"
#include "roq/kucoin_futures/json/ticker.h"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(server::Trace<json::Welcome> const &) = 0;
    virtual void operator()(server::Trace<json::Error> const &) = 0;
    virtual void operator()(server::Trace<json::Pong> const &) = 0;
    virtual void operator()(server::Trace<json::Ack> const &) = 0;

    virtual void operator()(server::Trace<json::Snapshot> const &) = 0;
    virtual void operator()(server::Trace<json::Ticker> const &) = 0;
    virtual void operator()(server::Trace<json::Level2> const &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      std::string_view const &message,
      core::json::Buffer &,
      server::TraceInfo const &);
};

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
