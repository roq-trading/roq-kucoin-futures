/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/protocol/json/ws_add_order_ack.hpp"
#include "roq/kucoin_futures/protocol/json/ws_auth.hpp"
#include "roq/kucoin_futures/protocol/json/ws_cancel_order_ack.hpp"
#include "roq/kucoin_futures/protocol/json/ws_error.hpp"
#include "roq/kucoin_futures/protocol/json/ws_pong.hpp"
#include "roq/kucoin_futures/protocol/json/ws_welcome.hpp"

namespace roq {
namespace kucoin_futures {
namespace protocol {
namespace json {

struct WSParser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::WSAuth> const &, std::string_view const &message) = 0;
    virtual void operator()(Trace<protocol::json::WSWelcome> const &) = 0;
    virtual void operator()(Trace<protocol::json::WSError> const &) = 0;
    virtual void operator()(Trace<protocol::json::WSPong> const &) = 0;
    virtual void operator()(Trace<protocol::json::WSAddOrderAck> const &) = 0;
    virtual void operator()(Trace<protocol::json::WSCancelOrderAck> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace protocol
}  // namespace kucoin_futures
}  // namespace roq
