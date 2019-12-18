// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_REQUEST_HPP
#define X_HTTP_SERVER_REQUEST_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <chrono>

#include "glog/logging.h"

#include "xhttpserver/header.hpp"

namespace xhttpserver {

// Request API: A XHttpRequest received from a client.
class XHttpRequest {
  // private param is accessable by inner class
  friend class HeaderParser;
  friend class Connection;
  friend class Handler;
  friend class StaticHandler;

 public:
  XHttpRequest(const XHttpRequest &) = delete;
  XHttpRequest &operator=(const XHttpRequest &) = delete;
  XHttpRequest() {}

 public:
  // API
  const std::string &remote() const {
    return remote_;
  }
  const std::string &content() const {
    return content_;
  }
  const std::string &method() const {
    return method_;
  }
  const std::string &uri() const {
    return uri_;
  }
  const std::string &query_string() const {
    return query_string_;
  }
  const std::string params(const std::string key) const {
    auto res = query_string_map_.find(key);
    if (res == query_string_map_.end()) {
      return "";
    } else {
      return res->second;
    }
  }
  const std::vector<Header> &headers() const {
    return headers_;
  }
  const std::string header(const std::string &key) const {
    for (auto item: headers_) {
      if (item.name.compare(key) == 0) {
        return item.value;
      }
    }
    return "";
  }
  const bool get_keep_alive() const {
    return keep_alive_;
  }
  //end API

 private:
  void finish(int status) {
    status_ = status;
    sum_cost_ = double((std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - start_)).count());
    sum_cost_ = sum_cost_ * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
    std::ostringstream oss;
    oss << " from=" << remote_;
    oss << " method=" << method_;
    oss << " uri=" << uri_;
    oss << " query_string=[" << query_string_ << "]";
    oss << " status=" << status_;
    oss << " sum_cost=" << sum_cost_ * 1000 << "ms";
    oss << " computer_cost=" << computer_cost_ * 1000 << "ms";
    oss << " filters=[";
    std::for_each(step_cost_.begin(), step_cost_.end(), [&oss](std::pair<std::string, double> step) {
      oss << step.first << "=" << std::to_string(step.second * 1000) << "ms ";
    });
    oss << "]";
    LOG(INFO) << oss.str();
  }
  void reset() {
    remote_ = "";
    method_ = "";
    uri_ = "";
    query_string_ = "";
    query_string_map_.clear();
    headers_.clear();
    content_length_ = 0;
    content_ = "";
    keep_alive_ = false;
    start_ = std::chrono::system_clock::now();
    step_cost_.clear();
    computer_cost_ = 0.0;
    sum_cost_ = 0.0;
    status_ = 0;
  }
 private:
  std::string remote_;
  std::string method_;
  std::string uri_;
  std::string query_string_;
  std::map<std::string, std::string> query_string_map_;
  std::vector<Header> headers_;
  std::size_t content_length_ = 0;
  std::string content_ = "";
  int http_version_major_ = 1;
  int http_version_minor_ = 0;
  bool keep_alive_ = false;
  //
  std::chrono::system_clock::time_point start_ = std::chrono::system_clock::now();
  std::vector<std::pair<std::string, double>> step_cost_;
  double computer_cost_ = 0.0;
  double sum_cost_ = 0.0;
  //
  int status_;
};
} // namespace xhttpserver

#endif // X_HTTP_SERVER_REQUEST_HPP
