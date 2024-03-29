/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include <iostream>
#include <thread>
#include "shame/shame.h"

int main() {
  shame::Shame shame;

  const std::string channel("Shame");
  const std::string str(1024 * 1024, '+');  // 1MB
  const bool shared_memory = true;
  int count = 0;

  while (true) {
    auto size = shame.publish(channel, str, shared_memory);

    std::cout << "[" << ++count << "]"
              << " Published " << size << " bytes"
              << " on channel " << channel << " via " << (shared_memory ? "shared memory" : "udpm")
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
