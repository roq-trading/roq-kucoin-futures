/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/kucoin_futures/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  Message message_{message, buffer};
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
        case ORDER_MARGIN_CHANGE: {
          OrderMarginChange order_margin_change{message, buffer};
          create_trace_and_dispatch(handler, trace_info, order_margin_change);
          break;
        }
        case AVAILABLE_BALANCE_CHANGE: {
          AvailableBalanceChange available_balance_change{message, buffer};
          create_trace_and_dispatch(handler, trace_info, available_balance_change);
          break;
        }
        case WITHDRAW_HOLD_CHANGE: {
          WithdrawHoldChange withdraw_hold_change{message, buffer};
          create_trace_and_dispatch(handler, trace_info, withdraw_hold_change);
          break;
        }
        case POSITION_CHANGE: {
          PositionChange position_change{message, buffer};
          create_trace_and_dispatch(handler, trace_info, position_change);
          break;
        }
        case POSITION_SETTLEMENT: {
          PositionSettlement position_settlement{message, buffer};
          create_trace_and_dispatch(handler, trace_info, position_settlement);
          break;
        }
      }
      break;
    case WELCOME: {
      Welcome welcome{message, buffer};
      create_trace_and_dispatch(handler, trace_info, welcome);
      break;
    }
    case ERROR: {
      Error error{message, buffer};
      create_trace_and_dispatch(handler, trace_info, error);
      break;
    }
    case PONG: {
      Pong pong{message, buffer};
      create_trace_and_dispatch(handler, trace_info, pong);
      break;
    }
    case ACK: {
      Ack ack{message, buffer};
      create_trace_and_dispatch(handler, trace_info, ack);
      break;
    }
    case MESSAGE:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED__:
        case UNKNOWN__:
          log::fatal("Unexpected"sv);
          break;
        case TICKER: {
          Ticker ticker{message, buffer};
          create_trace_and_dispatch(handler, trace_info, ticker);
          break;
        }
        case TICKER_V2: {
          TickerV2 ticker_v2{message, buffer};
          create_trace_and_dispatch(handler, trace_info, ticker_v2);
          break;
        }
        case MATCH: {
          Match match{message, buffer};
          create_trace_and_dispatch(handler, trace_info, match);
          break;
        }
        case EXECUTION: {
          Execution execution{message, buffer};
          create_trace_and_dispatch(handler, trace_info, execution);
          break;
        }
        case MARK_INDEX_PRICE: {
          MarkIndexPrice mark_index_price{message, buffer};
          create_trace_and_dispatch(handler, trace_info, mark_index_price);
          break;
        }
        case FUNDING_RATE: {
          FundingRate funding_rate{message, buffer};
          create_trace_and_dispatch(handler, trace_info, funding_rate);
          break;
        }
        case LEVEL2: {
          Level2 level2{message, buffer};
          create_trace_and_dispatch(handler, trace_info, level2);
          break;
        }
        case FUNDING_BEGIN: {
          FundingBegin funding_begin{message, buffer};
          create_trace_and_dispatch(handler, trace_info, funding_begin);
          break;
        }
        case FUNDING_END: {
          FundingEnd funding_end{message, buffer};
          create_trace_and_dispatch(handler, trace_info, funding_end);
          break;
        }
        case SNAPSHOT_24H: {
          Snapshot24h snapshot_24h{message, buffer};
          create_trace_and_dispatch(handler, trace_info, snapshot_24h);
          break;
        }
        case ORDER_CHANGE: {
          OrderChange order_change{message, buffer};
          create_trace_and_dispatch(handler, trace_info, order_change);
          break;
        }
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
