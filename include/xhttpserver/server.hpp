// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_HPP
#define X_HTTP_SERVER_HPP

#include <string>

#include "boost/asio.hpp"
#include "glog/logging.h"

#include "xhttpserver/router.hpp"
#include "xhttpserver/connection.hpp"
#include "xhttpserver/connection_manager.hpp"
#include "xhttpserver/handler.hpp"
#include "xhttpserver/static_handler.hpp"
#include "xhttpserver/config.hpp"
#include "xhttpserver/thread_pool.hpp"
#include "xhttpserver/server_discovery.hpp"

namespace xhttpserver {

void FUNCTION_EMPTY_CALLBACK(ServerStatus &server_status);

class XHttpServer {

 public:
  XHttpServer(const XHttpServer &) = delete;
  XHttpServer &operator=(const XHttpServer &) = delete;
  XHttpServer(int concurrency);
  virtual ~XHttpServer();

  // Run the server's io_service loop.
  virtual bool init();
  void run();
  /**
   * Config api can be overwrite by sub server instance for config and build router.
   */
 protected:
  // Config api: set server ip to bind
  virtual std::string config_address() const { return "127.0.0.1"; }
  // Config api: set server port to bind
  virtual int config_port() const { return 8010; }
  // Config api: set server html doc root path
  virtual std::string config_doc_root() const { return "."; }
  // Config api: use to build server router
  // The filter' order is very important
  virtual void config_router(XHttpRouter *router) = 0;
  // Config api: config http log dir
  virtual std::string config_log_dir() const { return ""; }
  // Config api: use to build standard request log
  virtual std::string config_request_id_header_name() const { return "X-HTTP-SERVER-REQUEST-ID"; }
  // Config api: server max service connections num
  virtual int config_max_connection_num() const { return 3000; }
  // Config api: alive connection idle timeout. by ms
  virtual int config_keep_alive_timeout() const { return 30000; }
  // Config api: read request timeout. by ms
  virtual int config_request_read_timeout() const { return 3000; }
  // Config api: read request content timeout. by ms
  virtual int config_request_content_read_timeout() const { return 3000; }
  // Config api: send request timeout. by ms
  virtual int config_response_send_timeout() const { return 3000; }
  // Config api: max request content length
  virtual int config_max_request_size_bytes() const { return 1024 * 10; }
  // Config api: close_connection_after_response
  virtual bool close_connection_after_response() const { return false; }
  //Config api: zookeeper
  virtual std::string config_zookeeper_url() const {return "";}
  virtual std::string config_zookeeper_root() const {return "/xhttpserver";}
  virtual ServerType config_zookeeper_server_type() const {return ServerType::regist;}
  virtual int config_zookeeper_timeout() const {return 30000;}
  virtual ServerStatusCallback config_zookeeper_callback() { return FUNCTION_EMPTY_CALLBACK;}
  /////////////////////////


 private:
  // Perform an asynchronous accept operation.
  void do_accept();

  // The server config object
  config_ptr config_;

  void build_config();

  // The thread_pool used to perform asynchronous operations.
  ThreadPool thread_pool_;

  // The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  // Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  // The next socket to be accepted.
  boost::asio::ip::tcp::socket socket_;

  // The Connection manager which owns all live connections.
  connection_manager_ptr connection_manager_;

  // The handler for all incoming requests.
  handle_ptr handler_;

  // The router for all incoming requests.
  router_ptr router_;

  //The pointer for server discovery
  ServerDiscovery *server_discovery_ptr = nullptr;

 public:
  //Zookeeper
  friend void XHttpServerDiscoveryCallback(ServerStatus server_status, ServerStatusCallback callback);
  std::string master_path_;
  std::string master_data_;
  bool is_master_;
  static XHttpServer * ptr;
  ServerStatusCallback Callback;
};

} // namespace xhttpserver

#endif // X_HTTP_SERVER_HPP
