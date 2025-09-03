/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kucoin_futures/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/kucoin_futures/json/map.hpp"
#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
    SupportType::POSITION,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context, auto &uri, auto &query) {
  io::web::URI uri_{uri};
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri_, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = query,
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() -> std::string { return {}; });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(
    Handler &handler,
    io::Context &context,
    uint16_t stream_id,
    Account &account,
    Shared &shared,
    std::string_view const &uri,
    std::string_view const &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context, uri, query)},
      ping_frequency_{ping_frequency}, decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .welcome = create_metrics(shared.settings, name_, "welcome"sv),
          .error = create_metrics(shared.settings, name_, "error"sv),
          .pong = create_metrics(shared.settings, name_, "pong"sv),
          .ack = create_metrics(shared.settings, name_, "ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, download_{{}, [this](auto state) { return download(state); }} {
}

bool DropCopy::ready() const {
  return (*connection_).ready();
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if ((*connection_).ready()) {
    if (welcome_) {
      if (next_ping_ < now) {
        send_ping(now);
      }
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."sv);
    (*connection_).close();
  }
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.welcome, metrics::Type::PROFILE)
      .write(profile_.error, metrics::Type::PROFILE)
      .write(profile_.pong, metrics::Type::PROFILE)
      .write(profile_.ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
  assert(logon_timeout_.count() == 0);
  auto now = clock::get_system();
  logon_timeout_ = now + shared_.settings.ws.request_timeout;
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  // note! wait for welcome
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case SUBSCRIBE:
      subscribe();
      return 0;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return 0;
  }
  assert(false);
  return 0;
}

void DropCopy::subscribe() {
  subscribe("/contractAccount/wallet"sv);
  subscribe("/contractMarket/tradeOrders"sv);
  subscribe("/contract/position"sv);  // XXX HANS maybe need symbol
}

void DropCopy::subscribe(std::string_view const &topic) {
  auto now = clock::get_system();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"sv,
      now.count(),
      topic);
  (*connection_).send_text(message);
}

void DropCopy::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("type":"ping")"
      R"(}})"sv,
      now.count());
  (*connection_).send_text(message);
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("welcome={}"sv, welcome);
    welcome_ = true;
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

// error={code=404, type=ERROR, data="topic /contract/position is not found", id="5750981774747"}
void DropCopy::operator()(Trace<json::Error> const &event) {
  profile_.error([&]() {
    // XXX HANS DEBUG
    auto &[trace_info, error] = event;
    log::error("error={}"sv, error);
  });
}

void DropCopy::operator()(Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("pong={}"sv, pong);
  });
}

void DropCopy::operator()(Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("ack={}"sv, ack);
  });
}

void DropCopy::operator()(Trace<json::TickerV2> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Match> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Execution> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::MarkIndexPrice> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::FundingRate> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Level2> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::FundingBegin> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::FundingEnd> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Snapshot24h> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::WalletBalanceChange> const &event) {
  auto &[trace_info, wallet_balance_change] = event;
  log::info<2>("wallet_balance_change={}"sv, wallet_balance_change);
  auto &data = wallet_balance_change.data;
  auto funds_update = FundsUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .currency = data.currency,
      .margin_mode = {},
      .balance = data.available_balance,
      .hold = data.hold_balance,
      .borrowed = NaN,
      .external_account = {},
      .update_type = UpdateType::INCREMENTAL,
      .exchange_time_utc = data.timestamp,  // ???
      .exchange_sequence = data.version,
      .sending_time_utc = {},
  };
  create_trace_and_dispatch(handler_, trace_info, funds_update, true);
}

void DropCopy::operator()(Trace<json::OrderMarginChange> const &event) {
  auto &[trace_info, order_margin_change] = event;
  log::info<2>("order_margin_change={}"sv, order_margin_change);
}

void DropCopy::operator()(Trace<json::AvailableBalanceChange> const &event) {
  auto &[trace_info, available_balance_change] = event;
  log::info<2>("available_balance_change={}"sv, available_balance_change);
}

void DropCopy::operator()(Trace<json::WithdrawHoldChange> const &event) {
  auto &[trace_info, withdraw_hold_change] = event;
  log::info<2>("withdraw_hold_change={}"sv, withdraw_hold_change);
}

void DropCopy::operator()(Trace<json::PositionChange> const &event) {
  auto &[trace_info, position_change] = event;
  log::info<2>("position_change={}"sv, position_change);
  auto &data = position_change.data;
  auto position_update = PositionUpdate{
      .stream_id = stream_id_,
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = data.symbol,
      .margin_mode = map(data.margin_mode),
      .external_account = {},
      .long_quantity = std::max(data.current_qty, 0.0),    // XXX FIXME TODO qty ???
      .short_quantity = std::max(-data.current_qty, 0.0),  // XXX FIXME TODO qty ???
      .update_type = UpdateType::INCREMENTAL,
      .exchange_time_utc = data.ts,  // ???
      .exchange_sequence = {},
      .sending_time_utc = data.current_timestamp,
  };
  create_trace_and_dispatch(handler_, trace_info, position_update, true);
}

void DropCopy::operator()(Trace<json::PositionSettlement> const &event) {
  auto &[trace_info, position_settlement] = event;
  log::info<2>("position_settlement={}"sv, position_settlement);
}

void DropCopy::operator()(Trace<json::PositionAdjustRiskLimit> const &event) {
  auto &[trace_info, positoin_adjust_risk_limit] = event;
  log::info<2>("positoin_adjust_risk_limit={}"sv, positoin_adjust_risk_limit);
}

void DropCopy::operator()(Trace<json::SymbolOrderChange> const &event) {
  auto &[trace_info, symbol_order_change] = event;
  log::info<2>("symbol_order_change={}"sv, symbol_order_change);
}

void DropCopy::operator()(Trace<json::OrderChange> const &event) {
  auto &[trace_info, order_change] = event;
  log::info<2>("order_change={}"sv, order_change);
}

}  // namespace kucoin_futures
}  // namespace roq
