/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kucoin_futures/drop_copy.hpp"

#include "roq/utils/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/core/json/buffer.hpp"

#include "roq/kucoin_futures/flags.hpp"

#include "roq/kucoin_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kucoin_futures {

namespace {
const auto NAME = "ex"sv;
const auto SUPPORTS = utils::Mask{
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context, const auto &uri, const auto &query) {
  core::URI uri_{uri};
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri_, 1},
      .query = query,
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}
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
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context, uri, query)), ping_frequency_(ping_frequency),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .welcome = create_metrics(name_, "welcome"sv),
          .error = create_metrics(name_, "error"sv),
          .pong = create_metrics(name_, "pong"sv),
          .ack = create_metrics(name_, "ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
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
    log::warn("Did not receive the welcome message, disconnecting now..."sv);
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

void DropCopy::operator()(const core::web::ClientSocket::Connected &) {
  assert(logon_timeout_.count() == 0);
  auto now = core::clock::GetSystem();
  logon_timeout_ = now + Flags::ws_request_timeout();
}

void DropCopy::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(const core::web::ClientSocket::Ready &) {
  // note! wait for welcome
}

void DropCopy::operator()(const core::web::ClientSocket::Close &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
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
  subscribe("/contractAccount/wallet"sv);
  subscribe("/contractMarket/tradeOrders"sv);
  subscribe("/contract/position"sv);  // XXX HANS maybe need symbol
}

void DropCopy::subscribe(const std::string_view &topic) {
  auto now = core::clock::GetSystem();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"sv,
      now.count(),
      topic);
  log::debug("message={}"sv, message);
  connection_.send_text(message);
}

void DropCopy::send_ping(std::chrono::nanoseconds now) {
  assert(ping_frequency_.count() > 0);
  next_ping_ = now + ping_frequency_ / 2;
  auto message = fmt::format(R"({{"id":{},"type":"ping"}})"sv, now.count());
  log::debug<1>(R"(message="{}")"sv, message);
  connection_.send_text(message);
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(server::Trace<json::Welcome> const &event) {
  profile_.welcome([&]() {
    auto &[trace_info, welcome] = event;
    log::info<1>("event={{trace_info={}, welcome={}}}"sv, trace_info, welcome);
    welcome_ = true;
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  });
}

// error={code=404, type=ERROR, data="topic /contract/position is not found", id="5750981774747"}
void DropCopy::operator()(server::Trace<json::Error> const &event) {
  profile_.error([&]() {
    // XXX HANS DEBUG
    auto &[trace_info, error] = event;
    log::warn("error={}"sv, error);
    // log::fatal("event={{trace_info={}, error={}}}"sv, trace_info, error);
  });
}

void DropCopy::operator()(server::Trace<json::Pong> const &event) {
  profile_.pong([&]() {
    auto &[trace_info, pong] = event;
    log::info<4>("event={{trace_info={}, pong={}}}"sv, trace_info, pong);
  });
}

void DropCopy::operator()(server::Trace<json::Ack> const &event) {
  profile_.ack([&]() {
    auto &[trace_info, ack] = event;
    log::info<2>("event={{trace_info={}, ack={}}}"sv, trace_info, ack);
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
