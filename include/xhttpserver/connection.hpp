// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_CONNECTION_HPP
#define X_HTTP_SERVER_CONNECTION_HPP

#include <array>
#include <memory>

#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"

#include "xhttpserver/response.hpp"
#include "xhttpserver/request.hpp"
#include "xhttpserver/handler.hpp"
#include "xhttpserver/header_parser.hpp"
#include "xhttpserver/config.hpp"
#include "xhttpserver/connection_manager.hpp"

namespace xhttpserver {

// Represents a single Connection from a client.
class Connection : public std::enable_shared_from_this<Connection> {
 public:
  Connection(const Connection &) = delete;
  Connection &operator=(const Connection &) = delete;

  // Construct a Connection with the given socket.
  explicit Connection(boost::asio::ip::tcp::socket socket, config_ptr config,
                      connection_manager_ptr manager, handle_ptr handler);

  // Start the first asynchronous operation for the Connection.
  void start();

  // Stop all asynchronous operations associated with the Connection.
  void stop();

  // Get address info
  std::string get_address_info();

  // check thread is idle
  bool idle(std::time_t &now) {
    return (config_->get_keep_alive_timeout()) <= static_cast<int>(now - last_active_time_);
  };

 private:
  // Perform an asynchronous read operation.
  void do_read(bool first);

  // Perform an asynchronous write operation.
  void do_write();

  // Control connection timeout
  void set_timeout(long milliseconds);
  void cancel_timeout();

  // The server config object
  config_ptr config_ = nullptr;

  // Last active time
  std::time_t last_active_time_;

  // Socket for the Connection.
  boost::asio::ip::tcp::socket socket_;
  //
  boost::asio::io_service::strand strand_;

  // The manager for this Connection.
  connection_manager_ptr connection_manager_;

  // The handler used to process the incoming XHttpRequest.
  handle_ptr handler_;

  // Buffer for incoming data.
  boost::asio::streambuf buffer_;

  // The incoming XHttpRequest.
  XHttpRequest request_;

  // The parser for the incoming XHttpRequest.
  HeaderParser request_header_parser_;

  // The XHttpResponse to be sent back to the client.
  XHttpResponse response_;

  // Timer used for timeout control
  std::unique_ptr<boost::asio::steady_timer> timer_;

  bool is_stop_;
};

typedef std::shared_ptr<Connection> connection_ptr;

} // namespace xhttpserver

#endif // X_HTTP_SERVER_CONNECTION_HPP
