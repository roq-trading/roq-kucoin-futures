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
    virtual void operator()(const Trace<StreamStatus> &) = 0;
    virtual void operator()(const Trace<ExternalLatency> &) = 0;
    virtual void operator()(const Trace<FundsUpdate> &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(PrivateToken const &) = 0;
  };

  OrderEntry(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(const OrderEntry &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id);

 protected:
  void operator()(const core::web::Client::Connected &);
  void operator()(const core::web::Client::Disconnected &);
  void operator()(const core::web::Client::Latency &);

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  void get_private_token();
  void get_private_token_ack(const Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const Trace<json::Token> &);

  void get_account();
  void get_account_ack(const Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const Trace<json::Account> &);

  void get_positions();
  void get_positions_ack(const Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const Trace<json::Positions> &);

  void get_orders();
  void get_orders_ack(const Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const Trace<json::Orders> &);

  void get_fills();
  void get_fills_ack(const Trace<core::web::Response> &, uint32_t sequence);
  void operator()(const Trace<json::Fills> &);

  void create_order(
      const Event<CreateOrder> &, const oms::Order &, const std::string_view &request_id);
  void create_order_ack(
      const Trace<core::web::Response> &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_order(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  void cancel_order_ack(
      const Trace<core::web::Response> &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_all_orders(const Event<CancelAllOrders> &, const std::string_view &request_id);
  void cancel_all_orders_ack(const Trace<core::web::Response> &);

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
