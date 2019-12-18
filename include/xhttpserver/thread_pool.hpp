// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_THREAD_POOL_HPP
#define X_HTTP_SERVER_THREAD_POOL_HPP

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>

namespace xhttpserver {
class ThreadPool {
 public:
  ThreadPool(int threadNum = std::thread::hardware_concurrency()): threadNum_(threadNum), work_(new boost::asio::io_service::work(service_)) {

  }

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  boost::asio::io_service &get_io_service() {
    return service_;
  }

  void run() {
    for (int i = 0; i < threadNum_; ++i) {
      threads_.emplace_back([this]() { service_.run(); });
    }
    work_.reset();
    for (auto &t: threads_) {
      t.join();
    }
  }

 private:
  int threadNum_;
  boost::asio::io_service service_;
  std::unique_ptr<boost::asio::io_service::work> work_;
  std::vector<std::thread> threads_;
};
}

#endif //X_HTTP_SERVER_THREAD_POOL_HPP
