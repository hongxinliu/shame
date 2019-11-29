/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#pragma once

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <functional>
#include <string>
#include <utility>

namespace shame {

template <typename T>
using Allocator =
    boost::interprocess::allocator<T, boost::interprocess::managed_shared_memory::segment_manager>;

using String = boost::container::basic_string<char, std::char_traits<char>, Allocator<char>>;

template <typename T>
using Vector = boost::container::vector<T, Allocator<T>>;

template <typename K, typename M>
using Pair = std::pair<const K, M>;

template <typename K, typename M>
using Map = boost::container::map<K, M, std::less<K>, Allocator<Pair<K, M>>>;

class ShameData {
 public:
  explicit ShameData(const boost::interprocess::managed_shared_memory& msm)
      : data_(msm.get_segment_manager()) {}

 public:
  size_t size() const { return data_.size(); }

  const uint8_t* data() const { return data_.data(); }

 public:
  mutable boost::interprocess::interprocess_sharable_mutex mutex_;
  Vector<uint8_t> data_;
};

}  // namespace shame
