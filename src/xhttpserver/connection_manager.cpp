// by ze.chen@hobot.cc

#include "xhttpserver/connection_manager.hpp"

#include "xhttpserver/connection.hpp"

namespace xhttpserver {

ConnectionManager::ConnectionManager(config_ptr config) : config_(config) {
}

void ConnectionManager::start(connection_ptr c) {
  std::lock_guard<std::mutex> lck(mutex_);
  if (connections_.size() >= config_->get_max_connection_num()) {
    LOG(ERROR) << "Connection num is exceed " << config_->get_max_connection_num() << ". Request from "
               << c.get()->get_address_info() << " is rejected";
    c->stop();
    return;
  }
  connections_.insert(c);
  LOG(INFO) << "new connection:" << c.get()->get_address_info() << ". connection num: " << connections_.size();
  c->start();
}

void ConnectionManager::stop(connection_ptr c) {
  std::lock_guard<std::mutex> lck(mutex_);
  connections_.erase(c);
  LOG(INFO) << "close connection. connection num: " << connections_.size();
  c->stop();
}

void ConnectionManager::stop_all() {
  std::lock_guard<std::mutex> lck(mutex_);
  LOG(INFO) << "close all connection. connection num: " << connections_.size();
  for (auto c: connections_)
    c->stop();
  connections_.clear();
}
} // namespace xhttpserver
