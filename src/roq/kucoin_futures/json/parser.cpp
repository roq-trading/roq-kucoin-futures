/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

bool Parser::dispatch(Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info) {
  Message message_{message, buffer_stack};
  switch (message_.type) {
    using enum json::Type::type_t;
    case UNDEFINED_INTERNAL:
    case UNKNOWN_INTERNAL:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          log::fatal("Unexpected"sv);
          break;
        case TICKER_V2:
        case MATCH:
        case EXECUTION:
        case MARK_INDEX_PRICE:
        case FUNDING_RATE:
        case LEVEL2:
        case FUNDING_BEGIN:
        case FUNDING_END:
        case SNAPSHOT_24H:
          log::fatal("Unexpected"sv);
          break;
        case WALLET_BALANCE_CHANGE:
          dispatch_helper<WalletBalanceChange>(handler, message, buffer_stack, trace_info);
          break;
        case ORDER_MARGIN_CHANGE:
          dispatch_helper<OrderMarginChange>(handler, message, buffer_stack, trace_info);
          break;
        case AVAILABLE_BALANCE_CHANGE:
          dispatch_helper<AvailableBalanceChange>(handler, message, buffer_stack, trace_info);
          break;
        case WITHDRAW_HOLD_CHANGE:
          dispatch_helper<WithdrawHoldChange>(handler, message, buffer_stack, trace_info);
          break;
        case POSITION_CHANGE:
          dispatch_helper<PositionChange>(handler, message, buffer_stack, trace_info);
          break;
        case POSITION_SETTLEMENT:
          dispatch_helper<PositionSettlement>(handler, message, buffer_stack, trace_info);
          break;
        case POSITION_ADJUST_RISK_LIMIT:
          dispatch_helper<PositionAdjustRiskLimit>(handler, message, buffer_stack, trace_info);
          break;
        case SYMBOL_ORDER_CHANGE:
        case ORDER_CHANGE:
          log::fatal("Unexpected"sv);
          break;
      }
      break;
    case WELCOME:
      dispatch_helper<Welcome>(handler, message, buffer_stack, trace_info);
      break;
    case ERROR:
      dispatch_helper<Error>(handler, message, buffer_stack, trace_info);
      break;
    case PONG:
      dispatch_helper<Pong>(handler, message, buffer_stack, trace_info);
      break;
    case ACK:
      dispatch_helper<Ack>(handler, message, buffer_stack, trace_info);
      break;
    case MESSAGE:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          log::fatal("Unexpected"sv);
          break;
        case TICKER_V2:
          dispatch_helper<TickerV2>(handler, message, buffer_stack, trace_info);
          break;
        case MATCH:
          dispatch_helper<Match>(handler, message, buffer_stack, trace_info);
          break;
        case EXECUTION:
          dispatch_helper<Execution>(handler, message, buffer_stack, trace_info);
          break;
        case MARK_INDEX_PRICE:
          dispatch_helper<MarkIndexPrice>(handler, message, buffer_stack, trace_info);
          break;
        case FUNDING_RATE:
          dispatch_helper<FundingRate>(handler, message, buffer_stack, trace_info);
          break;
        case LEVEL2:
          dispatch_helper<Level2>(handler, message, buffer_stack, trace_info);
          break;
        case FUNDING_BEGIN:
          dispatch_helper<FundingBegin>(handler, message, buffer_stack, trace_info);
          break;
        case FUNDING_END:
          dispatch_helper<FundingEnd>(handler, message, buffer_stack, trace_info);
          break;
        case SNAPSHOT_24H:
          dispatch_helper<Snapshot24h>(handler, message, buffer_stack, trace_info);
          break;
        case WALLET_BALANCE_CHANGE:
        case ORDER_MARGIN_CHANGE:
        case AVAILABLE_BALANCE_CHANGE:
        case WITHDRAW_HOLD_CHANGE:
        case POSITION_CHANGE:
        case POSITION_SETTLEMENT:
        case POSITION_ADJUST_RISK_LIMIT:
          log::fatal("Unexpected"sv);
          break;
        case SYMBOL_ORDER_CHANGE:
          dispatch_helper<SymbolOrderChange>(handler, message, buffer_stack, trace_info);
          break;
        case ORDER_CHANGE:
          dispatch_helper<OrderChange>(handler, message, buffer_stack, trace_info);
          break;
      }
      break;
  }
  return true;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
