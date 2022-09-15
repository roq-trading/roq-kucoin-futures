/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/gateway.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "roq/logging.hpp"

#include "roq/core/charconv.hpp"
#include "roq/core/clock.hpp"
#include "roq/core/utils.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

namespace {
template <typename R>
auto create_security(Config const &config) {
  R result;
  for (auto &[_, iter] : config.accounts)
    result.try_emplace(iter.name, std::make_unique<Security>(config, iter.name));
  return result;
}

template <typename R, typename T>
auto create_order_entry(Gateway &gateway, io::Context &context, uint16_t &stream_id, T &security, Shared &shared) {
  R result;
  for (auto &iter : security)
    result.try_emplace(iter.first, std::make_unique<OrderEntry>(gateway, context, ++stream_id, *iter.second, shared));
  return result;
}

template <typename R, typename T>
auto create_drop_copy(T &security) {
  R result;
  for (auto &iter : security)
    result.try_emplace(iter.first, nullptr);
  return result;
}
}  // namespace

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &config)
    : dispatcher_(dispatcher), master_account_(config.get_master_account()),
      security_(create_security<decltype(security_)>(config)), context_(io::engine::ContextFactory::create_libevent()),
      shared_(dispatcher), rest_(*this, *context_, ++stream_id_, shared_),
      order_entry_(create_order_entry<decltype(order_entry_)>(*this, *context_, stream_id_, security_, shared_)),
      drop_copy_(create_drop_copy<decltype(drop_copy_)>(security_)) {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting the gateway..."sv);
  rest_(event);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    if (static_cast<bool>(drop_copy))
      (*drop_copy)(event);
  assert(std::empty(market_data_));
  // order_entry_.download.begin();
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping the gateway..."sv);
  for (auto &iter : market_data_)
    (*iter)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    if (static_cast<bool>(drop_copy))
      (*drop_copy)(event);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  rest_(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  rest_(event);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(event);
  for (auto &[_, drop_copy] : drop_copy_)
    if (static_cast<bool>(drop_copy))
      (*drop_copy)(event);
  for (auto &iter : market_data_)
    (*iter)(event);
  (*context_).drain();
}

void Gateway::operator()(Event<Connected> const &) {
}

void Gateway::operator()(Event<Disconnected> const &event) {
  auto const &[message_info, disconnected] = event;
  if (disconnected.order_cancel_policy != OrderCancelPolicy{}) {
    log::warn("** CANCEL-ON-DISCONNECT *NOT* SUPPORTED ***"sv);
  }
}

void Gateway::operator()(Trace<StreamStatus> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ExternalLatency> const &event) {
  dispatcher_(event);
}

void Gateway::operator()(Trace<ReferenceData> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketStatus> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<TopOfBook> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last, bool refresh) {
  dispatcher_(
      event, is_last, refresh, shared_.final_bids, shared_.final_asks, []([[maybe_unused]] auto &market_by_price) {});
}

void Gateway::operator()(Trace<TradeSummary> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<StatisticsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<FundsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Rest::PublicToken const &public_token) {
  log::debug(
      R"(uri="{}", query="{}", ping_frequency={})"sv,
      public_token.uri,
      public_token.query,
      public_token.ping_frequency);
  public_ws_uri_ = public_token.uri;
  public_ws_query_ = public_token.query;
  public_ws_ping_frequency_ = public_token.ping_frequency;
  // note! could create first MarketData here, but this message will always arrive first
}

void Gateway::operator()(Rest::SymbolsUpdate &symbols_update) {
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &iter : market_data_)
    (*iter).subscribe(start_from);
}

void Gateway::ensure_symbol_slices(size_t size) {
  while (std::size(market_data_) < size) {
    auto stream_id = ++stream_id_;
    auto index = std::size(market_data_);
    log::debug("Create MarketData (stream_id={}, index={})"sv, stream_id, index);
    auto market_data = std::make_unique<MarketData>(
        *this, *context_, stream_id, shared_, index, public_ws_uri_, public_ws_query_, public_ws_ping_frequency_);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    market_data_.emplace_back(std::move(market_data));
  }
}

void Gateway::operator()(OrderEntry::PrivateToken const &private_token) {
  log::debug(
      R"(uri="{}", query="{}", ping_frequency={})"sv,
      private_token.uri,
      private_token.query,
      private_token.ping_frequency);
  auto account = private_token.account;
  auto &drop_copy = drop_copy_[account];
  if (!drop_copy) {
    auto tmp = std::make_unique<DropCopy>(
        *this,
        *context_,
        ++stream_id_,
        *security_[account],
        shared_,
        private_token.uri,
        private_token.query,
        private_token.ping_frequency);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*tmp, message_info, start);
    drop_copy = std::move(tmp);
  }
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, order, request_id);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, request_id, previous_request_id);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, request_id);
}

void Gateway::operator()(metrics::Writer &writer) {
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(writer);
  for (auto &[_, drop_copy] : drop_copy_)
    if (static_cast<bool>(drop_copy))
      (*drop_copy)(writer);
  for (auto &iter : market_data_)
    (*iter)(writer);
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_))
    return *(*iter).second;
  throw RuntimeError(R"(Unknown account="{}")"sv, account);
}

}  // namespace kucoin_futures
}  // namespace roq
