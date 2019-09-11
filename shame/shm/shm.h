/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <string>
#include <google/protobuf/message_lite.h>
#include <boost/interprocess/managed_shared_memory.hpp>

namespace shame {

class ShameData;

class Shm {
 public:
  /**
   * @brief constructor, open managed shared memory (open only, throws on fail)
   * @param name name of managed shared memory
   */
  explicit Shm(const std::string &name);

 public:
  /**
   * @brief find named segment
   * @param key name of segment
   * @return pointer to segment, nullptr for not found
   */
  ShameData *find(const std::string &key);

  /**
   * @brief find or construct segment
   * @param key name of segment
   * @return found or constructed segment
   */
  ShameData *find_or_construct(const std::string &key);

  /**
   * @brief put data into named segment
   * @param key name of segment
   * @param data pointer of data to be put
   * @param size length (in bytes) to be put
   * @return bytes transfered
   */
  size_t put(const std::string &key, const void *data, const size_t size);

  /**
   * @brief serialize protobuf message into named segment
   * @param key name of segment
   * @param msg protobuf message
   * @return bytes transfered (serialized protobuf message)
   */
  size_t put(const std::string &key, const google::protobuf::MessageLite &msg);

 protected:
  boost::interprocess::managed_shared_memory msm_;
};

}  // namespace shame
