/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kucoin_futures/gateway.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "roq/logging.hpp"

#include "roq/clock.hpp"
#include "roq/core/charconv.hpp"
#include "roq/core/utils.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === HELPERS ===

namespace {
template <typename R>
auto create_authenticator(auto const &config) {
  R result;
  for (auto &[_, account] : config.accounts)
    result.try_emplace(account.name, std::make_unique<Authenticator>(config, account.name));
  return result;
}

template <typename R>
auto create_order_entry(auto &gateway, auto &context, auto &stream_id, auto &authenticator_by_account, Shared &shared) {
  R result;
  for (auto &[account, authenticator] : authenticator_by_account)
    result.try_emplace(account, std::make_unique<OrderEntry>(gateway, context, ++stream_id, *authenticator, shared));
  return result;
}

template <typename R>
auto create_drop_copy(auto &authenticator_by_account) {
  R result;
  for (auto &[account, authenticator] : authenticator_by_account)
    result.try_emplace(account, nullptr);
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, master_account_{config.get_master_account()},
      authenticator_{create_authenticator<decltype(authenticator_)>(config)}, context_{context}, shared_{dispatcher},
      rest_{*this, context_, ++stream_id_, shared_},
      order_entry_{create_order_entry<decltype(order_entry_)>(*this, context_, stream_id_, authenticator_, shared_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(authenticator_)} {
}

void Gateway::operator()(Event<Start> const &event) {
  log::info("Starting..."sv);
  assert(std::empty(market_data_));
  dispatch(event);
}

void Gateway::operator()(Event<Stop> const &event) {
  log::info("Stopping..."sv);
  dispatch(event);
}

void Gateway::operator()(Event<Timer> const &event) {
  dispatch(event);
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

void Gateway::operator()(Trace<MarketByPriceUpdate> const &event, bool is_last) {
  auto callback = []([[maybe_unused]] auto &market_by_price) {};
  dispatcher_(event, is_last, bids_, asks_, callback);
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
        *this, context_, stream_id, shared_, index, public_ws_uri_, public_ws_query_, public_ws_ping_frequency_);
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
        context_,
        ++stream_id_,
        *authenticator_[account],
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
  dispatch(writer);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  rest_(std::forward<Args>(args)...);
  for (auto &[_, order_entry] : order_entry_)
    (*order_entry)(std::forward<Args>(args)...);
  for (auto &[_, drop_copy] : drop_copy_)
    if (static_cast<bool>(drop_copy))
      (*drop_copy)(std::forward<Args>(args)...);
  for (auto &iter : market_data_)
    (*iter)(std::forward<Args>(args)...);
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  auto iter = order_entry_.find(account);
  if (iter != std::end(order_entry_))
    return *(*iter).second;
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

}  // namespace kucoin_futures
}  // namespace roq
