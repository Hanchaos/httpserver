// by ze.chen@hobot.cc

#include "xhttpserver/connection.hpp"

namespace xhttpserver {

Connection::Connection(boost::asio::ip::tcp::socket socket, config_ptr config,
                       connection_manager_ptr manager, handle_ptr handler)
    : socket_(std::move(socket)),
      strand_(socket_.get_io_service()),
      is_stop_(false),
      config_(config),
      connection_manager_(manager),
      buffer_(config->get_max_request_size_bytes()),
      handler_(handler) {
  buffer_.prepare(config_->get_max_request_size_bytes());
  request_.remote_ = get_address_info();
}

std::string Connection::get_address_info() {
  std::string address = "";
  try {
    address = socket_.lowest_layer().remote_endpoint().address().to_string()
        + ":" + std::to_string(socket_.lowest_layer().remote_endpoint().port());
  } catch (std::exception &e) {
    LOG(ERROR) << "get address info fail. reason: " << e.what();
  }
  return address;
}

void Connection::start() {
  do_read(true);
}

void Connection::stop() {
  try {
    if (!is_stop_) {
      socket_.close();
      is_stop_ = true;
    }
  } catch (boost::system::system_error &e) {
    LOG(ERROR) << "conection stop exception,e = " << e.what();
  }
}

void Connection::set_timeout(long milliseconds) {
  if (milliseconds == 0) {
    timer_ = nullptr;
    return;
  }
  timer_ = std::unique_ptr<boost::asio::steady_timer>(new boost::asio::steady_timer(socket_.get_io_service()));
  timer_->expires_from_now(std::chrono::milliseconds(milliseconds));
  auto self(shared_from_this());
  timer_->async_wait([self](const boost::system::error_code &ec) {
    //if (!ec)
    //  LOG(ERROR) << "Connection processing timeout. stop the connection: " << self->request_.remote();
    if (ec != boost::asio::error::operation_aborted)
      self->stop();
  });
}

void Connection::cancel_timeout() {
  if (timer_) {
    boost::system::error_code ec;
    timer_->cancel(ec);
  }
}

void Connection::do_read(bool first) {
  auto self(shared_from_this());
  // Processing the first request, or wait for keep alive request
  if (first) {
    set_timeout(config_->get_request_read_timeout());
  } else {
    set_timeout(config_->get_keep_alive_timeout());
  }

  std::chrono::system_clock::time_point read_start = std::chrono::system_clock::now();
  // read http header from socket stream
  boost::asio::async_read_until(
      socket_,
      buffer_, "\r\n\r\n",
      strand_.wrap([this, self, read_start](boost::system::error_code ec, std::size_t bytes_transferred) {
        cancel_timeout();
        //
        double read_cost = double((std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now() - read_start)).count());
        read_cost = read_cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
        request_.step_cost_.push_back(std::make_pair("http-read", read_cost));
        // success request
        auto time_point = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        last_active_time_ =
            (std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch())).count();
        if (!ec) {
          if (buffer_.size() >= config_->get_max_request_size_bytes()) {
            response_ = XHttpResponse::error(XHttpResponse::http_code::forbidden);
            LOG(ERROR) << "request param size is too bigger. the max param is:"
                       << config_->get_max_request_size_bytes();
            do_write();
          } else {
            HeaderParser::result_type result;
            std::string bufferStr(
                boost::asio::buffers_begin(buffer_.data()),
                boost::asio::buffers_begin(buffer_.data()) + buffer_.size());
            //
            std::tie(result, std::ignore) = request_header_parser_.parse(
                request_, bufferStr.begin(), bufferStr.begin() + bytes_transferred);
            if (result == HeaderParser::good) {
              if (request_.content_length_ > 0) {
                // buffer_.size() is not necessarily the same as bytes_transferred, from Boost-docs:
                // "After a successful async_read_until operation, the streambuf may contain additional data beyond the delimiter"
                // The chosen solution is to extract lines from the stream directly when parsing the header. What is left of the
                // streambuf (maybe some bytes of the content) is appended to in the async_read-function below (for retrieving content).
                std::size_t num_additional_bytes = buffer_.size() - bytes_transferred;
                if (num_additional_bytes > 0) {
                  request_.content_ = std::string(bufferStr.begin() + bytes_transferred, bufferStr.end());
                }
                if (request_.content_length_ > num_additional_bytes) {
                  set_timeout(config_->get_request_content_read_timeout());
                  buffer_.consume(buffer_.size());
                  std::chrono::system_clock::time_point read_content_start = std::chrono::system_clock::now();
                  boost::asio::async_read(
                      socket_,
                      buffer_,
                      boost::asio::transfer_exactly(request_.content_length_ - num_additional_bytes),
                      strand_.wrap([this, self, read_content_start](boost::system::error_code ec,
                                                                        std::size_t bytes_transferred) {
                        cancel_timeout();
                        double read_content_cost = double((std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::system_clock::now() - read_content_start)).count());
                        read_content_cost =
                            read_content_cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
                        request_.step_cost_.push_back(std::make_pair("http-read-content", read_content_cost));
                        if (!ec) {
                          if (buffer_.size() >= config_->get_max_request_size_bytes()) {
                            LOG(ERROR) << "request param size is too bigger. the max param is:"
                                       << config_->get_max_request_size_bytes();
                            response_ = XHttpResponse::error(XHttpResponse::http_code::forbidden);
                            do_write();
                          } else {
                            std::string bufferStr(
                                boost::asio::buffers_begin(buffer_.data()),
                                boost::asio::buffers_begin(buffer_.data()) + buffer_.size());
                            request_.content_.append(bufferStr.begin(), bufferStr.end());
                            //
                            handler_->handle_request(request_, response_);
                            do_write();
                          }
                        } else {
                          LOG(ERROR) << "close connection[" << self->request_.remote()
                                     << "] for read error. error_code: "
                                     << ec;
                          request_.finish(XHttpResponse::http_code::service_unavailable);
                          boost::system::error_code ignored_ec;
                          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
                          connection_manager_->stop(shared_from_this());
                        }
                      }));
                } else {
                  //
                  handler_->handle_request(request_, response_);
                  do_write();
                }
              } else {
                handler_->handle_request(request_, response_);
                do_write();
              }
            } else {
              response_ = XHttpResponse::error(XHttpResponse::http_code::bad_request);
              LOG(ERROR) << "request header parse failed";
              do_write();
            }
          }
        } else {
          LOG(ERROR) << "close connection[" << self->request_.remote() << "] for read error. error_code: " << ec;
          request_.finish(XHttpResponse::http_code::service_unavailable);
          boost::system::error_code ignored_ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
          connection_manager_->stop(shared_from_this());
        }
      }));
}

