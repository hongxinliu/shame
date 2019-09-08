/*
 * Copyright (C) 2019 Hongxin Liu. All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace shame {

template <typename T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

 public:
  void enqueue(const T &element) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(element);
    cv_.notify_one();
  }

  bool dequeue(T *element) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    *element = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  bool waitDequeue(T *element) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]() { return break_all_wait_.load() || !queue_.empty(); });
    if (break_all_wait_.load()) {
      return false;
    }
    *element = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  typename std::queue<T>::size_type size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
      queue_.pop();
    }
  }

  void breakAllWait() {
    break_all_wait_.store(true);
    cv_.notify_all();
  }

  void reset() { break_all_wait_.store(false); }

 protected:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> break_all_wait_{false};
};

}  // namespace shame
