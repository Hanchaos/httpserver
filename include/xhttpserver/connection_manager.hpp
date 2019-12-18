// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_CONNECTION_MANAGER_HPP
#define X_HTTP_SERVER_CONNECTION_MANAGER_HPP

#include <set>
#include <mutex>

#include "xhttpserver/config.hpp"

namespace xhttpserver {

class Connection;
typedef std::shared_ptr<Connection> connection_ptr;
// Manages open connections so that they may be cleanly stopped when the Server
// needs to shut down.
class ConnectionManager {
 public:
  ConnectionManager(const ConnectionManager &) = delete;
  ConnectionManager &operator=(const ConnectionManager &) = delete;

  // Construct a Connection manager.
  ConnectionManager(config_ptr config);

  // Add the specified Connection to the manager and start it.
  void start(xhttpserver::connection_ptr c);

  // Stop the specified Connection.
  void stop(xhttpserver::connection_ptr c);

  // Stop all connections.
  void stop_all();

 private:
  // The server config object
  config_ptr config_;
  // The mutex to control multi-threads.
  std::mutex mutex_;
  // The managed connections.
  std::set<xhttpserver::connection_ptr> connections_;
};

typedef std::shared_ptr<ConnectionManager> connection_manager_ptr;

} // namespace xhttpserver

#endif // X_HTTP_SERVER_CONNECTION_MANAGER_HPP
