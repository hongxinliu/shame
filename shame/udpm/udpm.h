/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include <unordered_map>
#include <thread>
#include <random>

namespace shame {

template <typename T>
class ThreadSafeQueue;
class Socket;

struct Header {
  uint32_t signature;
  uint32_t id;
  uint32_t len_payload;
  uint32_t num_packets;
  uint32_t offset;
};

struct MessageBuffer {
  Header header;
  std::string channel;
  uint32_t num_received;
  std::shared_ptr<uint8_t> payload;
};

class Udpm {
 public:
  /**
   * @brief constructor of Udpm
   * @param multicast_addr address of udpm multicast
   * @param multicast_port port number of udpm multicast
   * @param ttl time to live, messages would not leave localhost while ttl=0
   */
  Udpm(const std::string &multicast_addr,
       const uint16_t multicast_port,
       const int ttl);

  /**
   * @brief destructor
   */
  ~Udpm();

 public:
  /**
   * @brief start async receiving
   * @param callback_recv callback function on receiving
   */
  void startAsyncReceiving(const std::function<void(const std::string &,
                                                    std::shared_ptr<uint8_t>,
                                                    size_t,
                                                    bool)> &callback_recv);

  /**
   * @brief stop async receiving
   */
  void stopAsyncReceiving();

  /**
   * @brief send raw data
   * @param channel channel name
   * @param payload pointer to data to be sent
   * @param len_payload length of data in bytes to be sent
   * @param shared_memory whether use shared memory
   * @return bytes transfered
   */
  size_t send(const std::string &channel,
              const void *payload,
              const size_t len_payload,
              const bool shared_memory);

 protected:
  /**
   * @brief inner callback function on receiving
   */
  void callbackReceive(std::shared_ptr<uint8_t> data, const size_t size);

  /**
   * @brief inner thread to pack messages
   */
  void threadPack();

  /**
   * @brief inner function to send message with header, channel and payload
   */
  size_t send(const Header &header,
              const std::string &channel,
              const void *payload,
              const size_t len_payload);

 protected:
  const uint32_t signature_udpm_message_;
  const uint32_t signature_shm_message_;

  std::shared_ptr<Socket> socket_;
  std::shared_ptr<ThreadSafeQueue<std::pair<std::shared_ptr<uint8_t>, size_t>>> msg_queue_;
  std::unordered_map<uint32_t, MessageBuffer> msg_buffer_;

  std::function<void(const std::string &, std::shared_ptr<uint8_t>, size_t, bool)> callback_recv_;
  std::shared_ptr<std::thread> handle_thread_pack_;
  std::atomic<bool> enable_thread_pack_;

  std::default_random_engine e_;
  std::uniform_int_distribution<uint32_t> d_;
};

}  // namespace shame
