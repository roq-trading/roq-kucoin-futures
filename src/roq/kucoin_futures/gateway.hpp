/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "roq/server.hpp"

#include "roq/io/context.hpp"

#include "roq/kucoin_futures/account.hpp"
#include "roq/kucoin_futures/config.hpp"
#include "roq/kucoin_futures/drop_copy.hpp"
#include "roq/kucoin_futures/market_data.hpp"
#include "roq/kucoin_futures/order_entry.hpp"
#include "roq/kucoin_futures/rest.hpp"
#include "roq/kucoin_futures/shared.hpp"

namespace roq {
namespace kucoin_futures {

struct Gateway final : public server::Handler,
                       public Rest::Handler,
                       public OrderEntry::Handler,
                       public DropCopy::Handler,
                       public MarketData::Handler {
  Gateway(server::Dispatcher &, Config const &, io::Context &);

 protected:
  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

  // many

  void operator()(Trace<StreamStatus> const &) override;
  void operator()(Trace<ExternalLatency> const &) override;
  void operator()(Trace<ReferenceData> const &, bool is_last) override;
  void operator()(Trace<MarketStatus> const &, bool is_last) override;
  void operator()(Trace<TopOfBook> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate> const &, bool is_last) override;
  void operator()(Trace<TradeSummary> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate> const &, bool is_last) override;
  void operator()(Trace<FundsUpdate> const &, bool is_last) override;

  void operator()(Rest::PublicToken const &) override;
  void operator()(Rest::SymbolsUpdate &) override;

  void ensure_symbol_slices(size_t size);

  void operator()(OrderEntry::PrivateToken const &) override;

  // utilities

  template <typename... Args>
  void dispatch(Args &&...);

  OrderEntry &get_order_entry(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // accounts
  absl::flat_hash_map<std::string, std::unique_ptr<Account>> const accounts_;
  // io
  io::Context &context_;
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
  std::string public_ws_query_;
  std::chrono::nanoseconds public_ws_ping_frequency_;
  std::string private_ws_uri_;  // XXX HANS this is BY ACCOUNT !!!
  // cache
  std::vector<MBPUpdate> bids_, asks_;
};

}  // namespace kucoin_futures
}  // namespace roq
