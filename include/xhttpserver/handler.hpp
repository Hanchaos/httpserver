// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_REQUEST_HANDLER_HPP
#define X_HTTP_SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <tuple>

#include "xhttpserver/config.hpp"
#include "xhttpserver/router.hpp"
#include "xhttpserver/static_handler.hpp"
#include "xhttpserver/request.hpp"
#include "xhttpserver/response.hpp"

namespace xhttpserver {

// The common handler for all incoming requests.
class Handler {
 public:
  Handler(const Handler &) = delete;
  Handler &operator=(const Handler &) = delete;

  // Construct with a directory containing files to be served.
  explicit Handler(config_ptr config, router_ptr router);

  // Handle a XHttpRequest and produce a XHttpResponse.
  void handle_request(XHttpRequest &req, XHttpResponse &rep);

 private:
  // The server config object
  config_ptr config_;

  //The router object
  router_ptr router_;

  // The handler for static requests.
  static_handler_ptr static_handler_;
};


typedef std::shared_ptr<Handler> handle_ptr;
} // namespace xhttpserver

#endif // X_HTTP_SERVER_REQUEST_HANDLER_HPP
