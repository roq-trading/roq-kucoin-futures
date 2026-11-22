/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kucoin_futures/gateway/account.hpp"
#include "roq/kucoin_futures/gateway/shared.hpp"

#include "roq/kucoin_futures/protocol/json/ws_parser.hpp"

namespace roq {
namespace kucoin_futures {
namespace gateway {

struct OrderEntryWS final : public Base<OrderEntryWS>,
                            public server::OrderActionStream,
                            public web::socket::Client::Handler,
                            public protocol::json::WSParser::Handler {
  struct Handler {};

  OrderEntryWS(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

 protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override;

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::OrderActionStream

  void operator()(Trace<server::oms::OrderActionBatchBegin> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchCommit> const &) override;
  void operator()(Trace<server::oms::OrderActionBatchRollback> const &) override;

  uint16_t operator()(Trace<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Trace<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Trace<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Trace<CancelAllOrders> const &, std::string_view const &request_id) override;

  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;
  //
  std::string_view get_query() const override;

  // protocol::json::WSParser::Handler

  void operator()(Trace<protocol::json::WSAuth> const &, std::string_view const &message) override;
  void operator()(Trace<protocol::json::WSWelcome> const &) override;
  void operator()(Trace<protocol::json::WSError> const &) override;
  void operator()(Trace<protocol::json::WSPong> const &) override;
  void operator()(Trace<protocol::json::WSAddOrderAck> const &) override;
  void operator()(Trace<protocol::json::WSCancelOrderAck> const &) override;

  // helpers

  void send_ping(std::chrono::nanoseconds now);

  void parse(TraceInfo const &, std::string_view const &message);

 private:
  [[maybe_unused]] Handler &handler_;
  //
  uint16_t const stream_id_;
  std::string const name_;
  std::unique_ptr<web::socket::Client> const connection_;
  //
  core::json::BufferStack decode_buffer_;
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,   //
        auth, welcome, error, pong,  //
        add_order_ack, cancel_order_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  Account &account_;
  Shared &shared_;
  bool welcome_ = false;
  std::chrono::nanoseconds ping_freq_ = {};
  ConnectionStatus connection_status_ = {};
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  std::string encode_buffer_;
  //
  std::string query_buffer_;
};

}  // namespace gateway
}  // namespace kucoin_futures
}  // namespace roq
