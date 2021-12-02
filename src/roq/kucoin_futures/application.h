/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include "roq/service.h"

namespace roq {
namespace kucoin_futures {

class Application final : public roq::Service {
 public:
  using roq::Service::Service;

 protected:
  int main(int, char **) override;
};

}  // namespace kucoin_futures
}  // namespace roq
