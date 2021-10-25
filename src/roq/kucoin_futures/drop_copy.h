/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/socket.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/kucoin_futures/drop_copy_state.h"
#include "roq/kucoin_futures/security.h"
#include "roq/kucoin_futures/shared.h"

#include "roq/kucoin_futures/json/parser.h"

namespace roq {
namespace kucoin_futures {

class DropCopy final : public core::web::Socket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<FundsUpdate> &, bool is_last) = 0;
  };

  DropCopy(
      Handler &,
      core::io::Context &,
      uint16_t stream_id,
      Security &,
      Shared &,
      const std::string_view &uri,
      const std::string_view &query,
      std::chrono::nanoseconds ping_frequency);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  bool ready() const;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::Socket::Connected &) override;
  void operator()(const core::web::Socket::Disconnected &) override;
  void operator()(const core::web::Socket::Ready &) override;
  void operator()(const core::web::Socket::Close &) override;
  void operator()(const core::web::Socket::Latency &) override;
  void operator()(const core::web::Socket::Text &) override;
  void operator()(const core::web::Socket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void subscribe();

  void subscribe(const std::string_view &topic);

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

  void operator()(server::Trace<json::OrderChange> const &) override;
  void operator()(server::Trace<json::OrderMarginChange> const &) override;
  void operator()(server::Trace<json::AvailableBalanceChange> const &) override;
  void operator()(server::Trace<json::WithdrawHoldChange> const &) override;
  void operator()(server::Trace<json::PositionChange> const &) override;
  void operator()(server::Trace<json::PositionSettlement> const &) override;

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::Socket connection_;
  const std::chrono::nanoseconds ping_frequency_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse,  //
        welcome, error, pong, ack;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  bool welcome_ = false;
  bool ready_ = false;
  ConnectionStatus status_ = {};
  server::Download<DropCopyState> download_;
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace kucoin_futures
}  // namespace roq
