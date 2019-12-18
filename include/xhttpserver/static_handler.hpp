// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_REQUEST_STATIC_HANDLER_HPP
#define X_HTTP_SERVER_REQUEST_STATIC_HANDLER_HPP

#include <string>

#include "glog/logging.h"

#include "xhttpserver/config.hpp"
#include "xhttpserver/request.hpp"
#include "xhttpserver/response.hpp"

namespace xhttpserver {

// The handler for static file requests.
class StaticHandler {
 public:
  StaticHandler(const StaticHandler &) = delete;
  StaticHandler &operator=(const StaticHandler &) = delete;

  // Construct with a directory containing files to be served.
  StaticHandler(config_ptr config);

  // Handle a XHttpRequest and produce a XHttpResponse.
  XHttpResponse handle_request(XHttpRequest &req);

 private:
  // The server config object
  config_ptr config_;
};

typedef std::shared_ptr<StaticHandler> static_handler_ptr;
} // namespace xhttpserver

#endif // X_HTTP_SERVER_REQUEST_STATIC_HANDLER_HPP
