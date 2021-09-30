/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/socket.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/kucoin_futures/market_data_state.h"
#include "roq/kucoin_futures/shared.h"

#include "roq/kucoin_futures/json/parser.h"

namespace roq {
namespace kucoin_futures {

class MarketData final : public core::web::Socket::Handler, public json::Parser::Handler {
 public:
  struct RequestL2Snapshot final {
    uint16_t stream_id = {};
    std::string_view symbol;
  };

  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<TopOfBook> &, bool is_last) = 0;
    virtual void operator()(
        const server::Trace<MarketByPriceUpdate> &, bool is_last, bool refresh) = 0;
    virtual void operator()(const server::Trace<TradeSummary> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<StatisticsUpdate> &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(RequestL2Snapshot const &) = 0;
  };

  MarketData(
      Handler &,
      core::io::Context &,
      uint32_t stream_id,
      Shared &,
      const std::string_view &uri,
      std::chrono::nanoseconds ping_frequency);

  MarketData(MarketData &&) = delete;
  MarketData(const MarketData &) = delete;

  uint16_t stream_id() const { return stream_id_; }

  bool ready() const { return ready_; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  void update_subscriptions(std::vector<std::string> &symbols);

  void check_request_queue(std::chrono::nanoseconds now);

 protected:
  void operator()(const core::web::Socket::Connected &) override;
  void operator()(const core::web::Socket::Disconnected &) override;
  void operator()(const core::web::Socket::Ready &) override;
  void operator()(const core::web::Socket::Close &) override;
  void operator()(const core::web::Socket::Latency &) override;
  void operator()(const core::web::Socket::Text &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(MarketDataState);

  void subscribe(const roq::span<std::string> &symbols);

  void subscribe(const std::string_view &topic);
  void subscribe(const std::string_view &topic, const roq::span<std::string> &symbols);

  void send_ping(std::chrono::nanoseconds now);

  void parse(const std::string_view &message);

  void operator()(server::Trace<json::Welcome> const &) override;
  void operator()(server::Trace<json::Error> const &) override;
  void operator()(server::Trace<json::Pong> const &) override;
  void operator()(server::Trace<json::Ack> const &) override;

  void operator()(server::Trace<json::Ticker> const &) override;
  void operator()(server::Trace<json::TickerV2> const &) override;
  void operator()(server::Trace<json::Match> const &) override;
  void operator()(server::Trace<json::MarkIndexPrice> const &) override;
  void operator()(server::Trace<json::FundingRate> const &) override;
  void operator()(server::Trace<json::Level2> const &) override;
  void operator()(server::Trace<json::FundingBegin> const &) override;
  void operator()(server::Trace<json::FundingEnd> const &) override;
  void operator()(server::Trace<json::Snapshot24h> const &) override;

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  const std::chrono::nanoseconds ping_frequency_;
  // web socket
  core::web::Socket connection_;
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
  std::vector<std::string> symbols_;
  // state
  bool welcome_ = false;
  bool ready_ = false;
  ConnectionStatus status_ = {};
  server::Download<MarketDataState> download_;
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  // experimental
  absl::flat_hash_map<std::string, bool> order_book_ready_;
  std::deque<std::pair<std::chrono::nanoseconds, std::string> > request_queue_;
};

}  // namespace kucoin_futures
}  // namespace roq
