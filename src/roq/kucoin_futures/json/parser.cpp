/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/kucoin_futures/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  Message message_2{message, buffer_stack};
  switch (message_2.type) {
    using enum json::Type::type_t;
    case UNDEFINED_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
    case UNKNOWN_INTERNAL:
      break;
    case WELCOME:
      dispatch_helper<Welcome>(handler, message, buffer_stack, trace_info);
      return true;
    case ERROR:
      dispatch_helper<Error>(handler, message, buffer_stack, trace_info);
      return true;
    case PONG:
      dispatch_helper<Pong>(handler, message, buffer_stack, trace_info);
      return true;
    case ACK:
      dispatch_helper<Ack>(handler, message, buffer_stack, trace_info);
      return true;
    case MESSAGE:
      switch (message_2.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED_INTERNAL:
          break;
        case UNKNOWN_INTERNAL:
          if (allow_unknown_event_types) {
            return false;
          }
          break;
        case TICKER_V2:
          dispatch_helper<TickerV2>(handler, message, buffer_stack, trace_info);
          return true;
        case MATCH:
          dispatch_helper<Match>(handler, message, buffer_stack, trace_info);
          return true;
        case EXECUTION:
          dispatch_helper<Execution>(handler, message, buffer_stack, trace_info);
          return true;
        case MARK_INDEX_PRICE:
          dispatch_helper<MarkIndexPrice>(handler, message, buffer_stack, trace_info);
          return true;
        case FUNDING_RATE:
          dispatch_helper<FundingRate>(handler, message, buffer_stack, trace_info);
          return true;
        case LEVEL2:
          dispatch_helper<Level2>(handler, message, buffer_stack, trace_info);
          return true;
        case FUNDING_BEGIN:
          dispatch_helper<FundingBegin>(handler, message, buffer_stack, trace_info);
          return true;
        case FUNDING_END:
          dispatch_helper<FundingEnd>(handler, message, buffer_stack, trace_info);
          return true;
        case SNAPSHOT_24H:
          dispatch_helper<Snapshot24h>(handler, message, buffer_stack, trace_info);
          return true;
        case WALLET_BALANCE_CHANGE:
          dispatch_helper<WalletBalanceChange>(handler, message, buffer_stack, trace_info);
          return true;
        case ORDER_MARGIN_CHANGE:
          dispatch_helper<OrderMarginChange>(handler, message, buffer_stack, trace_info);
          return true;
        case AVAILABLE_BALANCE_CHANGE:
          dispatch_helper<AvailableBalanceChange>(handler, message, buffer_stack, trace_info);
          return true;
        case WITHDRAW_HOLD_CHANGE:
          dispatch_helper<WithdrawHoldChange>(handler, message, buffer_stack, trace_info);
          return true;
        case POSITION_CHANGE:
          dispatch_helper<PositionChange>(handler, message, buffer_stack, trace_info);
          return true;
        case POSITION_SETTLEMENT:
          dispatch_helper<PositionSettlement>(handler, message, buffer_stack, trace_info);
          return true;
        case POSITION_ADJUST_RISK_LIMIT:
          dispatch_helper<PositionAdjustRiskLimit>(handler, message, buffer_stack, trace_info);
          return true;
        case SYMBOL_ORDER_CHANGE:
          dispatch_helper<SymbolOrderChange>(handler, message, buffer_stack, trace_info);
          return true;
        case ORDER_CHANGE:
          dispatch_helper<OrderChange>(handler, message, buffer_stack, trace_info);
          return true;
      }
      break;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
