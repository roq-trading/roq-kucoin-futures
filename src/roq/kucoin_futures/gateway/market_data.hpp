/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/timer_queue.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kucoin_futures/gateway/shared.hpp"

#include "roq/kucoin_futures/protocol/json/parser.hpp"

namespace roq {
namespace kucoin_futures {
namespace gateway {

struct MarketData final : public Base<MarketData>,
                          public server::MarketDataStream,
                          public web::socket::Client::Handler,
                          public protocol::json::Parser::Handler {
  struct Handler {};

  MarketData(
      Handler &,
      io::Context &,
      uint16_t stream_id,
      Shared &,
      size_t index,
      std::string_view const &uri,
      std::string_view const &query,
      std::chrono::nanoseconds ping_frequency);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::MarketDataStream

  void subscribe(size_t start_from = 0) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;
  //
  std::string_view get_query() const override { return query_; }

  // protocol::json::Parser::Handler

  void operator()(Trace<protocol::json::Welcome> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;
  void operator()(Trace<protocol::json::Pong> const &) override;
  void operator()(Trace<protocol::json::Ack> const &) override;

  void operator()(Trace<protocol::json::TickerV2> const &) override;
  void operator()(Trace<protocol::json::Match> const &) override;
  void operator()(Trace<protocol::json::Execution> const &) override;
  void operator()(Trace<protocol::json::MarkIndexPrice> const &) override;
  void operator()(Trace<protocol::json::FundingRate> const &) override;
  void operator()(Trace<protocol::json::Level2> const &) override;
  void operator()(Trace<protocol::json::FundingBegin> const &) override;
  void operator()(Trace<protocol::json::FundingEnd> const &) override;
  void operator()(Trace<protocol::json::Snapshot24h> const &) override;

  void operator()(Trace<protocol::json::WalletBalanceChange> const &) override;
  void operator()(Trace<protocol::json::OrderMarginChange> const &) override;
  void operator()(Trace<protocol::json::AvailableBalanceChange> const &) override;
  void operator()(Trace<protocol::json::WithdrawHoldChange> const &) override;
  void operator()(Trace<protocol::json::PositionChange> const &) override;
  void operator()(Trace<protocol::json::PositionSettlement> const &) override;
  void operator()(Trace<protocol::json::PositionAdjustRiskLimit> const &) override;
  void operator()(Trace<protocol::json::SymbolOrderChange> const &) override;
  void operator()(Trace<protocol::json::OrderChange> const &) override;

  // helpers

  void check_subscribe_queue(std::chrono::nanoseconds now);

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols);

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
  std::chrono::nanoseconds const ping_frequency_;
  // web socket
  std::string query_;
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect, total_bytes_received;
  } counter_;
  struct {
    utils::metrics::Profile parse, welcome, error, pong, ack, ticker_v2, ticker, match, execution, mark_index_price, funding_rate, level2, funding_begin,
        funding_end, snapshot_24h;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  std::vector<Symbol> symbols_;
  // state
  bool welcome_ = false;
  ConnectionStatus connection_status_ = {};
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  // queue
  core::TimerQueue<std::string> subscribe_queue_;
};

}  // namespace gateway
}  // namespace kucoin_futures
}  // namespace roq
