/* Copyright (c) 2017-2024, Hans Erik Thrane */

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
void dispatch_helper(auto &handler, auto &message, auto &buffer, auto &trace_info) {
  auto obj = T::create(message, buffer);
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info) {
  auto message_ = Message::create(message, buffer);
  switch (message_.type) {
    using enum json::Type::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED__:
        case UNKNOWN__:
          log::fatal("Unexpected"sv);
          break;
        case TICKER:
        case TICKER_V2:
        case MATCH:
        case EXECUTION:
        case MARK_INDEX_PRICE:
        case FUNDING_RATE:
        case LEVEL2:
        case FUNDING_BEGIN:
        case FUNDING_END:
        case SNAPSHOT_24H:
        case ORDER_CHANGE:
          log::fatal("Unexpected"sv);
          break;
        case ORDER_MARGIN_CHANGE:
          dispatch_helper<OrderMarginChange>(handler, message, buffer, trace_info);
          break;
        case AVAILABLE_BALANCE_CHANGE:
          dispatch_helper<AvailableBalanceChange>(handler, message, buffer, trace_info);
          break;
        case WITHDRAW_HOLD_CHANGE:
          dispatch_helper<WithdrawHoldChange>(handler, message, buffer, trace_info);
          break;
        case POSITION_CHANGE:
          dispatch_helper<PositionChange>(handler, message, buffer, trace_info);
          break;
        case POSITION_SETTLEMENT:
          dispatch_helper<PositionSettlement>(handler, message, buffer, trace_info);
          break;
      }
      break;
    case WELCOME:
      dispatch_helper<Welcome>(handler, message, buffer, trace_info);
      break;
    case ERROR:
      dispatch_helper<Error>(handler, message, buffer, trace_info);
      break;
    case PONG:
      dispatch_helper<Pong>(handler, message, buffer, trace_info);
      break;
    case ACK:
      dispatch_helper<Ack>(handler, message, buffer, trace_info);
      break;
    case MESSAGE:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED__:
        case UNKNOWN__:
          log::fatal("Unexpected"sv);
          break;
        case TICKER:
          dispatch_helper<Ticker>(handler, message, buffer, trace_info);
          break;
        case TICKER_V2:
          dispatch_helper<TickerV2>(handler, message, buffer, trace_info);
          break;
        case MATCH:
          dispatch_helper<Match>(handler, message, buffer, trace_info);
          break;
        case EXECUTION:
          dispatch_helper<Execution>(handler, message, buffer, trace_info);
          break;
        case MARK_INDEX_PRICE:
          dispatch_helper<MarkIndexPrice>(handler, message, buffer, trace_info);
          break;
        case FUNDING_RATE:
          dispatch_helper<FundingRate>(handler, message, buffer, trace_info);
          break;
        case LEVEL2:
          dispatch_helper<Level2>(handler, message, buffer, trace_info);
          break;
        case FUNDING_BEGIN:
          dispatch_helper<FundingBegin>(handler, message, buffer, trace_info);
          break;
        case FUNDING_END:
          dispatch_helper<FundingEnd>(handler, message, buffer, trace_info);
          break;
        case SNAPSHOT_24H:
          dispatch_helper<Snapshot24h>(handler, message, buffer, trace_info);
          break;
        case ORDER_CHANGE:
          dispatch_helper<OrderChange>(handler, message, buffer, trace_info);
          break;
        case ORDER_MARGIN_CHANGE:
        case AVAILABLE_BALANCE_CHANGE:
        case WITHDRAW_HOLD_CHANGE:
        case POSITION_CHANGE:
        case POSITION_SETTLEMENT:
          log::fatal("Unexpected"sv);
          break;
      }
      break;
  }
  return true;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
