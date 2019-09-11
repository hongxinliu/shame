/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace shame {

/**
 * @brief get current timestamp in microseconds
 */
inline uint64_t now() {
  return (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
}

class Clock {
 public:
  /**
   * @brief record current timestamp
   */
  void tick() {
    start_ = now();
  }

  /**
   * @brief get duration in microseconds from last tick
   */
  uint64_t tock() {
    return now() - start_;
  }

 protected:
  uint64_t start_;
};

class ThreadSafeClock {
 public:
  /**
   * @brief record current timestamp with a name
   */
  void tick(const std::string &name = "DEFAULT") {
    std::lock_guard<std::mutex> lock(mutex_);
    start_[name] = now();
  }

  /**
   * @brief get duration in microseconds for specific name
   */
  uint64_t tock(const std::string &name = "DEFAULT") {
    // get timestamp before locking
    auto t = now();
    std::lock_guard<std::mutex> lock(mutex_);
    return (start_.find(name) == start_.end() ? 0 : t - start_[name]);
  }

 protected:
  std::unordered_map<std::string, uint64_t> start_;
  std::mutex mutex_;
};

}  // namespace shame
