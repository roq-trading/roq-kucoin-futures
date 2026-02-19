/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/account.hpp"
#include "roq/kucoin_futures/drop_copy_state.hpp"
#include "roq/kucoin_futures/private_token.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/parser.hpp"

namespace roq {
namespace kucoin_futures {

struct DropCopy final : public web::socket::Client::Handler, public json::Parser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  DropCopy(
      Handler &,
      io::Context &,
      uint16_t stream_id,
      Account &,
      Shared &,
      std::string_view const &uri,
      std::string_view const &query,
      std::chrono::nanoseconds ping_frequency);

  DropCopy(DropCopy const &) = delete;

  bool ready() const;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  void operator()(PrivateToken const &);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;
  //
  std::string_view get_query() const override { return query_; }

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void subscribe();

  void subscribe(std::string_view const &topic);

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

  void operator()(Trace<json::Welcome> const &) override;
  void operator()(Trace<json::Error> const &) override;
  void operator()(Trace<json::Pong> const &) override;
  void operator()(Trace<json::Ack> const &) override;

  void operator()(Trace<json::TickerV2> const &) override;
  void operator()(Trace<json::Match> const &) override;
  void operator()(Trace<json::Execution> const &) override;
  void operator()(Trace<json::MarkIndexPrice> const &) override;
  void operator()(Trace<json::FundingRate> const &) override;
  void operator()(Trace<json::Level2> const &) override;
  void operator()(Trace<json::FundingBegin> const &) override;
  void operator()(Trace<json::FundingEnd> const &) override;
  void operator()(Trace<json::Snapshot24h> const &) override;

  void operator()(Trace<json::WalletBalanceChange> const &) override;
  void operator()(Trace<json::OrderMarginChange> const &) override;
  void operator()(Trace<json::AvailableBalanceChange> const &) override;
  void operator()(Trace<json::WithdrawHoldChange> const &) override;
  void operator()(Trace<json::PositionChange> const &) override;
  void operator()(Trace<json::PositionSettlement> const &) override;
  void operator()(Trace<json::PositionAdjustRiskLimit> const &) override;
  void operator()(Trace<json::SymbolOrderChange> const &) override;
  void operator()(Trace<json::OrderChange> const &) override;

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::string query_;
  std::unique_ptr<web::socket::Client> const connection_;
  std::chrono::nanoseconds const ping_frequency_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,  //
        welcome, error, pong, ack;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
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
