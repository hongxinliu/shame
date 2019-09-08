/*
 * Copyright (C) 2019 Hongxin Liu. All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include "shame/shame.h"
#include "shame/examples/proto/raw.pb.h"
#include "shame/common/time.h"
#include <thread>
#include <iostream>

int main(int argc, char **argv) {
  shame::Shame shame;

  const std::string channel("Shame");
  const bool shared_memory = true;
  int count = 0;

  shame::examples::Raw raw;
  raw.set_content(std::string(1024*1024, '+'));
  std::cout << raw.descriptor()->full_name() << std::endl;

  while(true) {
    raw.set_timestamp(shame::now());
    shame.publish(channel, raw, shared_memory);

    std::cout << "[" << ++count << "]"
              << " Published proto with " << raw.content().size() << " bytes"
              << " on channel " << channel
              << " via " << (shared_memory ? "shared memory" : "udpm")
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}