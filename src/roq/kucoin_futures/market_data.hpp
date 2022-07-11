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

#include "roq/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/parser.hpp"

namespace roq {
namespace kucoin_futures {

class MarketData final : public core::web::ClientSocket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<MarketStatus const> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) = 0;
    virtual void operator()(Trace<TradeSummary const> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate const> const &, bool is_last) = 0;
  };

  MarketData(
      Handler &,
      io::Context &,
      uint32_t stream_id,
      Shared &,
      size_t index,
      std::string_view const &uri,
      std::string_view const &query,
      std::chrono::nanoseconds ping_frequency);

  MarketData(MarketData &&) = delete;
  MarketData(MarketData const &) = delete;

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(core::web::ClientSocket::Connected const &) override;
  void operator()(core::web::ClientSocket::Disconnected const &) override;
  void operator()(core::web::ClientSocket::Ready const &) override;
  void operator()(core::web::ClientSocket::Close const &) override;
  void operator()(core::web::ClientSocket::Latency const &) override;
  void operator()(core::web::ClientSocket::Text const &) override;
  void operator()(core::web::ClientSocket::Binary const &) override;

 private:
  void operator()(ConnectionStatus);

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &topic);
  void subscribe(std::string_view const &topic, std::span<Symbol const> const &symbols);

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

  void operator()(Trace<json::Welcome const> const &) override;
  void operator()(Trace<json::Error const> const &) override;
  void operator()(Trace<json::Pong const> const &) override;
  void operator()(Trace<json::Ack const> const &) override;

  void operator()(Trace<json::Ticker const> const &) override;
  void operator()(Trace<json::TickerV2 const> const &) override;
  void operator()(Trace<json::Match const> const &) override;
  void operator()(Trace<json::Execution const> const &) override;
  void operator()(Trace<json::MarkIndexPrice const> const &) override;
  void operator()(Trace<json::FundingRate const> const &) override;
  void operator()(Trace<json::Level2 const> const &) override;
  void operator()(Trace<json::FundingBegin const> const &) override;
  void operator()(Trace<json::FundingEnd const> const &) override;
  void operator()(Trace<json::Snapshot24h const> const &) override;

  void operator()(Trace<json::OrderChange const> const &) override;
  void operator()(Trace<json::OrderMarginChange const> const &) override;
  void operator()(Trace<json::AvailableBalanceChange const> const &) override;
  void operator()(Trace<json::WithdrawHoldChange const> const &) override;
  void operator()(Trace<json::PositionChange const> const &) override;
  void operator()(Trace<json::PositionSettlement const> const &) override;

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
    core::metrics::Profile parse, welcome, error, pong, ack, ticker_v2, ticker, match, execution, mark_index_price,
        funding_rate, level2, funding_begin, funding_end, snapshot_24h;
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
