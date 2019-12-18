// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_FILTER_H
#define X_HTTP_SERVER_FILTER_H

#include "glog/logging.h"

#include "xhttpserver/request.hpp"
#include "xhttpserver/response.hpp"

namespace xhttpserver {

// Filter API
// Use Case like this:
// class UserFilter: public XHttpFilter {
//    public FilterResult filter(XHttpRequest *request, XHttpResponse *response) {
//        return FilterResult.next
//    }
//  }
class XHttpFilter {
 public:
  explicit XHttpFilter(const std::string &name_) : name_(name_) {}
  virtual ~XHttpFilter() {}

  // Filter can be add before control or end of control.
  enum FilterType {
    pre_filter, post_filter
  };

  enum FilterResult {
    next, stop
  };
  // Filter API for one request. filter can change request and response.
  // where filter resutlt is stop, response must be set by filter.
  virtual FilterResult filter(XHttpRequest &req, XHttpResponse &rep)= 0;

 public:
  // Use for log
  const std::string &get_name() const {
    return name_;
  }

 private:
  std::string name_;
};
}

#endif //X_HTTP_SERVER_FILTER_H
