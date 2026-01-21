/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kucoin_futures/json/ws_parser.hpp"

namespace roq {
namespace kucoin_futures {

template <typename T>
struct WSParserTester final : public json::WSParser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    WSParserTester handler{callback};
    auto res = json::WSParser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit WSParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<json::WSAuth> const &event, [[maybe_unused]] std::string_view const &message) override { dispatch(event); }
  void operator()(Trace<json::WSWelcome> const &event) override { dispatch(event); }
  void operator()(Trace<json::WSError> const &event) override { dispatch(event); }
  void operator()(Trace<json::WSPong> const &event) override { dispatch(event); }
  void operator()(Trace<json::WSAddOrderAck> const &event) override { dispatch(event); }
  void operator()(Trace<json::WSCancelOrderAck> const &event) override { dispatch(event); }

  template <typename U>
  void dispatch(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace kucoin_futures
}  // namespace roq
