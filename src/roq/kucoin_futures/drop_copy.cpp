/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kucoin_futures/drop_copy.h"

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/core/json/buffer.h"

#include "roq/kucoin_futures/flags.h"

#include "roq/kucoin_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kucoin_futures {

namespace {
static const auto NAME = "ex"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::FUNDS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared,
    const std::string_view &uri,
    const std::string_view &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          core::URI{uri},
          query,
          Flags::ws_ping_freq(),
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          []() { return std::string(); }),
      ping_frequency_(ping_frequency), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .welcome = create_metrics(name_, "welcome"_sv),
          .error = create_metrics(name_, "error"_sv),
          .pong = create_metrics(name_, "pong"_sv),
          .ack = create_metrics(name_, "ack"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
      },
      security_(security), shared_(shared),
      download_({}, [this](auto state) { return download(state); }) {
}

bool DropCopy::ready() const {
  return connection_.ready();
}

void DropCopy::operator()(const Event<Start> &) {
  connection_.start();
}

void DropCopy::operator()(const Event<Stop> &) {
  connection_.stop();
}

void DropCopy::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
  if (connection_.ready()) {
    if (welcome_) {
      if (next_ping_ < now)
        send_ping(now);
    }
  } else if (logon_timeout_.count() && logon_timeout_ < now) {
    assert(!welcome_);
    log::warn("Did not receive the welcome message, disconnecting now..."_sv);
    connection_.close();
  }
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.welcome, metrics::PROFILE)
      .write(profile_.error, metrics::PROFILE)
      .write(profile_.pong, metrics::PROFILE)
      .write(profile_.ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::Socket::Connected &) {
  assert(logon_timeout_.count() == 0);
  auto now = core::get_system_clock();
  logon_timeout_ = now + Flags::ws_request_timeout();
}

void DropCopy::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(const core::web::Socket::Ready &) {
  // note! wait for welcome
}

void DropCopy::operator()(const core::web::Socket::Close &) {
}

void DropCopy::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::Socket::Binary &) {
  log::fatal("Unexpected"_sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    case DropCopyState::UNDEFINED:
      assert(false);
      break;
    case DropCopyState::SUBSCRIBE:
      subscribe();
      return {};
    case DropCopyState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::subscribe() {
  subscribe("/contractAccount/wallet"_sv);
  subscribe("/contractMarket/tradeOrders"_sv);
  subscribe("/contract/position"_sv);  // XXX HANS maybe need symbol
}

void DropCopy::subscribe(const std::string_view &topic) {
  auto now = core::get_system_clock();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"_sv,
      now.count(),
      topic);
  log::debug("message={}"_sv, message);
  connection_.send_text(message);
}

void DropCopy::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(R"({{"id":{},"type":"ping"}})"_sv, now.count());
  log::debug<1>(R"(message="{}")"_sv, message);
  connection_.send_text(message);
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      server::TraceInfo trace_info;
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"_sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(server::Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("event={{trace_info={}, welcome={}}}"_sv, trace_info, welcome);
    welcome_ = true;
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

void DropCopy::operator()(server::Trace<json::Error> const &event) {
  profile_.error([&]() {
    // XXX HANS DEBUG
    auto &[trace_info, error] = event;
    log::warn("error={}"_sv, error);
    // log::fatal("event={{trace_info={}, error={}}}"_sv, trace_info, error);
  });
}

void DropCopy::operator()(server::Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("event={{trace_info={}, pong={}}}"_sv, trace_info, pong);
  });
}

void DropCopy::operator()(server::Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("event={{trace_info={}, ack={}}}"_sv, trace_info, ack);
  });
}

void DropCopy::operator()(server::Trace<json::Ticker> const &) {
}

void DropCopy::operator()(server::Trace<json::TickerV2> const &) {
}

void DropCopy::operator()(server::Trace<json::Match> const &) {
}

void DropCopy::operator()(server::Trace<json::MarkIndexPrice> const &) {
}

void DropCopy::operator()(server::Trace<json::FundingRate> const &) {
}

void DropCopy::operator()(server::Trace<json::Level2> const &) {
}

void DropCopy::operator()(server::Trace<json::FundingBegin> const &) {
}

void DropCopy::operator()(server::Trace<json::FundingEnd> const &) {
}

void DropCopy::operator()(server::Trace<json::Snapshot24h> const &) {
}

void DropCopy::operator()(server::Trace<json::OrderChange> const &) {
}

void DropCopy::operator()(server::Trace<json::OrderMarginChange> const &) {
}

void DropCopy::operator()(server::Trace<json::AvailableBalanceChange> const &) {
}

void DropCopy::operator()(server::Trace<json::WithdrawHoldChange> const &) {
}

void DropCopy::operator()(server::Trace<json::PositionChange> const &) {
}

void DropCopy::operator()(server::Trace<json::PositionSettlement> const &) {
}

}  // namespace kucoin_futures
}  // namespace roq
