/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/protocol/json/parser.hpp"

namespace roq {
namespace kucoin_futures {

template <typename T>
struct ParserTester final : public protocol::json::Parser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    ParserTester handler{callback};
    auto res = protocol::json::Parser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit ParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<protocol::json::Welcome> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Error> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Pong> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Ack> const &event) override { dispatch(event); }

  void operator()(Trace<protocol::json::TickerV2> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Match> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Execution> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::MarkIndexPrice> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::FundingRate> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Level2> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::FundingBegin> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::FundingEnd> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::Snapshot24h> const &event) override { dispatch(event); }

  void operator()(Trace<protocol::json::WalletBalanceChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::OrderMarginChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::AvailableBalanceChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::WithdrawHoldChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::PositionChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::PositionSettlement> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::PositionAdjustRiskLimit> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::SymbolOrderChange> const &event) override { dispatch(event); }
  void operator()(Trace<protocol::json::OrderChange> const &event) override { dispatch(event); }

  template <typename U>
  void dispatch(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace kucoin_futures
}  // namespace roq
