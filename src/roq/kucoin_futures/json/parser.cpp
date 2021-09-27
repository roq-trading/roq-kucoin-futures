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
      log::fatal("Unexpected"_sv);
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
        case json::Subject::TRADE_SNAPSHOT: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Snapshot snapshot(root, buffer);
          create_trace_and_dispatch(trace_info, snapshot, handler);
          break;
        }
        case json::Subject::TRADE_TICKER: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Ticker ticker(root, buffer);
          create_trace_and_dispatch(trace_info, ticker, handler);
          break;
        }
        case json::Subject::TRADE_L2UPDATE: {
          core::json::Parser parser(message);
          auto root = parser.root();
          json::Level2 level2(root, buffer);
          create_trace_and_dispatch(trace_info, level2, handler);
          break;
        }
      }
      break;
  }
  return true;
}

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
