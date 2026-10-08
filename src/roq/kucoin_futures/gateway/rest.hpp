/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kucoin_futures/gateway/shared.hpp"

#include "roq/kucoin_futures/protocol/json/token.hpp"

#include "roq/kucoin_futures/protocol/json/contracts_ack.hpp"
#include "roq/kucoin_futures/protocol/json/order_book_ack.hpp"

namespace roq {
namespace kucoin_futures {
namespace gateway {

struct Rest final : public Base<Rest>, public server::Stream, public web::rest::Client::Handler {
  struct PublicToken final {
    std::string_view uri;
    std::string_view query;
    std::chrono::nanoseconds ping_frequency = {};
  };

  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct Handler {
    virtual void operator()(PublicToken const &) = 0;
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;

  // core::Download

  enum class State {
    UNDEFINED = 0,
    PUBLIC_TOKEN,
    CONTRACTS,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // bullet-public

  void get_public_token();
  void get_public_token_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::Token> const &);

  // contracts

  void get_contracts();
  void get_contracts_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::ContractsAck> const &);

  // order-book

  void get_order_book(std::string_view const &symbol);
  void get_order_book_ack(Trace<web::rest::Response> const &, std::string_view const &symbol);
  void operator()(Trace<protocol::json::OrderBookAck> const &);

  // helpers

  void check_request_queue(std::chrono::nanoseconds now);

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

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
    utils::metrics::Profile public_token, public_token_ack,  //
        contracts, contracts_ack,                            //
        order_book, order_book_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
};

}  // namespace gateway
}  // namespace kucoin_futures
}  // namespace roq
