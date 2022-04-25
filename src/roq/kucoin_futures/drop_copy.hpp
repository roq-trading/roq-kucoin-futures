/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/drop_copy_state.hpp"
#include "roq/kucoin_futures/security.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/parser.hpp"

namespace roq {
namespace kucoin_futures {

class DropCopy final : public core::web::ClientSocket::Handler, public json::Parser::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus const> &) = 0;
    virtual void operator()(const Trace<ExternalLatency const> &) = 0;
    virtual void operator()(const Trace<FundsUpdate const> &, bool is_last) = 0;
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
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void subscribe();

  void subscribe(const std::string_view &topic);

  void send_ping(std::chrono::nanoseconds now);

  void parse(const std::string_view &message);

  void operator()(Trace<json::Welcome const> const &) override;
  void operator()(Trace<json::Error const> const &) override;
  void operator()(Trace<json::Pong const> const &) override;
  void operator()(Trace<json::Ack const> const &) override;

  void operator()(Trace<json::Ticker const> const &) override;
  void operator()(Trace<json::TickerV2 const> const &) override;
  void operator()(Trace<json::Match const> const &) override;
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

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::ClientSocket connection_;
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
  core::Download<DropCopyState> download_;
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
};

}  // namespace kucoin_futures
}  // namespace roq
