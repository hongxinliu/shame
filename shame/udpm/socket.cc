/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include "shame/udpm/socket.h"
#include <boost/bind.hpp>

namespace ba = boost::asio;

namespace shame {

static const size_t kLenIpHeader = 20;
static const size_t kLenUdpHeader = 8;

Socket::Socket(const std::string &multicast_addr, const uint16_t multicast_port, const int ttl) :
max_len_packet_((ttl == 0 ? 65535 : 1500) - kLenIpHeader - kLenUdpHeader),
ep_multicast_(ba::ip::address::from_string(multicast_addr), multicast_port),
socket_send_(ios_, ep_multicast_.protocol()),
socket_recv_(ios_, ep_multicast_.protocol()) {
  // set TTL of send socket
  socket_send_.set_option(ba::ip::multicast::hops(ttl));

  // bind receive socket to multicast port
  socket_recv_.set_option(ba::ip::udp::socket::reuse_address(true));
  socket_recv_.bind(ep_multicast_);

  // add receive socket to multicast group
  socket_recv_.set_option(ba::ip::multicast::join_group(ep_multicast_.address()));
}

Socket::~Socket() {
  stopAsyncReceiving();
}

size_t Socket::send(const void *data, const size_t size) {
  return socket_send_.send_to(ba::buffer(data, size), ep_multicast_);
}

size_t Socket::send(const std::string &data) {
  return socket_send_.send_to(ba::buffer(data), ep_multicast_);
}

size_t Socket::send(const boost::asio::const_buffers_1 &buffer) {
  return socket_send_.send_to(buffer, ep_multicast_);
}

size_t Socket::send(const boost::asio::mutable_buffers_1 &buffer) {
  return socket_send_.send_to(buffer, ep_multicast_);
}

size_t Socket::send(const std::vector<boost::asio::const_buffers_1> &buffers) {
  return socket_send_.send_to(buffers, ep_multicast_);
}

size_t Socket::send(const std::vector<boost::asio::mutable_buffers_1> &buffers) {
  return socket_send_.send_to(buffers, ep_multicast_);
}

void Socket::startAsyncReceiving(const std::function<void(std::shared_ptr<uint8_t>, size_t)> &callback_recv) {
  callback_recv_ = callback_recv;
  stopAsyncReceiving();

  enable_thread_receive_.store(true);
  handle_thread_receive_.reset(new std::thread(&Socket::threadReceive, this));
}

void Socket::stopAsyncReceiving() {
  // break from ThreadReceive
  enable_thread_receive_.store(false);
  ios_.stop();

  // join ThreadReceive
  if (handle_thread_receive_) {
    handle_thread_receive_->join();
    handle_thread_receive_.reset();
  }

  // reset io_service to normal status
  ios_.reset();
}

void Socket::threadReceive() {
  // trigger the first async receive
  std::shared_ptr<uint8_t> buffer(new uint8_t[max_len_packet_], std::default_delete<uint8_t[]>());
  socket_recv_.async_receive(ba::buffer(buffer.get(), max_len_packet_),
                             boost::bind(&Socket::callbackReceive, this, buffer,
                                         ba::placeholders::bytes_transferred(),
                                         ba::placeholders::error()));

  // handle async messages
  while (enable_thread_receive_.load()) {
    ios_.run();
  }
}

void Socket::callbackReceive(std::shared_ptr<uint8_t> data, size_t size,
                             boost::system::error_code ec) {
  // if no error found, callback to user
  if (!ec) {
    callback_recv_(data, size);
  }

  // trigger next async receive
  std::shared_ptr<uint8_t> buffer(new uint8_t[max_len_packet_], std::default_delete<uint8_t[]>());
  socket_recv_.async_receive(ba::buffer(buffer.get(), max_len_packet_),
                             boost::bind(&Socket::callbackReceive, this, buffer,
                                         ba::placeholders::bytes_transferred(),
                                         ba::placeholders::error()));
}

}  // namespace shame
