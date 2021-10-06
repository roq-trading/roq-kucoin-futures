/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/json/parser.h"

#include "roq/logging.h"

#include "roq/kucoin_futures/json/message.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {
namespace json {

bool Parser::dispatch(
    Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    server::TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  json::Message message_(root, buffer);
  switch (message_.type) {
    case json::Type::UNDEFINED:
    case json::Type::UNKNOWN:
      switch (message_.subject) {
        case json::Subject::UNDEFINED:
        case json::Subject::UNKNOWN:
          log::fatal("Unexpected"_sv);
          break;
        case json::Subject::TICKER:
        case json::Subject::TICKER_V2:
        case json::Subject::MATCH:
        case json::Subject::MARK_INDEX_PRICE:
        case json::Subject::FUNDING_RATE:
        case json::Subject::LEVEL2:
        case json::Subject::FUNDING_BEGIN:
        case json::Subject::FUNDING_END:
        case json::Subject::SNAPSHOT_24H:
        case json::Subject::ORDER_CHANGE:
          log::fatal("Unexpected"_sv);
          break;
        case json::Subject::ORDER_MARGIN_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::OrderMarginChange order_margin_change(root, buffer);
          create_trace_and_dispatch(trace_info, order_margin_change, handler);
          break;
        }
        case json::Subject::AVAILABLE_BALANCE_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::AvailableBalanceChange available_balance_change(root, buffer);
          create_trace_and_dispatch(trace_info, available_balance_change, handler);
          break;
        }
        case json::Subject::WITHDRAW_HOLD_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::WithdrawHoldChange withdraw_hold_change(root, buffer);
          create_trace_and_dispatch(trace_info, withdraw_hold_change, handler);
          break;
        }
        case json::Subject::POSITION_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::PositionChange position_change(root, buffer);
          create_trace_and_dispatch(trace_info, position_change, handler);
          break;
        }
        case json::Subject::POSITION_SETTLEMENT: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::PositionSettlement position_settlement(root, buffer);
          create_trace_and_dispatch(trace_info, position_settlement, handler);
          break;
        }
      }
      break;
    case json::Type::WELCOME: {
      core::json::Parser parser(message);
      auto root = parser.root();
      json::Welcome welcome(root, buffer);
      create_trace_and_dispatch(trace_info, welcome, handler);
      break;
    }
    case json::Type::ERROR: {
      core::json::Parser parser(message);
      auto root = parser.root();
      json::Error error(root, buffer);
      create_trace_and_dispatch(trace_info, error, handler);
      break;
    }
    case json::Type::PONG: {
      core::json::Parser parser(message);
      auto root = parser.root();
      json::Pong pong(root, buffer);
      create_trace_and_dispatch(trace_info, pong, handler);
      break;
    }
    case json::Type::ACK: {
      core::json::Parser parser(message);
      auto root = parser.root();
      json::Ack ack(root, buffer);
      create_trace_and_dispatch(trace_info, ack, handler);
      break;
    }
    case json::Type::MESSAGE:
      switch (message_.subject) {
        case json::Subject::UNDEFINED:
        case json::Subject::UNKNOWN:
          log::fatal("Unexpected"_sv);
          break;
        case json::Subject::TICKER: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Ticker ticker(root, buffer);
          create_trace_and_dispatch(trace_info, ticker, handler);
          break;
        }
        case json::Subject::TICKER_V2: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::TickerV2 ticker_v2(root, buffer);
          create_trace_and_dispatch(trace_info, ticker_v2, handler);
          break;
        }
        case json::Subject::MATCH: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Match match(root, buffer);
          create_trace_and_dispatch(trace_info, match, handler);
          break;
        }
        case json::Subject::MARK_INDEX_PRICE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::MarkIndexPrice mark_index_price(root, buffer);
          create_trace_and_dispatch(trace_info, mark_index_price, handler);
          break;
        }
        case json::Subject::FUNDING_RATE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::FundingRate funding_rate(root, buffer);
          create_trace_and_dispatch(trace_info, funding_rate, handler);
          break;
        }
        case json::Subject::LEVEL2: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Level2 level2(root, buffer);
          create_trace_and_dispatch(trace_info, level2, handler);
          break;
        }
        case json::Subject::FUNDING_BEGIN: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::FundingBegin funding_begin(root, buffer);
          create_trace_and_dispatch(trace_info, funding_begin, handler);
          break;
        }
        case json::Subject::FUNDING_END: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::FundingEnd funding_end(root, buffer);
          create_trace_and_dispatch(trace_info, funding_end, handler);
          break;
        }
        case json::Subject::SNAPSHOT_24H: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Snapshot24h snapshot_24h(root, buffer);
          create_trace_and_dispatch(trace_info, snapshot_24h, handler);
          break;
        }
        case json::Subject::ORDER_CHANGE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::OrderChange order_change(root, buffer);
          create_trace_and_dispatch(trace_info, order_change, handler);
          break;
        }
        case json::Subject::ORDER_MARGIN_CHANGE:
        case json::Subject::AVAILABLE_BALANCE_CHANGE:
        case json::Subject::WITHDRAW_HOLD_CHANGE:
        case json::Subject::POSITION_CHANGE:
        case json::Subject::POSITION_SETTLEMENT:
          log::fatal("Unexpected"_sv);
          break;
      }
      break;
  }
  return true;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
