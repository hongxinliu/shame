/*
 * Copyright (c) 2019 Hongxin Liu. All rights reserved.
 * Licensed under the MIT License. See the LICENSE file for details.
 *
 * Author: Hongxin Liu <hongxinliu.com> <github.com/hongxinliu>
 * Date: Sept.08, 2019
 */

#include <boost/interprocess/managed_shared_memory.hpp>
#include <signal.h>
#include <memory>
#include <thread>
#include <iostream>

namespace bi = boost::interprocess;

class ShameServer {
 public:
  ShameServer(const std::string &name, const size_t size) :
  name_(name) {
    bi::shared_memory_object::remove(name_.c_str());
    bi::managed_shared_memory segment(bi::create_only, name_.c_str(), size);
    std::cout << "Allocated " << size << " bytes for shared memory segment: " << name_ << std::endl;
  }

  ~ShameServer() {
    bi::shared_memory_object::remove(name_.c_str());
    std::cout << "Removed shared memory segment: " << name_ << std::endl; 
  }

 protected:
  const std::string name_;
};

std::unique_ptr<ShameServer> shame_server;

void sig_handler(int sig) {
  if (sig == SIGINT) {
    std::cout << "Exiting on signal SIGINT" << std::endl;
    shame_server.reset();
    exit(1);
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " NAME SIZE" << std::endl;
  }

  signal(SIGINT, sig_handler);

  try {
    shame_server.reset(new ShameServer(argv[1], std::stoi(argv[2])));
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
