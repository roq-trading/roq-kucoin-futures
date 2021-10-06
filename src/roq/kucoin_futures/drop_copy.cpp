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
    std::chrono::nanoseconds ping_frequency)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          core::URI{uri},
          {},
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
  connection_.refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::Socket::Connected &) {
}

void DropCopy::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void DropCopy::operator()(const core::web::Socket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
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

void DropCopy::operator()(server::Trace<json::Welcome> const &) {
}

void DropCopy::operator()(server::Trace<json::Error> const &) {
}

void DropCopy::operator()(server::Trace<json::Pong> const &) {
}

void DropCopy::operator()(server::Trace<json::Ack> const &) {
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
