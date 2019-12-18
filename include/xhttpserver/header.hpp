// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_HEADER_HPP
#define X_HTTP_SERVER_HEADER_HPP

#include <string>

namespace xhttpserver {

struct Header {
  std::string name;
  std::string value;
};

} // namespace xhttpserver

#endif // X_HTTP_SERVER_HEADER_HPP
