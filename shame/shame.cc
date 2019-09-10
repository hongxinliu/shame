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
#include "shame/udpm/udpm.h"
#include "shame/shm/shm.h"
#include "shame/common/thread_safe_queue.h"
#include <regex>
#include <iostream>

namespace shame {

Shame::Shame(const std::string &multicast_addr,
             const uint16_t multicast_port,
             const int ttl,
             const std::string &name_shm) :
msg_queue_(new ThreadSafeQueue<std::tuple<std::string, std::shared_ptr<uint8_t>, size_t, bool>>()) {
  try {
    udpm_.reset(new Udpm(multicast_addr, multicast_port, ttl));
  } catch (std::exception &e) {
    std::cout << "Failed to construct UDPM, you may not connected to any network. " << std::endl
              << "Try the following commands to setup local loopback:" << std::endl
              << "sudo ifconfig lo multicast" << std::endl
              << "sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev lo" << std::endl;
    exit(1);
  }

  if (!name_shm.empty()) {
    try {
      shm_.reset(new Shm(name_shm));
    } catch (std::exception &e) {
      std::cout << "Failed to open shared memory object: " << name_shm << std::endl;
      exit(1);
    }
  }
}

void Shame::startHandling() {
  stopHandling();
  msg_queue_->clear();
  msg_queue_->reset();

  enable_thread_dispatch_.store(true);
  handle_thread_dispatch_.reset(new std::thread(&Shame::threadDispatch, this));

  udpm_->startAsyncReceiving(std::bind(&Shame::callbackReceive,
                                       this,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3,
                                       std::placeholders::_4));
}

void Shame::stopHandling() {
  udpm_->stopAsyncReceiving();

  enable_thread_dispatch_.store(false);
  msg_queue_->breakAllWait();
  if(handle_thread_dispatch_) {
    handle_thread_dispatch_->join();
    handle_thread_dispatch_.reset();
  }
}

size_t Shame::publish(const std::string &channel, const void *data, const size_t size, const bool shared_memory) {
  if (shared_memory) {
    if (!shm_) {
      std::cout << "This shame instance was not constructed with shared memory supported" << std::endl;
      return 0;
    }

    // TODO(Hongxin): generate random unique key from channel
    const std::string key(channel);

    // put data to shared memory
    size_t size_sent = 0;
    try {
      size_sent = shm_->put(key, data, size);
    } catch (std::exception &e) {
      std::cout << "Failed to put data to shared memory key: " << key << std::endl;
      return 0;
    }

    // send shared memory key via udpm
    if (udpm_->send(channel, key.data(), key.size(), true) != key.size()) {
      std::cout << "Sent unexpected length" << std::endl;
      return 0;
    }

    return size_sent;
  } else {
    return udpm_->send(channel, data, size, false);
  }
}

size_t Shame::publish(const std::string &channel, const std::string &data, const bool shared_memory) {
  return publish(channel, (const void*)data.data(), data.size(), shared_memory);
}

size_t Shame::publish(const std::string &channel,
                      const google::protobuf::MessageLite &msg,
                      const bool shared_memory) {
  if (shared_memory) {
    if (!shm_) {
      std::cout << "This shame instance was not constructed with shared memory supported" << std::endl;
      return 0;
    }

    // TODO(Hongxin): generate random unique key from channel
    const std::string key(channel);

    size_t size;
    try {
      size = shm_->put(key, msg);
    } catch (std::exception &e) {
      std::cout << "Failed to put data to shared memory key: " << key << std::endl;
      return 0;
    }

    // send shared memory key via udpm
    if (udpm_->send(channel, key.data(), key.size(), true) != key.size()) {
      std::cout << "Sent unexpected length" << std::endl;
      return 0;
    }

    return size;
  } else {
    std::string msg_str;
    msg.SerializeToString(&msg_str);
    return publish(channel, msg_str.data(), msg_str.size(), false);
  }
}

Subscription *Shame::subscribe(const std::string &channel,
                               const std::function<void(const std::string &channel,
                                                        std::shared_ptr<uint8_t>,
                                                        size_t)> &callback_udpm,
                               const std::function<void(const std::string &channel,
                                                        ShameData *)> &callback_shm) {
  auto subscription = new RawSubscription(channel, callback_udpm, callback_shm);
  subscriptions_[channel].push_back(subscription);
  return subscription;
}

bool Shame::unsubscribe(Subscription *subscription) {
  if (!subscription) {
    return false;
  }

  auto items = subscriptions_.find(subscription->channel());
  if (items != subscriptions_.end()) {
    for (auto it = items->second.begin(); it != items->second.end(); ++it) {
      if (*it == subscription) {
        items->second.erase(it);
        return true;
      }
    }
  }

  return false;
}

void Shame::callbackReceive(const std::string &channel,
                            std::shared_ptr<uint8_t> data,
                            size_t size,
                            bool shared_memory) {
  msg_queue_->enqueue(std::make_tuple(channel, data, size, shared_memory));
}

void Shame::threadDispatch() {
  while(enable_thread_dispatch_.load()) {
    std::tuple<std::string, std::shared_ptr<uint8_t>, size_t, bool> msg;
    if(!msg_queue_->waitDequeue(&msg)) {
      continue;
    }

    // TODO(Hongxin): parallel dispatch
    for (auto items : subscriptions_) {
      std::regex pattern(items.first);
      if (std::regex_match(std::get<0>(msg), pattern)) {
        if (std::get<3>(msg)) {
          const std::string key(reinterpret_cast<char*>(std::get<1>(msg).get()), std::get<2>(msg));
          auto shame_data = shm_->find(key);
          if (!shame_data) {
            std::cout << "Failed to get data from shared memory key: " << key << std::endl;
            return;
          }

          for (auto &item : items.second) {
            item->callbackReceiveShm(std::get<0>(msg), shame_data);
          }
        } else {
          for (auto &item : items.second) {
            item->callbackReceiveUdpm(std::get<0>(msg), std::get<1>(msg), std::get<2>(msg));
          }
        }
      }
    }
  }
}

}  // namespace shame
