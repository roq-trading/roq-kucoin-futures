/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/buffer.hpp"
#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/account.hpp"
#include "roq/kucoin_futures/order_entry_state.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/account.hpp"
#include "roq/kucoin_futures/json/fills.hpp"
#include "roq/kucoin_futures/json/orders.hpp"
#include "roq/kucoin_futures/json/positions.hpp"
#include "roq/kucoin_futures/json/token.hpp"

namespace roq {
namespace kucoin_futures {

struct OrderEntry final : public web::rest::Client::Handler {
  struct PrivateToken final {
    std::string_view account;
    std::string_view uri;
    std::string_view query;
    std::chrono::nanoseconds ping_frequency = {};
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(PrivateToken const &) = 0;
  };

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

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
  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;
  void operator()(Trace<web::rest::Response> const &, uint64_t request_id, uint64_t opaque) override;

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  void get_private_token();
  void get_private_token_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Token> const &);

  void get_account();
  void get_account_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Account> const &);

  void get_positions();
  void get_positions_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Positions> const &);

  void get_orders();
  void get_orders_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Orders> const &);

  void get_fills();
  void get_fills_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Fills> const &);

  void create_order(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_order(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint32_t order_id, uint32_t version);

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &);

  template <typename SuccessHandler, typename ErrorHandler>
  void process_response(web::rest::Response const &, SuccessHandler, ErrorHandler);

  template <typename... Args>
  void operator()(Trace<oms::Response> const &, uint8_t user_id, uint32_t order_id, Args &&...);

  void operator()(Trace<oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
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
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
};

}  // namespace kucoin_futures
}  // namespace roq
