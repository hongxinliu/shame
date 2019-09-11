/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include "shame/subscription.h"
#include <google/protobuf/message_lite.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

namespace shame {

template <typename T>
class ThreadSafeQueue;
class Udpm;
class Shm;

class Shame {
 public:
  /**
   * @brief constructor of Shame, throws on fail
   * @param multicast_addr IP address of UDPM multicast
   * @param multicast_port port number of UDPM multicast
   * @param ttl ttl of UDP message
   * @param name_shm name of managed shared memory, empty string accepted to disable shared memory
   */
  Shame(const std::string &multicast_addr = "239.255.67.76",
        const uint16_t multicast_port = 6776,
        const int ttl = 0,
        const std::string &name_shm = "Shame");

 public:
  /**
   * @brief start message handling
   */
  void startHandling();

  /**
   * @brief stop message handling
   */
  void stopHandling();

  /**
   * @brief publish raw data
   * @param channel channel name
   * @param data pointer to data to be published
   * @param size length of data in bytes to be published
   * @param shared_memory whether shared memory used
   * @return bytes published
   */
  size_t publish(const std::string &channel, const void *data, const size_t size, const bool shared_memory);

  /**
   * @brief publish string
   * @param channel channel name
   * @param data string to be published
   * @param shared_memory whether shared memory used
   * @return bytes published
   */
  size_t publish(const std::string &channel, const std::string &data, const bool shared_memory);

  /**
   * @brief publish protobuf message
   * @param channel channel name
   * @param msg protobuf message
   * @param shared_memory whether shared memory used
   * @return bytes published
   */
  size_t publish(const std::string &channel, const google::protobuf::MessageLite &msg, const bool shared_memory);

  /**
   * @brief subscribe as raw data
   * @param channel channel name
   * @param callback_msg_udpm callback function on udpm message
   * @param callback_msg_shm callback function on shm message
   * @return handle of this subscription
   */
  Subscription *subscribe(const std::string &channel,
                          const std::function<void(const std::string &channel,
                                                   std::shared_ptr<uint8_t>,
                                                   size_t)> &callback_msg_udpm,
                          const std::function<void(const std::string &channel,
                                                   ShameData *)> &callback_msg_shm);

  /**
   * @brief subscribe as protobuf message
   * @param channel channel name
   * @param callback_msg callback function on message
   * @return handle of this subscription
   */
  template <typename ProtoType,
            typename std::enable_if<std::is_base_of<google::protobuf::MessageLite, ProtoType>::value>::type * = nullptr>
  Subscription *subscribe(const std::string &channel,
                          const std::function<void(const std::string &,
                                                   std::shared_ptr<ProtoType>,
                                                   bool)> &callback_msg) {
    auto subscription = std::make_shared<ProtobufSubscription<ProtoType>>(channel, callback_msg);
    subscriptions_[channel].push_back(subscription);
    return subscription.get();
  }

  /**
   * @brief unsubscribe message
   * @param subscription handle of subscription returned by subscribe
   * @return true on success
   */
  bool unsubscribe(Subscription *subscription);

 protected:
  /**
   * @brief callback function from udpm
   */
  void callbackReceive(const std::string &channel, std::shared_ptr<uint8_t> data, size_t size, bool shared_memory);

  /**
   * @brief inner thread to dispatch messages
   */
  void threadDispatch();

 protected:
  std::shared_ptr<Udpm> udpm_;
  std::shared_ptr<Shm> shm_;
  std::unordered_map<std::string, std::list<std::shared_ptr<Subscription>>> subscriptions_;
  std::shared_ptr<ThreadSafeQueue<std::tuple<std::string, std::shared_ptr<uint8_t>, size_t, bool>>> msg_queue_;
  std::shared_ptr<std::thread> handle_thread_dispatch_;
  std::atomic<bool> enable_thread_dispatch_;
};

}  // namespace shame
