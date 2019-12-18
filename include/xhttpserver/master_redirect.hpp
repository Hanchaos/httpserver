// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_MASTER_REDIRECT_HPP
#define X_HTTP_SERVER_MASTER_REDIRECT_HPP

#include "xhttpserver/filter.hpp"
#include "xhttpserver/server.hpp"
#include <set>

namespace xhttpserver {
class MasterRedirect : public XHttpFilter {
 public:
  MasterRedirect(std::set<std::string> uris, XHttpServer *server)
      : XHttpFilter("master_redirect"), uris_(uris), server_(server) {
  }

  virtual XHttpFilter::FilterResult filter(XHttpRequest &req, XHttpResponse &rep) {
    if (req.method() == "get" && !server_->is_master_ && !server_->master_path_.empty()) {
      auto result = uris_.find(req.uri());
      if (result != uris_.end()) {
        std::string url = "http://"+server_->master_data_ + req.uri();
        if (req.query_string().length() != 0) {
          url = url + "?" + req.query_string();
        }
        LOG(INFO) << "redirect to master: " + server_->master_data_;
        rep = XHttpResponse::redirect(url);
        return XHttpFilter::stop;
      }
    }
    return XHttpFilter::next;
  }
 private:
  std::set<std::string> uris_;
  XHttpServer *server_;
};
}

#endif //X_HTTP_SERVER_MASTER_REDIRECT_HPP
