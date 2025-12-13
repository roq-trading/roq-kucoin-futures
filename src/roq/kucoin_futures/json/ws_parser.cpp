/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/json/ws_parser.hpp"

#include "roq/logging.hpp"

#include "roq/kucoin_futures/json/ws_message.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

// === HELPERS ===

namespace {
int32_t const CODE_SUCCESS = 200000;
}

// === HELPERS ===

namespace {
template <typename T, typename... Args>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info, Args &&...args) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj, std::forward<Args>(args)...);
}
}  // namespace

// === IMPLEMENTATION ===

bool WSParser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  WSMessage message_2{message, buffer_stack};
  if (std::empty(message_2.session_id)) {
    switch (message_2.op) {
      using enum json::WSOp::type_t;
      case UNDEFINED_INTERNAL:
        if (allow_unknown_event_types) {
          return false;
        }
        break;
      case UNKNOWN_INTERNAL:
        break;
      case PONG:
        dispatch_helper<WSPong>(handler, message, buffer_stack, trace_info);
        return true;
      case ADD_ORDER_ACK:
        if (message_2.code != CODE_SUCCESS) {
          dispatch_helper<WSError>(handler, message, buffer_stack, trace_info);
          return true;
        }
        dispatch_helper<WSAddOrderAck>(handler, message, buffer_stack, trace_info);
        return true;
      case CANCEL_ORDER_ACK:
        if (message_2.code != CODE_SUCCESS) {
          dispatch_helper<WSError>(handler, message, buffer_stack, trace_info);
          return true;
        }
        dispatch_helper<WSCancelOrderAck>(handler, message, buffer_stack, trace_info);
        return true;
    }
  } else {
    if (message_2.ping_interval.count()) {
      dispatch_helper<WSWelcome>(handler, message, buffer_stack, trace_info);
      return true;
    } else {
      dispatch_helper<WSAuth>(handler, message, buffer_stack, trace_info, message);
      return true;
    }
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
