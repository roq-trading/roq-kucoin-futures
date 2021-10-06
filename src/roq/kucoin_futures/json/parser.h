/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kucoin_futures/json/ack.h"
#include "roq/kucoin_futures/json/error.h"
#include "roq/kucoin_futures/json/pong.h"
#include "roq/kucoin_futures/json/welcome.h"

#include "roq/kucoin_futures/json/funding_begin.h"
#include "roq/kucoin_futures/json/funding_end.h"
#include "roq/kucoin_futures/json/funding_rate.h"
#include "roq/kucoin_futures/json/level2.h"
#include "roq/kucoin_futures/json/mark_index_price.h"
#include "roq/kucoin_futures/json/match.h"
#include "roq/kucoin_futures/json/snapshot24h.h"
#include "roq/kucoin_futures/json/ticker.h"
#include "roq/kucoin_futures/json/ticker_v2.h"

#include "roq/kucoin_futures/json/available_balance_change.h"
#include "roq/kucoin_futures/json/order_change.h"
#include "roq/kucoin_futures/json/order_margin_change.h"
#include "roq/kucoin_futures/json/position_change.h"
#include "roq/kucoin_futures/json/position_settlement.h"
#include "roq/kucoin_futures/json/withdraw_hold_change.h"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(server::Trace<json::Welcome> const &) = 0;
    virtual void operator()(server::Trace<json::Error> const &) = 0;
    virtual void operator()(server::Trace<json::Pong> const &) = 0;
    virtual void operator()(server::Trace<json::Ack> const &) = 0;

    virtual void operator()(server::Trace<json::Ticker> const &) = 0;
    virtual void operator()(server::Trace<json::TickerV2> const &) = 0;
    virtual void operator()(server::Trace<json::Match> const &) = 0;
    virtual void operator()(server::Trace<json::MarkIndexPrice> const &) = 0;
    virtual void operator()(server::Trace<json::FundingRate> const &) = 0;
    virtual void operator()(server::Trace<json::Level2> const &) = 0;
    virtual void operator()(server::Trace<json::FundingBegin> const &) = 0;
    virtual void operator()(server::Trace<json::FundingEnd> const &) = 0;
    virtual void operator()(server::Trace<json::Snapshot24h> const &) = 0;

    virtual void operator()(server::Trace<json::OrderChange> const &) = 0;
    virtual void operator()(server::Trace<json::OrderMarginChange> const &) = 0;
    virtual void operator()(server::Trace<json::AvailableBalanceChange> const &) = 0;
    virtual void operator()(server::Trace<json::WithdrawHoldChange> const &) = 0;
    virtual void operator()(server::Trace<json::PositionChange> const &) = 0;
    virtual void operator()(server::Trace<json::PositionSettlement> const &) = 0;
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
