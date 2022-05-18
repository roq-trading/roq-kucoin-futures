/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/buffer.hpp"
#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/order_entry_state.hpp"
#include "roq/kucoin_futures/security.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/account.hpp"
#include "roq/kucoin_futures/json/fills.hpp"
#include "roq/kucoin_futures/json/orders.hpp"
#include "roq/kucoin_futures/json/positions.hpp"
#include "roq/kucoin_futures/json/token.hpp"

namespace roq {
namespace kucoin_futures {

class OrderEntry final : public core::web::Client::Handler {
 public:
  struct PrivateToken final {
    std::string_view account;
    std::string_view uri;
    std::string_view query;
    std::chrono::nanoseconds ping_frequency = {};
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<FundsUpdate const> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(PrivateToken const &) = 0;
  };

  OrderEntry(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(OrderEntry const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  void operator()(core::web::Client::Connected const &);
  void operator()(core::web::Client::Disconnected const &);
  void operator()(core::web::Client::Latency const &);

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  void get_private_token();
  void get_private_token_ack(Trace<core::web::Response const> const &, uint32_t sequence);
  void operator()(Trace<json::Token const> const &);

  void get_account();
  void get_account_ack(Trace<core::web::Response const> const &, uint32_t sequence);
  void operator()(Trace<json::Account const> const &);

  void get_positions();
  void get_positions_ack(Trace<core::web::Response const> const &, uint32_t sequence);
  void operator()(Trace<json::Positions const> const &);

  void get_orders();
  void get_orders_ack(Trace<core::web::Response const> const &, uint32_t sequence);
  void operator()(Trace<json::Orders const> const &);

  void get_fills();
  void get_fills_ack(Trace<core::web::Response const> const &, uint32_t sequence);
  void operator()(Trace<json::Fills const> const &);

  void create_order(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  void create_order_ack(Trace<core::web::Response const> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_order(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<core::web::Response const> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<core::web::Response const> const &);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  core::web::Client connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile private_token, private_token_ack,  //
        account, account_ack,                                 //
        positions, positions_ack,                             //
        orders, orders_ack,                                   //
        fills, fills_ack,                                     //
        create_order, create_order_ack,                       //
        cancel_order, cancel_order_ack,                       //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
};

}  // namespace kucoin_futures
}  // namespace roq
