/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include "shame/shm/shm.h"
#include "shame/shame_data.h"

namespace bi = boost::interprocess;

namespace shame {

Shm::Shm(const std::string &name) : msm_(bi::open_only, name.c_str()) {}

ShameData *Shm::find(const std::string &key) { return msm_.find<ShameData>(key.c_str()).first; }

ShameData *Shm::find_or_construct(const std::string &key) {
  return msm_.find_or_construct<ShameData>(key.c_str())(msm_);
}

size_t Shm::put(const std::string &key, const void *data, const size_t size) {
  auto shame_data = find_or_construct(key);
  if (!shame_data) {
    return 0;
  }

  shame_data->mutex_.lock();
  shame_data->data_.resize(size);
  memcpy(shame_data->data_.data(), data, size);
  shame_data->mutex_.unlock();
  return size;
}

size_t Shm::put(const std::string &key, const google::protobuf::MessageLite &msg) {
  auto shame_data = find_or_construct(key);
  if (!shame_data) {
    return 0;
  }

  auto size = msg.ByteSize();
  shame_data->mutex_.lock();
  shame_data->data_.resize(size);
  msg.SerializeToArray(shame_data->data_.data(), size);
  shame_data->mutex_.unlock();

  return size;
}

}  // namespace shame
