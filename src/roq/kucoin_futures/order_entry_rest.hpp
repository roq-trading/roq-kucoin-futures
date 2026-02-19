/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/account.hpp"
#include "roq/kucoin_futures/order_entry.hpp"
#include "roq/kucoin_futures/order_entry_state.hpp"
#include "roq/kucoin_futures/private_token.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/token.hpp"

#include "roq/kucoin_futures/json/account_ack.hpp"
#include "roq/kucoin_futures/json/fills_ack.hpp"
#include "roq/kucoin_futures/json/orders_ack.hpp"
#include "roq/kucoin_futures/json/positions_ack.hpp"

#include "roq/kucoin_futures/json/add_order_ack.hpp"
#include "roq/kucoin_futures/json/cancel_all_orders_ack.hpp"
#include "roq/kucoin_futures/json/cancel_order_ack.hpp"

namespace roq {
namespace kucoin_futures {

struct OrderEntryREST final : public OrderEntry, public web::rest::Client::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(PrivateToken const &) = 0;
  };

  OrderEntryREST(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  OrderEntryREST(OrderEntryREST const &) = delete;

  bool ready() const override { return status_ == ConnectionStatus::READY; }

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
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(OrderEntryState state);

  // bullet-private

  void get_private_token();
  void get_private_token_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Token> const &);

  // account

  void get_account();
  void get_account_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::AccountAck> const &);

  // positions

  void get_positions();
  void get_positions_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::PositionsAck> const &);

  // orders

  void get_orders();
  void get_orders_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::OrdersAck> const &);

  // fills

  void get_fills();
  void get_fills_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::FillsAck> const &);

  // add-order

  void create_order(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::AddOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);
  void operator()(Trace<json::CancelOrderAck> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-all-orders

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);
  void operator()(Trace<json::CancelAllOrdersAck> const &);

  // helpers

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, uint8_t user_id, uint64_t order_id, Args &&...);

  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile private_token, private_token_ack,  //
        account, account_ack,                                  //
        positions, positions_ack,                              //
        orders, orders_ack,                                    //
        fills, fills_ack,                                      //
        create_order, create_order_ack,                        //
        cancel_order, cancel_order_ack,                        //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus status_ = {};
  core::Download<OrderEntryState> download_;
  //
  std::string encode_buffer_;
  //
  std::chrono::nanoseconds next_private_token_refresh_ = {};
};

}  // namespace kucoin_futures
}  // namespace roq
