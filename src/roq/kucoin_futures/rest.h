/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/promise.h"

#include "roq/core/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/kucoin_futures/rest_state.h"
#include "roq/kucoin_futures/security.h"
#include "roq/kucoin_futures/shared.h"

#include "roq/kucoin_futures/json/contracts.h"
#include "roq/kucoin_futures/json/order_book.h"
#include "roq/kucoin_futures/json/token.h"

namespace roq {
namespace kucoin_futures {

class Rest final : public core::web::Client::Handler {
 public:
  struct PublicToken final {
    std::string_view token;
    std::string_view endpoint;
    std::string_view uri;
    std::chrono::nanoseconds ping_frequency = {};
  };

  struct SymbolsUpdate final {
    std::vector<std::string> &symbols;
  };

  struct Level2Snapshot final {
    std::string_view symbol;
    int64_t sequence = {};  // <=0 means failed
    uint16_t stream_id = {};
  };

  struct Handler {
    virtual void operator()(server::Trace<StreamStatus> const &) = 0;
    virtual void operator()(server::Trace<ExternalLatency> const &) = 0;
    virtual void operator()(server::Trace<ReferenceData> const &, bool is_last) = 0;
    virtual void operator()(server::Trace<MarketStatus> const &, bool is_last) = 0;
    virtual void operator()(server::Trace<MarketByPriceUpdate> const &, bool is_last) = 0;
    // cross-communication
    virtual void operator()(PublicToken const &) = 0;
    virtual void operator()(SymbolsUpdate &) = 0;
    virtual void operator()(Level2Snapshot const &) = 0;
  };

  Rest(Handler &, core::io::Context &context, uint16_t stream_id, Security &, Shared &);

  Rest(Rest &&) = delete;
  Rest(const Rest &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  void get_order_book(const std::string_view &symbol, uint16_t stream_id);

  void operator()(server::Trace<json::OrderBook> const &);

 protected:
  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

  void operator()(ConnectionStatus);

  template <typename T>
  void get(std::function<void(const core::Promise<T> &)> &&callback);

  uint32_t download(RestState);

  void download_public_token();
  void download_contracts();

  void operator()(json::Token const &);
  void operator()(json::Contracts const &);

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
    core::metrics::Profile public_token, contracts, order_book;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  absl::flat_hash_set<std::string> all_symbols_;
  // state
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  server::Download<RestState> download_;
};

}  // namespace kucoin_futures
}  // namespace roq
