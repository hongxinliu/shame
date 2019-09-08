/*
 * Copyright (C) 2019 Hongxin Liu. All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include "shame/udpm/udpm.h"
#include "shame/udpm/socket.h"
#include "shame/common/thread_safe_queue.h"
#include <vector>

namespace shame {

Udpm::Udpm(const std::string &multicast_addr,
           const uint16_t multicast_port,
           const int ttl) :
signature_udpm_message_(0x19651116),
signature_shm_message_(0x19691125),
socket_(new Socket(multicast_addr, multicast_port, ttl)),
msg_queue_(new ThreadSafeQueue<std::pair<std::shared_ptr<uint8_t>, size_t>>()),
e_(std::random_device {}()),
d_(0, 0xffffffff) {
}

Udpm::~Udpm() {
  stopAsyncReceiving();
}

void Udpm::startAsyncReceiving(const std::function<void(const std::string &,
                                                        std::shared_ptr<uint8_t>,
                                                        size_t,
                                                        bool)> &callback_recv) {
  stopAsyncReceiving();
  callback_recv_ = callback_recv;
  msg_queue_->clear();
  msg_queue_->reset();

  enable_thread_pack_.store(true);
  handle_thread_pack_.reset(new std::thread(std::bind(&Udpm::threadPack, this)));

  socket_->startAsyncReceiving(std::bind(&Udpm::callbackReceive, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
}

void Udpm::stopAsyncReceiving() {
  socket_->stopAsyncReceiving();

  enable_thread_pack_.store(false);
  msg_queue_->breakAllWait();

  if (handle_thread_pack_) {
    handle_thread_pack_->join();
    handle_thread_pack_.reset();
  }
}

size_t Udpm::send(const std::string &channel,
            const void *payload,
            const size_t len_payload,
            const bool shared_memory) {
  Header header;
  header.signature = (shared_memory ? signature_shm_message_ : signature_udpm_message_);
  header.id = d_(e_);
  header.len_payload = len_payload;

  if (sizeof(Header) + channel.size() + 1 + len_payload <= socket_->maxLengthOfPacket()) {
    header.num_packets = 1;
    header.offset = 0;
    return send(header, channel, payload, len_payload) - sizeof(Header) - channel.size() - 1;
  } else {
    const size_t max_len_payload_per_packet = socket_->maxLengthOfPacket() - sizeof(Header) - channel.size() - 1;
    uint32_t num_packets = len_payload / max_len_payload_per_packet;
    if (num_packets * max_len_payload_per_packet < len_payload) {
      ++num_packets;
    }

    header.num_packets = num_packets;
    header.offset = 0;

    size_t len_sent_payload = 0;
    for (uint32_t packet_no = 0; packet_no < num_packets - 1; ++packet_no) {
      len_sent_payload += send(header, channel, (const uint8_t *)payload + header.offset, max_len_payload_per_packet)
                          - sizeof(Header) - channel.size() - 1;
      header.offset += max_len_payload_per_packet;
    }

    len_sent_payload += send(header, channel, (const uint8_t *)payload + header.offset, len_payload - header.offset)
                        - sizeof(Header) - channel.size() - 1;
    return len_sent_payload;
  }
}

void Udpm::callbackReceive(std::shared_ptr<uint8_t> data, const size_t size) {
  msg_queue_->enqueue(std::make_pair(data, size));
}

void Udpm::threadPack() {
  while (enable_thread_pack_.load()) {
    std::pair<std::shared_ptr<uint8_t>, size_t> packet;
    if (!msg_queue_->waitDequeue(&packet)) {
      continue;
    }

    auto p = packet.first.get();
    auto header = reinterpret_cast<Header *>(p);
    std::string channel(reinterpret_cast<char *>(p) + sizeof(Header));
    auto data = p + sizeof(Header) + channel.size() + 1;

    if (header->num_packets == 1) {
      std::shared_ptr<uint8_t> payload(new uint8_t[header->len_payload], std::default_delete<uint8_t[]>());
      memcpy(payload.get(), data, header->len_payload);
      callback_recv_(channel, payload, header->len_payload, (header->signature == signature_shm_message_));
    } else {
      auto it = msg_buffer_.find(header->id);
      if (it == msg_buffer_.end()) {
        MessageBuffer buffer;
        buffer.header = *header;
        buffer.channel = channel;
        buffer.num_received = 0;
        buffer.payload.reset(new uint8_t[header->len_payload], std::default_delete<uint8_t[]>());
        msg_buffer_[header->id] = buffer;
        it = msg_buffer_.find(header->id);
      }

      memcpy(it->second.payload.get() + header->offset, data, packet.second - sizeof(Header) - channel.size() - 1);
      if (++it->second.num_received == it->second.header.num_packets) {
        callback_recv_(it->second.channel, it->second.payload,
                       it->second.header.len_payload,
                       (it->second.header.signature == signature_shm_message_));
        msg_buffer_.erase(it);
      }
    }
  }
}

size_t Udpm::send(const Header &header,
            const std::string &channel,
            const void *payload,
            const size_t len_payload) {
  std::vector<boost::asio::const_buffers_1> buffers;
  buffers.emplace_back(&header, sizeof(header));
  buffers.emplace_back(channel.data(), channel.size() + 1);
  buffers.emplace_back(payload, len_payload);
  return socket_->send(buffers);
}

}  // namespace shame
