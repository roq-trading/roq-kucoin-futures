/* Copyright (c) 2017-2023, Hans Erik Thrane */

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

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server.hpp"

#include "roq/kucoin_futures/rest_state.hpp"
#include "roq/kucoin_futures/shared.hpp"

#include "roq/kucoin_futures/json/contracts.hpp"
#include "roq/kucoin_futures/json/order_book.hpp"
#include "roq/kucoin_futures/json/token.hpp"

namespace roq {
namespace kucoin_futures {

class Rest final : public web::rest::Client::Handler {
 public:
  struct PublicToken final {
    std::string_view uri;
    std::string_view query;
    std::chrono::nanoseconds ping_frequency = {};
  };

  struct SymbolsUpdate final {
    std::vector<Symbol> &symbols;
  };

  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate> const &, bool is_last, bool refresh) = 0;
    // cross-communication
    virtual void operator()(PublicToken const &) = 0;
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

  Rest(Rest &&) = delete;
  Rest(Rest const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(web::rest::Client::Connected const &) override;
  void operator()(web::rest::Client::Disconnected const &) override;
  void operator()(web::rest::Client::Latency const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(RestState);

  void get_public_token();
  void get_public_token_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Token> const &);

  void get_contracts();
  void get_contracts_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<json::Contracts> const &);

  void get_order_book(std::string_view const &symbol);
  void get_order_book_ack(Trace<web::rest::Response> const &, std::string_view const &symbol);
  void operator()(Trace<json::OrderBook> const &);

  void check_request_queue(std::chrono::nanoseconds now);

  template <typename SuccessHandler, typename ErrorHandler>
  void process_response(web::rest::Response const &, SuccessHandler, ErrorHandler);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // connection
  std::unique_ptr<web::rest::Client> connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile public_token, public_token_ack,  //
        contracts, contracts_ack,                           //
        order_book, order_book_ack;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  absl::flat_hash_set<Symbol> all_symbols_;
  // state
  ConnectionStatus status_ = {};
  core::Download<RestState> download_;
};

}  // namespace kucoin_futures
}  // namespace roq
