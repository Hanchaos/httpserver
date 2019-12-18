// by ze.chen@hobot.cc

#include "xhttpserver/server.hpp"

namespace xhttpserver {
//zookeeper
void XHttpServerDiscoveryCallback(ServerStatus server_status, ServerStatusCallback callback) {
  XHttpServer::ptr->master_path_ = server_status.master_path;
  XHttpServer::ptr->master_data_ = server_status.master_data;
  XHttpServer::ptr->is_master_ = server_status.is_master;
  callback(server_status);
  LOG(INFO) << "master path: " << server_status.master_path \
 << "; master data: " << server_status.master_data \
 << "; is_master : " << server_status.is_master;
}
void FUNCTION_EMPTY_CALLBACK(ServerStatus &server_status) {
}

XHttpServer::XHttpServer(int concurrency) :
    thread_pool_(concurrency),
    socket_(thread_pool_.get_io_service()),
    acceptor_(thread_pool_.get_io_service()),
    signals_(thread_pool_.get_io_service()) {
  google::InitGoogleLogging("x_http_server");
  ptr = this;
  is_master_ = false;
  master_path_ = "";
  master_data_ = "";
};

bool XHttpServer::init() {
  build_config();
  std::string logDir = config_log_dir();
  if (logDir.length() > 0) {
    FLAGS_log_dir = logDir;
    FLAGS_logbufsecs = 0;//缓冲日志输出，默认0秒
    FLAGS_max_log_size = 1024; //最大日志大小为 1024MB
    FLAGS_stop_logging_if_full_disk = true;
  } else {
    FLAGS_logtostderr = true;
  }
  if (config_->get_zookeeper_url().empty() == false) {
    ServerStatusCallback callback =
        std::bind(XHttpServerDiscoveryCallback, std::placeholders::_1, config_zookeeper_callback());
    server_discovery_ptr = new ServerDiscovery(
        config_->get_zookeeper_url(),
        config_->get_zookeeper_root(),
        config_->get_server_zookeeper_data(),
        config_->get_zookeeper_timeout(),callback,
        config_->get_server_zookeeper_type());
  }
  return true;
}

void XHttpServer::run() {
  connection_manager_ = std::make_shared<ConnectionManager>(config_);
  router_ = std::make_shared<XHttpRouter>(config_);
  handler_ = std::make_shared<Handler>(config_, router_);

  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
  signals_.add(SIGQUIT);
  signals_.async_wait(
      [this](boost::system::error_code /*ec*/, int /*signo*/) {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        acceptor_.close();
        connection_manager_->stop_all();
      });

  LOG(INFO) << "start to build http server.";
  if (router_.get() != nullptr) {
    config_router(router_.get());
  }
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  auto endpoint = boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(config_address()), config_port());
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.set_option(boost::asio::ip::tcp::no_delay(true));
  try {
    acceptor_.bind(endpoint);
    acceptor_.listen();
  }
  catch (std::exception &e) {
    LOG(INFO) << "http server start with exception: " << e.what();
    exit(1);
  }

  LOG(INFO) << "http server start success."
            << " listen address: http://" << config_address() << ":" << config_port();

  do_accept();

  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  thread_pool_.run();
}
void XHttpServer::do_accept() {
  acceptor_.async_accept(
      socket_,
      [this](boost::system::error_code ec) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open()) {
          return;
        }
        if (!ec && socket_.is_open()) {
          connection_manager_->start(std::make_shared<Connection>(
              std::move(socket_), config_, connection_manager_, handler_));
        }

        do_accept();
      });
}

void XHttpServer::build_config() {
  config_ = std::make_shared<Config>();
  config_->set_keep_alive_timeout(config_keep_alive_timeout());
  config_->set_max_connection_num(config_max_connection_num());
  config_->set_max_request_size_bytes(config_max_request_size_bytes());
  config_->set_request_id_header_name(config_request_id_header_name());
  config_->set_request_read_timeout(config_request_read_timeout());
  config_->set_request_content_read_timeout(config_request_content_read_timeout());
  config_->set_doc_root(config_doc_root());
  config_->set_close_connection_after_response(close_connection_after_response());
  config_->set_response_send_timeout(config_response_send_timeout());
  //
  config_->set_zookeeper_url(config_zookeeper_url());
  config_->set_zookeeper_root(config_zookeeper_root());
  config_->set_zookeeper_timeout(config_zookeeper_timeout());
  config_->set_server_zookeeper_type(config_zookeeper_server_type());
  config_->set_server_zookeeper_data(config_address() + ":" + std::to_string(config_port()));
}

XHttpServer::~XHttpServer() {
  if (server_discovery_ptr != nullptr) {
    delete server_discovery_ptr;
    server_discovery_ptr = nullptr;
  }
  google::ShutdownGoogleLogging();
}

} // namespace xhttpserver
