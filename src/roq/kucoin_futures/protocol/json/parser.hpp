/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kucoin_futures/protocol/json/ack.hpp"
#include "roq/kucoin_futures/protocol/json/error.hpp"
#include "roq/kucoin_futures/protocol/json/pong.hpp"
#include "roq/kucoin_futures/protocol/json/welcome.hpp"

#include "roq/kucoin_futures/protocol/json/execution.hpp"
#include "roq/kucoin_futures/protocol/json/funding_begin.hpp"
#include "roq/kucoin_futures/protocol/json/funding_end.hpp"
#include "roq/kucoin_futures/protocol/json/funding_rate.hpp"
#include "roq/kucoin_futures/protocol/json/level2.hpp"
#include "roq/kucoin_futures/protocol/json/mark_index_price.hpp"
#include "roq/kucoin_futures/protocol/json/match.hpp"
#include "roq/kucoin_futures/protocol/json/snapshot24h.hpp"
#include "roq/kucoin_futures/protocol/json/ticker_v2.hpp"

#include "roq/kucoin_futures/protocol/json/available_balance_change.hpp"
#include "roq/kucoin_futures/protocol/json/order_change.hpp"
#include "roq/kucoin_futures/protocol/json/order_margin_change.hpp"
#include "roq/kucoin_futures/protocol/json/position_adjust_risk_limit.hpp"
#include "roq/kucoin_futures/protocol/json/position_change.hpp"
#include "roq/kucoin_futures/protocol/json/position_settlement.hpp"
#include "roq/kucoin_futures/protocol/json/symbol_order_change.hpp"
#include "roq/kucoin_futures/protocol/json/wallet_balance_change.hpp"
#include "roq/kucoin_futures/protocol/json/withdraw_hold_change.hpp"

namespace roq {
namespace kucoin_futures {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::Welcome> const &) = 0;
    virtual void operator()(Trace<protocol::json::Error> const &) = 0;
    virtual void operator()(Trace<protocol::json::Pong> const &) = 0;
    virtual void operator()(Trace<protocol::json::Ack> const &) = 0;

    virtual void operator()(Trace<protocol::json::TickerV2> const &) = 0;
    virtual void operator()(Trace<protocol::json::Match> const &) = 0;
    virtual void operator()(Trace<protocol::json::Execution> const &) = 0;
    virtual void operator()(Trace<protocol::json::MarkIndexPrice> const &) = 0;
    virtual void operator()(Trace<protocol::json::FundingRate> const &) = 0;
    virtual void operator()(Trace<protocol::json::Level2> const &) = 0;
    virtual void operator()(Trace<protocol::json::FundingBegin> const &) = 0;
    virtual void operator()(Trace<protocol::json::FundingEnd> const &) = 0;
    virtual void operator()(Trace<protocol::json::Snapshot24h> const &) = 0;

    virtual void operator()(Trace<protocol::json::WalletBalanceChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::OrderMarginChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::AvailableBalanceChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::WithdrawHoldChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::PositionChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::PositionSettlement> const &) = 0;
    virtual void operator()(Trace<protocol::json::PositionAdjustRiskLimit> const &) = 0;
    virtual void operator()(Trace<protocol::json::SymbolOrderChange> const &) = 0;
    virtual void operator()(Trace<protocol::json::OrderChange> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace kucoin_futures
}  // namespace roq
