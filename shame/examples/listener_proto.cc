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

void callbackReceive(const std::string &channel, std::shared_ptr<shame::examples::Raw> raw, bool shared_memory) {
  static int count = 0;
  std::cout << "[" << ++count << "]"
            << " Received proto with " << raw->content().size() << " bytes"
            << " on channel " << channel
            << " via " << (shared_memory ? "shared memory" : "udpm")
            << " after " << shame::now() - raw->timestamp() << " us"
            << std::endl;
}

int main(int argc, char **agrv) {
  shame::Shame shame;
  shame.subscribe<shame::examples::Raw>("Shame", callbackReceive);
  shame.startHandling();

  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
