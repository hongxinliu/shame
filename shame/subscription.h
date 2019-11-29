/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <google/protobuf/message_lite.h>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "shame/shame_data.h"

namespace shame {

class Subscription {
 public:
  /**
   * @brief constructor of subscription
   * @param channel channel name (regex supported)
   */
  explicit Subscription(const std::string &channel) : channel_(channel) {}

  virtual ~Subscription() {}

 public:
  /**
   * @brief get channel name
   */
  std::string channel() const { return channel_; }

  /**
   * callback function from lower level on udpm message
   */
  virtual void callbackReceiveUdpm(const std::string &channel, const std::shared_ptr<uint8_t> &data,
                                   const size_t size) = 0;

  /**
   * callback function from lower level on shm message
   */
  virtual void callbackReceiveShm(const std::string &channel, ShameData *shame_data) = 0;

 protected:
  std::string channel_;
};

class RawSubscription : public Subscription {
 public:
  /**
   * @brief constructor of raw subscription
   * @param channel channel name to subscribe
   * @param callback_msg_udpm callback function on udpm message
   * @param callback_msg_shm callback function on shm message
   */
  RawSubscription(const std::string &channel,
                  const std::function<void(const std::string &, const std::shared_ptr<uint8_t>&, const size_t)>
                      &callback_msg_udpm,
                  const std::function<void(const std::string &, ShameData *)> &callback_msg_shm)
      : Subscription(channel),
        callback_msg_udpm_(callback_msg_udpm),
        callback_msg_shm_(callback_msg_shm) {}

 public:
  void callbackReceiveUdpm(const std::string &channel, const std::shared_ptr<uint8_t> &data,
                           const size_t size) override {
    callback_msg_udpm_(channel, data, size);
  };

  void callbackReceiveShm(const std::string &channel, ShameData *shame_data) override {
    callback_msg_shm_(channel, shame_data);
  }

 protected:
  const std::function<void(const std::string &, const std::shared_ptr<uint8_t>&, const size_t)>
      callback_msg_udpm_;
  const std::function<void(const std::string &, ShameData *)> callback_msg_shm_;
};

template <typename ProtoType,
          typename std::enable_if<
              std::is_base_of<google::protobuf::MessageLite, ProtoType>::value>::type * = nullptr>
class ProtobufSubscription : public Subscription {
 public:
  /**
   * @brief constructor of protobuf subscription
   * @param channel channel name to subscribe
   * @param callback_msg callback function on message
   */
  ProtobufSubscription(const std::string &channel,
                       const std::function<void(const std::string &, const std::shared_ptr<ProtoType>&,
                                                const bool)> &callback_msg)
      : Subscription(channel), callback_msg_(callback_msg) {}

 public:
  void callbackReceiveUdpm(const std::string &channel, const std::shared_ptr<uint8_t> &data,
                           const size_t size) override {
    auto msg = std::make_shared<ProtoType>();
    if (msg->ParseFromArray(data.get(), size)) {
      callback_msg_(channel, msg, false);
    } else {
      std::cout << "Failed to parse data to type " << msg->descriptor()->full_name() << std::endl;
    }
  };

  void callbackReceiveShm(const std::string &channel, ShameData *shame_data) override {
    auto msg = std::make_shared<ProtoType>();
    shame_data->mutex_.lock_sharable();
    auto ret = msg->ParseFromArray(shame_data->data_.data(), shame_data->data_.size());
    shame_data->mutex_.unlock_sharable();
    if (ret) {
      callback_msg_(channel, msg, true);
    } else {
      std::cout << "Failed to parse data to type " << msg->descriptor()->full_name() << std::endl;
    }
  }

 protected:
  const std::function<void(const std::string &, const std::shared_ptr<ProtoType>&, const bool)> callback_msg_;
};

}  // namespace shame
