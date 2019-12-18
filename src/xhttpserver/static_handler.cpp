// by ze.chen@hobot.cc

#include "xhttpserver/static_handler.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace xhttpserver {

StaticHandler::StaticHandler(config_ptr config)
    : config_(config) {
}

struct {
  const char *extension;
  const char *mime_type;
} mappings[] = {
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"svg", "image/svg+xml"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"js", "text/javascript"},
    {"css", "text/css"},
};

std::string extension_to_type(const std::string &extension) {
  for (auto m: mappings) {
    if (m.extension == extension) {
      return m.mime_type;
    }
  }
  return "text/plain";
}

XHttpResponse StaticHandler::handle_request(XHttpRequest &req) {
  std::string request_path = req.uri_;
  // XHttpRequest path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos) {
    return XHttpResponse::error(XHttpResponse::http_code::bad_request);
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/') {
    request_path += "index.html";
  }

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of("/");
  std::size_t last_dot_pos = request_path.find_last_of(".");
  std::string extension;
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
    extension = request_path.substr(last_dot_pos + 1);
  }

  // Open the file to send back.
  std::string full_path = config_->get_doc_root() + request_path;
  std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!is) {
    return XHttpResponse::error(XHttpResponse::http_code::not_found);
  }

  // Fill out the XHttpResponse to be sent to the client.
  std::string content;
  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0)
    content.append(buf, is.gcount());
  return XHttpResponse::success(content);
}

} // namespace xhttpserver