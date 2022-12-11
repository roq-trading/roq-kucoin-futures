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
  core::json::Parser parser(message);
  auto root = parser.root();
  json::Message message_(root, buffer);
  switch (message_.type) {
    using enum json::Type::type_t;
    case UNDEFINED:
    case UNKNOWN:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED:
        case UNKNOWN:
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
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::OrderMarginChange order_margin_change(root, buffer);
          create_trace_and_dispatch(handler, trace_info, order_margin_change);
          break;
        }
        case AVAILABLE_BALANCE_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::AvailableBalanceChange available_balance_change(root, buffer);
          create_trace_and_dispatch(handler, trace_info, available_balance_change);
          break;
        }
        case WITHDRAW_HOLD_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::WithdrawHoldChange withdraw_hold_change(root, buffer);
          create_trace_and_dispatch(handler, trace_info, withdraw_hold_change);
          break;
        }
        case POSITION_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::PositionChange position_change(root, buffer);
          create_trace_and_dispatch(handler, trace_info, position_change);
          break;
        }
        case POSITION_SETTLEMENT: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::PositionSettlement position_settlement(root, buffer);
          create_trace_and_dispatch(handler, trace_info, position_settlement);
          break;
        }
      }
      break;
    case WELCOME: {
      core::json::Parser parser(message);
      auto root = parser.root();
      const json::Welcome welcome(root, buffer);
      create_trace_and_dispatch(handler, trace_info, welcome);
      break;
    }
    case ERROR: {
      core::json::Parser parser(message);
      auto root = parser.root();
      const json::Error error(root, buffer);
      create_trace_and_dispatch(handler, trace_info, error);
      break;
    }
    case PONG: {
      core::json::Parser parser(message);
      auto root = parser.root();
      const json::Pong pong(root, buffer);
      create_trace_and_dispatch(handler, trace_info, pong);
      break;
    }
    case ACK: {
      core::json::Parser parser(message);
      auto root = parser.root();
      const json::Ack ack(root, buffer);
      create_trace_and_dispatch(handler, trace_info, ack);
      break;
    }
    case MESSAGE:
      switch (message_.subject) {
        using enum json::Subject::type_t;
        case UNDEFINED:
        case UNKNOWN:
          log::fatal("Unexpected"sv);
          break;
        case TICKER: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::Ticker ticker(root, buffer);
          create_trace_and_dispatch(handler, trace_info, ticker);
          break;
        }
        case TICKER_V2: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::TickerV2 ticker_v2(root, buffer);
          create_trace_and_dispatch(handler, trace_info, ticker_v2);
          break;
        }
        case MATCH: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::Match match(root, buffer);
          create_trace_and_dispatch(handler, trace_info, match);
          break;
        }
        case EXECUTION: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::Execution execution(root, buffer);
          create_trace_and_dispatch(handler, trace_info, execution);
          break;
        }
        case MARK_INDEX_PRICE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::MarkIndexPrice mark_index_price(root, buffer);
          create_trace_and_dispatch(handler, trace_info, mark_index_price);
          break;
        }
        case FUNDING_RATE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::FundingRate funding_rate(root, buffer);
          create_trace_and_dispatch(handler, trace_info, funding_rate);
          break;
        }
        case LEVEL2: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::Level2 level2(root, buffer);
          create_trace_and_dispatch(handler, trace_info, level2);
          break;
        }
        case FUNDING_BEGIN: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::FundingBegin funding_begin(root, buffer);
          create_trace_and_dispatch(handler, trace_info, funding_begin);
          break;
        }
        case FUNDING_END: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::FundingEnd funding_end(root, buffer);
          create_trace_and_dispatch(handler, trace_info, funding_end);
          break;
        }
        case SNAPSHOT_24H: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::Snapshot24h snapshot_24h(root, buffer);
          create_trace_and_dispatch(handler, trace_info, snapshot_24h);
          break;
        }
        case ORDER_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          const json::OrderChange order_change(root, buffer);
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
