/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kucoin_futures/gateway.hpp"

#include "roq/logging.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === HELPERS ===

namespace {
template <typename R>
R create_accounts(auto const &config) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[_, account] : config.accounts) {
    result.try_emplace(static_cast<std::string_view>(account.name), std::make_unique<Account>(config, account.name));
  }
  return result;
}

template <typename R>
R create_order_entry_rest(auto &gateway, auto &context, auto &stream_id, auto &account_by_account, Shared &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[name, account] : account_by_account) {
    result.try_emplace(static_cast<std::string_view>(name), std::make_unique<OrderEntryREST>(gateway, context, ++stream_id, *account, shared));
  }
  return result;
}

template <typename R>
R create_order_entry_ws(auto &gateway, auto &context, auto &stream_id, auto &account_by_account, Shared &shared) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  if (shared.settings.ws_api && !std::empty(shared.settings.ws.uri)) {
    for (auto &[name, account] : account_by_account) {
      result.try_emplace(static_cast<std::string_view>(name), std::make_unique<OrderEntryWS>(gateway, context, ++stream_id, *account, shared));
    }
  }
  return result;
}

template <typename R>
R create_drop_copy(auto &account_by_account) {
  using result_type = std::remove_cvref_t<R>;
  result_type result;
  for (auto &[name, account] : account_by_account) {
    result.try_emplace(static_cast<std::string_view>(name), nullptr);
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Gateway::Gateway(server::Dispatcher &dispatcher, Settings const &settings, Config const &config, io::Context &context)
    : dispatcher_{dispatcher}, accounts_{create_accounts<decltype(accounts_)>(config)}, context_{context}, shared_{dispatcher, settings},
      rest_{*this, context_, ++stream_id_, shared_},
      order_entry_rest_{create_order_entry_rest<decltype(order_entry_rest_)>(*this, context_, stream_id_, accounts_, shared_)},
      order_entry_ws_{create_order_entry_ws<decltype(order_entry_ws_)>(*this, context_, stream_id_, accounts_, shared_)},
      drop_copy_{create_drop_copy<decltype(drop_copy_)>(accounts_)} {
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

void Gateway::operator()(Event<Control> const &event) {
  auto &[message_info, control] = event;
  switch (control.action) {
    using enum Action;
    case UNDEFINED:
      assert(false);
      break;
    case ENABLE:
      dispatcher_(State::ENABLED);
      break;
    case DISABLE:
      dispatcher_(State::DISABLED);
      break;
  }
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

void Gateway::operator()(Trace<TradeUpdate> const &event, bool is_last, uint8_t user_id, std::string_view const &request_id) {
  dispatcher_(event, is_last, user_id, request_id);
}

void Gateway::operator()(Trace<FundsUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Trace<PositionUpdate> const &event, bool is_last) {
  dispatcher_(event, is_last);
}

void Gateway::operator()(Rest::PublicToken const &public_token) {
  log::debug(R"(uri="{}", query="{}", ping_frequency={})"sv, public_token.uri, public_token.query, public_token.ping_frequency);
  public_ws_uri_ = public_token.uri;
  public_ws_query_ = public_token.query;
  public_ws_ping_frequency_ = public_token.ping_frequency;
  // note! could create first MarketData here, but this message will always arrive first
}

void Gateway::operator()(Rest::SymbolsUpdate &symbols_update) {
  auto [size, start_from] = shared_.symbols(symbols_update.symbols);
  ensure_symbol_slices(size);
  for (auto &iter : market_data_) {
    (*iter).subscribe(start_from);
  }
}

void Gateway::ensure_symbol_slices(size_t size) {
  while (std::size(market_data_) < size) {
    auto stream_id = ++stream_id_;
    auto index = std::size(market_data_);
    log::debug("Create MarketData (stream_id={}, index={})"sv, stream_id, index);
    auto market_data = std::make_unique<MarketData>(*this, context_, stream_id, shared_, index, public_ws_uri_, public_ws_query_, public_ws_ping_frequency_);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*market_data, message_info, start);
    market_data_.emplace_back(std::move(market_data));
  }
}

void Gateway::operator()(PrivateToken const &private_token) {
  log::debug(R"(uri="{}", query="{}", ping_frequency={})"sv, private_token.uri, private_token.query, private_token.ping_frequency);
  auto account = private_token.account;
  auto &drop_copy = drop_copy_[account];
  if (!drop_copy) {
    auto tmp = std::make_unique<DropCopy>(
        *this, context_, ++stream_id_, *accounts_.at(account), shared_, private_token.uri, private_token.query, private_token.ping_frequency);
    MessageInfo message_info;
    Start start;
    create_event_and_dispatch(*tmp, message_info, start);
    drop_copy = std::move(tmp);
  } else {
    (*drop_copy)(private_token);
  }
}

void Gateway::operator()(Event<Subscribe> const &event) {
  auto &[message_info, subscribe] = event;
  std::vector<Symbol> symbols;
  for (auto &item : subscribe.symbols) {
    if (shared_.all_symbols.emplace(item).second) {
      symbols.emplace_back(item);
    } else {
      log::warn(R"(*** DUPLICATE SUBSCRIPTION *** (symbol="{}")"sv, item);
    }
  }
  auto symbols_update = Rest::SymbolsUpdate{
      .symbols = symbols,
  };
  (*this)(symbols_update);
}

uint16_t Gateway::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry(event.value.account)(event, order, ref_data, request_id);
}

uint16_t Gateway::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Gateway::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  assert(!std::empty(event.value.account));
  assert(event.value.account == order.account);
  return get_order_entry(event.value.account)(event, order, ref_data, request_id, previous_request_id);
}

uint16_t Gateway::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  assert(!std::empty(event.value.account));
  return get_order_entry_rest(event.value.account)(event, request_id);  // note! REST-only
}

uint16_t Gateway::operator()(Event<MassQuote> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

uint16_t Gateway::operator()(Event<CancelQuotes> const &) {
  throw server::oms::NotSupported{"not supported"sv};
}

void Gateway::operator()(metrics::Writer &writer) const {
  dispatch_helper(*this, writer);
}

template <typename... Args>
void Gateway::dispatch(Args &&...args) {
  dispatch_helper(*this, std::forward<Args>(args)...);
}

template <typename... Args>
void Gateway::dispatch_helper(auto &self, Args &&...args) {
  auto helper = [&](auto &target) { target(std::forward<Args>(args)...); };
  helper(self.rest_);
  for (auto &[_, item] : self.order_entry_rest_) {
    helper(*item);
  }
  for (auto &[_, item] : self.order_entry_ws_) {
    helper(*item);
  }
  for (auto &[_, item] : self.drop_copy_) {
    if (static_cast<bool>(item)) {
      helper(*item);
    }
  }
  for (auto &item : self.market_data_) {
    helper(*item);
  }
}

OrderEntry &Gateway::get_order_entry_rest(std::string_view const &account) {
  auto iter = order_entry_rest_.find(account);
  if (iter != std::end(order_entry_rest_)) {
    return *(*iter).second;
  }
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

OrderEntry &Gateway::get_order_entry_ws(std::string_view const &account) {
  auto iter = order_entry_ws_.find(account);
  if (iter != std::end(order_entry_ws_)) {
    return *(*iter).second;
  }
  throw RuntimeError{R"(Unknown account="{}")"sv, account};
}

OrderEntry &Gateway::get_order_entry(std::string_view const &account) {
  if (shared_.settings.ws_api) {
    return get_order_entry_ws(account);
  }
  return get_order_entry_rest(account);
}

}  // namespace kucoin_futures
}  // namespace roq
