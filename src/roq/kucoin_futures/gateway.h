/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "roq/server.h"

#include "roq/core/io/context.h"

#include "roq/kucoin_futures/config.h"
#include "roq/kucoin_futures/drop_copy.h"
#include "roq/kucoin_futures/market_data.h"
#include "roq/kucoin_futures/order_entry.h"
#include "roq/kucoin_futures/rest.h"
#include "roq/kucoin_futures/security.h"
#include "roq/kucoin_futures/shared.h"

namespace roq {
namespace kucoin_futures {

class Gateway final : public server::Handler,
                      public Rest::Handler,
                      public OrderEntry::Handler,
                      public DropCopy::Handler,
                      public MarketData::Handler {
 public:
  Gateway(server::Dispatcher &, const Config &);

 protected:
  void operator()(const Event<Start> &) override;
  void operator()(const Event<Stop> &) override;
  void operator()(const Event<Timer> &) override;
  void operator()(const Event<Connected> &) override;
  void operator()(const Event<Disconnected> &) override;

  uint16_t operator()(const Event<CreateOrder> &, const std::string_view &request_id) override;
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id) override;
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id) override;

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id) override;

  void operator()(metrics::Writer &) override;

  // many

  void operator()(server::Trace<StreamStatus> const &) override;
  void operator()(server::Trace<ExternalLatency> const &) override;
  void operator()(server::Trace<ReferenceData> const &, bool is_last) override;
  void operator()(server::Trace<MarketStatus> const &, bool is_last) override;
  void operator()(server::Trace<TopOfBook> const &, bool is_last) override;
  void operator()(server::Trace<MarketByPriceUpdate> const &, bool is_last, bool refresh) override;
  void operator()(server::Trace<TradeSummary> const &, bool is_last) override;
  void operator()(server::Trace<StatisticsUpdate> const &, bool is_last) override;
  void operator()(server::Trace<FundsUpdate> const &, bool is_last) override;

  void operator()(Rest::PublicToken const &) override;
  void operator()(Rest::SymbolsUpdate &) override;

  void operator()(MarketData::RequestL2Snapshot const &) override;

  void operator()(OrderEntry::PrivateToken const &) override;

  // utilities

  OrderEntry &get_order_entry(const std::string_view &account);

 private:
  server::Dispatcher &dispatcher_;
  // config
  const std::string master_account_;
  // security
  absl::flat_hash_map<std::string, std::unique_ptr<Security>> security_;
  // io
  core::io::Context context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Rest rest_;
  absl::flat_hash_map<std::string, std::unique_ptr<OrderEntry>> order_entry_;
  absl::flat_hash_map<std::string, std::unique_ptr<DropCopy>> drop_copy_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
  // websocket uri's
  std::string public_ws_uri_;
  std::chrono::nanoseconds public_ws_ping_frequency_;
  std::string private_ws_uri_;  // XXX HANS this is BY ACCOUNT !!!
};

}  // namespace kucoin_futures
}  // namespace roq