void Connection::do_write() {
  auto self(shared_from_this());
  set_timeout(config_->get_response_send_timeout());
  std::chrono::system_clock::time_point write_start = std::chrono::system_clock::now();
  boost::asio::async_write(
      socket_,
      response_.to_buffers(),
      strand_.wrap([this, self, write_start](boost::system::error_code ec, std::size_t bytes_sended) {
        cancel_timeout();
        double cost = double((std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now() - write_start)).count());
        cost = cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
        request_.step_cost_.push_back(std::make_pair("http-write", cost));
        if (!ec) {
          if (config_->get_close_connection_after_response() || !request_.get_keep_alive()) {
            LOG(INFO) << "close connection[" << self->request_.remote() << "]";
            boost::system::error_code ignored_ec;
            request_.finish(response_.status_);
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            connection_manager_->stop(shared_from_this());
          } else {
            LOG(INFO) << "continue to keep alive. connection:" << self->request_.remote();
            request_.finish(response_.status_);
            request_.reset();
            do_read(false);
          }
        } else {
          LOG(ERROR) << "close connection[" << self->request_.remote() << "] for some error. error_code: " << ec;
          boost::system::error_code ignored_ec;
          request_.finish(response_.status_);
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
          connection_manager_->stop(shared_from_this());
        }
      }));
}

} // namespace xhttpserver
