/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/json/ack.hpp"
#include "roq/kucoin_futures/json/error.hpp"
#include "roq/kucoin_futures/json/pong.hpp"
#include "roq/kucoin_futures/json/welcome.hpp"

#include "roq/kucoin_futures/json/execution.hpp"
#include "roq/kucoin_futures/json/funding_begin.hpp"
#include "roq/kucoin_futures/json/funding_end.hpp"
#include "roq/kucoin_futures/json/funding_rate.hpp"
#include "roq/kucoin_futures/json/level2.hpp"
#include "roq/kucoin_futures/json/mark_index_price.hpp"
#include "roq/kucoin_futures/json/match.hpp"
#include "roq/kucoin_futures/json/snapshot24h.hpp"
#include "roq/kucoin_futures/json/ticker_v2.hpp"

#include "roq/kucoin_futures/json/available_balance_change.hpp"
#include "roq/kucoin_futures/json/order_change.hpp"
#include "roq/kucoin_futures/json/order_margin_change.hpp"
#include "roq/kucoin_futures/json/position_adjust_risk_limit.hpp"
#include "roq/kucoin_futures/json/position_change.hpp"
#include "roq/kucoin_futures/json/position_settlement.hpp"
#include "roq/kucoin_futures/json/symbol_order_change.hpp"
#include "roq/kucoin_futures/json/wallet_balance_change.hpp"
#include "roq/kucoin_futures/json/withdraw_hold_change.hpp"

namespace roq {
namespace kucoin_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Welcome> const &) = 0;
    virtual void operator()(Trace<json::Error> const &) = 0;
    virtual void operator()(Trace<json::Pong> const &) = 0;
    virtual void operator()(Trace<json::Ack> const &) = 0;

    virtual void operator()(Trace<json::TickerV2> const &) = 0;
    virtual void operator()(Trace<json::Match> const &) = 0;
    virtual void operator()(Trace<json::Execution> const &) = 0;
    virtual void operator()(Trace<json::MarkIndexPrice> const &) = 0;
    virtual void operator()(Trace<json::FundingRate> const &) = 0;
    virtual void operator()(Trace<json::Level2> const &) = 0;
    virtual void operator()(Trace<json::FundingBegin> const &) = 0;
    virtual void operator()(Trace<json::FundingEnd> const &) = 0;
    virtual void operator()(Trace<json::Snapshot24h> const &) = 0;

    virtual void operator()(Trace<json::WalletBalanceChange> const &) = 0;
    virtual void operator()(Trace<json::OrderMarginChange> const &) = 0;
    virtual void operator()(Trace<json::AvailableBalanceChange> const &) = 0;
    virtual void operator()(Trace<json::WithdrawHoldChange> const &) = 0;
    virtual void operator()(Trace<json::PositionChange> const &) = 0;
    virtual void operator()(Trace<json::PositionSettlement> const &) = 0;
    virtual void operator()(Trace<json::PositionAdjustRiskLimit> const &) = 0;
    virtual void operator()(Trace<json::SymbolOrderChange> const &) = 0;
    virtual void operator()(Trace<json::OrderChange> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace kucoin_futures
}  // namespace roq
