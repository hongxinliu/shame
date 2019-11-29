/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include <iostream>
#include <thread>
#include "examples/proto/raw.pb.h"
#include "shame/common/time.h"
#include "shame/shame.h"

void callbackReceive(const std::string &channel, std::shared_ptr<shame::examples::Raw> raw,
                     bool shared_memory) {
  static int count = 0;
  std::cout << "[" << ++count << "]"
            << " Received proto with " << raw->content().size() << " bytes"
            << " on channel " << channel << " via " << (shared_memory ? "shared memory" : "udpm")
            << " after " << shame::now() - raw->timestamp() << " us" << std::endl;
}

int main() {
  shame::Shame shame;
  shame.subscribe<shame::examples::Raw>("Shame", callbackReceive);
  shame.startHandling();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
