/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace shame {

class Socket {
 public:
  /**
   * @brief constructor of Socket
   * @param multicast_addr address of udpm multicast
   * @param multicast_port port number of udpm multicast
   * @param ttl time to live, messages would not leave localhost while ttl=0
   */
  Socket(const std::string &multicast_addr, const uint16_t multicast_port, const int ttl);

  /**
   * @brief destructor
   */
  ~Socket();

 public:
  /**
   * @brief send raw data
   * @param data pointer to data to be sent
   * @param size length in bytes to be sent
   * @return bytes transfered
   */
  size_t send(const void *data, const size_t size);

  /**
   * @brief send string
   * @param data string to be sent
   * @return bytes transfered
   */
  size_t send(const std::string &data);

  /**
   * @brief send boost const buffer
   * @param buffer boost const buffer
   * @return bytes transfered
   */
  size_t send(const boost::asio::const_buffers_1 &buffer);

  /**
   * @brief send boost mutable buffer
   * @param buffer boost mutable buffer
   * @return bytes transfered
   */
  size_t send(const boost::asio::mutable_buffers_1 &buffer);

  /**
   * @brief send boost const buffers
   * @param buffer boost const buffers
   * @return bytes transfered
   */
  size_t send(const std::vector<boost::asio::const_buffers_1> &buffers);

  /**
   * @brief send boost mutable buffers
   * @param buffer boost mutable buffers
   * @return bytes transfered
   */
  size_t send(const std::vector<boost::asio::mutable_buffers_1> &buffers);

  /**
   * @brief start async receiving
   * @param callback_recv callback function on receiving
   */
  void startAsyncReceiving(
      const std::function<void(const std::shared_ptr<uint8_t> &, const size_t)> &callback_recv);

  /**
   * @brief stop async receiving
   */
  void stopAsyncReceiving();

  /**
   * @brief get max length of single packet
   * @return max length of single packet in bytes
   */
  size_t maxLengthOfPacket() const { return max_len_packet_; }

 protected:
  /**
   * @brief inner thread to handle async receive
   */
  void threadReceive();

  /**
   * @brief inner callback function on receiving
   */
  void callbackReceive(const std::shared_ptr<uint8_t> &data, const size_t size,
                       const boost::system::error_code ec);

 protected:
  const size_t max_len_packet_;

  boost::asio::io_service ios_;
  boost::asio::ip::udp::endpoint ep_multicast_;
  boost::asio::ip::udp::socket socket_send_;
  boost::asio::ip::udp::socket socket_recv_;

  std::function<void(const std::shared_ptr<uint8_t> &, const size_t)> callback_recv_;
  std::shared_ptr<std::thread> handle_thread_receive_;
  std::atomic<bool> enable_thread_receive_;
};

}  // namespace shame
