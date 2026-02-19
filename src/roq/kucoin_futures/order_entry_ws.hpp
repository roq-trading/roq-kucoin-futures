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
#include "roq/kucoin_futures/order_entry.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/ws_parser.hpp"

namespace roq {
namespace kucoin_futures {

struct OrderEntryWS final : public OrderEntry, public web::socket::Client::Handler, public json::WSParser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  OrderEntryWS(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  OrderEntryWS(OrderEntryWS const &) = delete;

  bool ready() const override;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

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

  void send_ping(std::chrono::nanoseconds now);

  void parse(std::string_view const &message);

  void operator()(Trace<json::WSAuth> const &, std::string_view const &message) override;
  void operator()(Trace<json::WSWelcome> const &) override;
  void operator()(Trace<json::WSError> const &) override;
  void operator()(Trace<json::WSPong> const &) override;
  void operator()(Trace<json::WSAddOrderAck> const &) override;
  void operator()(Trace<json::WSCancelOrderAck> const &) override;

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::string query_;
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse,  //
        auth, welcome, error, pong, add_order_ack, cancel_order_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  Shared &shared_;
  // state
  bool welcome_ = false;
  std::chrono::nanoseconds ping_freq_ = {};
  bool ready_ = false;
  ConnectionStatus status_ = {};
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  //
  std::string encode_buffer_;
};

}  // namespace kucoin_futures
}  // namespace roq
