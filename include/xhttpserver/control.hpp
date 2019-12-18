// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_CONTROL_H
#define X_HTTP_SERVER_CONTROL_H

#include <string>
#include <set>

#include "glog/logging.h"

#include "xhttpserver/request.hpp"
#include "xhttpserver/response.hpp"

// Used to build Control.
#define X_HTTP_CONTROL_FUNCTION_SIGNATURE(index, method) \
  xhttpserver::XHttpResponse http_##method##_##index(xhttpserver::XHttpRequest &req)
#define X_HTTP_CONTROL_CLASS_(index, method) method##_##index##_class
#define X_HTTP_CONTROL_INSTANCE_(index, method) method##_##index##_instance

#define X_HTTP_SERVICE(control_name, index, uri, method)  \
  friend class X_HTTP_CONTROL_CLASS_(index, method); \
  class X_HTTP_CONTROL_CLASS_(index, method): public xhttpserver::XHttpControlInterface { \
    public: \
      explicit X_HTTP_CONTROL_CLASS_(index, method)(control_name *control): control_(control) {\
        control_->register_control(this, #uri, #method);\
        LOG(INFO) << "register new control: " << #control_name << "->[" << #method << "]" << #uri; \
      }\
      xhttpserver::XHttpResponse apply(xhttpserver::XHttpRequest &req) { \
        return control_->http_##method##_##index(req); \
      }\
    private: \
      control_name *control_; \
  }; \
  private: \
    X_HTTP_CONTROL_CLASS_(index, method) X_HTTP_CONTROL_INSTANCE_(index, method) =  \
      X_HTTP_CONTROL_CLASS_(index, method)(this);\
    X_HTTP_CONTROL_FUNCTION_SIGNATURE(index, method)

// API for control. index do not has any meaning, only used to express uniqueness.
#define X_HTTP_GET(control_name, index, uri) X_HTTP_SERVICE(control_name, index, uri, get)
#define X_HTTP_POST(control_name, index, uri) X_HTTP_SERVICE(control_name, index, uri, post)
#define X_HTTP_HEAD(control_name, index, uri) X_HTTP_SERVICE(control_name, index, uri, head)

namespace xhttpserver {
class XHttpControlInterface {
 public:
  virtual xhttpserver::XHttpResponse apply(xhttpserver::XHttpRequest &req) { return xhttpserver::XHttpResponse::success(""); };
  virtual ~XHttpControlInterface() {};
};

// Control api. multiple interfaces inside control is non-thread-safe.
// Use Case like this:
// class UserControl: public XHttpControl {
//    X_HTTP_HEAD(XHttpControl, 0, "/test"){
//        deal with xhttpserver::XHttpRequest &req
//    }
//    X_HTTP_POST(XHttpControl, 0, "/test"){
//        deal with xhttpserver::XHttpRequest &req
//    }
//    X_HTTP_GET(XHttpControl, 0, "/test"){
//        deal with xhttpserver::XHttpRequest &req
//    }
// }
class XHttpControl {
 public:
  const std::set<std::tuple<std::string, std::string, XHttpControlInterface *>> &get_controls() const {
    return controls_;
  };

 protected:
  void register_control(XHttpControlInterface *wrap, std::string uri, std::string method) {
    controls_.insert(std::make_tuple(method, uri, wrap));
  }

 private:
  std::set<std::tuple<std::string, std::string, XHttpControlInterface *>> controls_;
};
}

#endif //X_HTTP_SERVER_CONTROL_H
