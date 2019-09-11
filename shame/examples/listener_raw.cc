/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include "shame/shame.h"
#include <thread>
#include <iostream>

void callbackReceiveUdpm(const std::string &channel, std::shared_ptr<uint8_t> data, size_t size) {
  static int count = 0;
  std::cout << "[" << ++count << "]"
            << " Received " << size << " bytes"
            << " on channel " << channel
            << " via udpm"
            << std::endl;
}

void callbackReceiveShm(const std::string &channel, shame::ShameData *shame_data) {
  static int count = 0;
  shame_data->mutex_.lock_sharable();
  std::cout << "[" << ++count << "]"
            << " Received " << shame_data->data_.size() << " bytes"
            << " on channel " << channel
            << " via shared memory"
            << std::endl;
  shame_data->mutex_.unlock_sharable();
}

int main(int argc, char **agrv) {
  shame::Shame shame;
  shame.subscribe("Shame", callbackReceiveUdpm, callbackReceiveShm);
  shame.startHandling();

  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
