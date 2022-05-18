/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/json/ack.hpp"
#include "roq/kucoin_futures/json/error.hpp"
#include "roq/kucoin_futures/json/pong.hpp"
#include "roq/kucoin_futures/json/welcome.hpp"

#include "roq/kucoin_futures/json/funding_begin.hpp"
#include "roq/kucoin_futures/json/funding_end.hpp"
#include "roq/kucoin_futures/json/funding_rate.hpp"
#include "roq/kucoin_futures/json/level2.hpp"
#include "roq/kucoin_futures/json/mark_index_price.hpp"
#include "roq/kucoin_futures/json/match.hpp"
#include "roq/kucoin_futures/json/snapshot24h.hpp"
#include "roq/kucoin_futures/json/ticker.hpp"
#include "roq/kucoin_futures/json/ticker_v2.hpp"

#include "roq/kucoin_futures/json/available_balance_change.hpp"
#include "roq/kucoin_futures/json/order_change.hpp"
#include "roq/kucoin_futures/json/order_margin_change.hpp"
#include "roq/kucoin_futures/json/position_change.hpp"
#include "roq/kucoin_futures/json/position_settlement.hpp"
#include "roq/kucoin_futures/json/withdraw_hold_change.hpp"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Welcome const> const &) = 0;
    virtual void operator()(Trace<json::Error const> const &) = 0;
    virtual void operator()(Trace<json::Pong const> const &) = 0;
    virtual void operator()(Trace<json::Ack const> const &) = 0;

    virtual void operator()(Trace<json::Ticker const> const &) = 0;
    virtual void operator()(Trace<json::TickerV2 const> const &) = 0;
    virtual void operator()(Trace<json::Match const> const &) = 0;
    virtual void operator()(Trace<json::MarkIndexPrice const> const &) = 0;
    virtual void operator()(Trace<json::FundingRate const> const &) = 0;
    virtual void operator()(Trace<json::Level2 const> const &) = 0;
    virtual void operator()(Trace<json::FundingBegin const> const &) = 0;
    virtual void operator()(Trace<json::FundingEnd const> const &) = 0;
    virtual void operator()(Trace<json::Snapshot24h const> const &) = 0;

    virtual void operator()(Trace<json::OrderChange const> const &) = 0;
    virtual void operator()(Trace<json::OrderMarginChange const> const &) = 0;
    virtual void operator()(Trace<json::AvailableBalanceChange const> const &) = 0;
    virtual void operator()(Trace<json::WithdrawHoldChange const> const &) = 0;
    virtual void operator()(Trace<json::PositionChange const> const &) = 0;
    virtual void operator()(Trace<json::PositionSettlement const> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::Buffer &, TraceInfo const &);
};

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
