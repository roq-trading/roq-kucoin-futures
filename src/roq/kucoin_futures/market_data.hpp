/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "roq/core/download.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/parser.hpp"

namespace roq {
namespace kucoin_futures {

class MarketData final : public core::web::ClientSocket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus> &) = 0;
    virtual void operator()(const Trace<ExternalLatency> &) = 0;
    virtual void operator()(const Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const Trace<TopOfBook> &, bool is_last) = 0;
    virtual void operator()(const Trace<MarketByPriceUpdate> &, bool is_last, bool refresh) = 0;
    virtual void operator()(const Trace<TradeSummary> &, bool is_last) = 0;
    virtual void operator()(const Trace<StatisticsUpdate> &, bool is_last) = 0;
  };

  MarketData(
      Handler &,
      core::io::Context &,
      uint32_t stream_id,
      Shared &,
      size_t index,
      const std::string_view &uri,
      const std::string_view &query,
      std::chrono::nanoseconds ping_frequency);

  MarketData(MarketData &&) = delete;
  MarketData(const MarketData &) = delete;

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe(const std::span<Symbol const> &symbols);

  void subscribe(const std::string_view &topic);
  void subscribe(const std::string_view &topic, const std::span<Symbol const> &symbols);

  void send_ping(std::chrono::nanoseconds now);

  void parse(const std::string_view &message);

  void operator()(Trace<json::Welcome> const &) override;
  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::Pong> const &) override;
  void operator()(Trace<json::Ack> const &) override;

  void operator()(Trace<json::Ticker> const &) override;
  void operator()(Trace<json::TickerV2> const &) override;
  void operator()(Trace<json::Match> const &) override;
  void operator()(Trace<json::MarkIndexPrice> const &) override;
  void operator()(Trace<json::FundingRate> const &) override;
  void operator()(Trace<json::Level2> const &) override;
  void operator()(Trace<json::FundingBegin> const &) override;
  void operator()(Trace<json::FundingEnd> const &) override;
  void operator()(Trace<json::Snapshot24h> const &) override;

  void operator()(Trace<json::OrderChange> const &) override;
  void operator()(Trace<json::OrderMarginChange> const &) override;
  void operator()(Trace<json::AvailableBalanceChange> const &) override;
  void operator()(Trace<json::WithdrawHoldChange> const &) override;
  void operator()(Trace<json::PositionChange> const &) override;
  void operator()(Trace<json::PositionSettlement> const &) override;

  void check_subscribe_queue(std::chrono::nanoseconds now);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  const size_t index_;
  const std::chrono::nanoseconds ping_frequency_;
  // web socket
  core::web::ClientSocket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // session
  uint64_t request_id_ = {};
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, welcome, error, pong, ack, ticker_v2, ticker, match,
        mark_index_price, funding_rate, level2, funding_begin, funding_end, snapshot_24h;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  std::vector<Symbol> symbols_;
  // state
  bool welcome_ = false;
  ConnectionStatus status_ = {};
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  // queue
  core::TimerQueue subscribe_queue_;
};

}  // namespace kucoin_futures
}  // namespace roq
