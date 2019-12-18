// by ze.chen@hobot.cc
#include <iostream>
#include <glog/logging.h>
#include <boost/asio.hpp>
#include "xhttpserver/api.hpp"


namespace xhttpserver {

class DemoFilter : public XHttpFilter {
 public:
  explicit DemoFilter() : XHttpFilter("demo") {}
  XHttpFilter::FilterResult filter(XHttpRequest &req, XHttpResponse &rep) {
    LOG(INFO) << "demo filter";
    return XHttpFilter::next;
  }
};

class DemoControl : public XHttpControl {
  X_HTTP_GET(DemoControl, 0, /api/demo/test) {
    LOG(INFO) << "get /api/demo/test";
    return XHttpResponse::success("success");
  }
  X_HTTP_HEAD(DemoControl, 0, /api/demo/test) {
    LOG(INFO) << "head /api/demo/test";
    return XHttpResponse::success("success");
  }
  X_HTTP_POST(DemoControl, 0, /api/demo/test) {
    LOG(INFO) << "test /api/demo/test";
    LOG(INFO) << req.content();
    LOG(INFO) << req.content().length();
    return XHttpResponse::success("success");
  }
};
class DemoServer : public XHttpServer {
 public:
  DemoServer(int concurrency) : XHttpServer(concurrency) {}
 private:
  virtual void config_router(XHttpRouter *router) {
    std::set<std::string> master_uris;
    master_uris.insert("/api/demo/test");
    router
        ->filter(new MasterRedirect(master_uris, this))
        ->filter(new DemoFilter())
        ->add(new DemoControl());
  }
  virtual int config_max_connection_num() const { return 2000; }
  virtual int config_request_read_timeout() const { return 100000; }
  virtual ServerStatusCallback config_zookeeper_callback() { return FUNCTION_EMPTY_CALLBACK;}
};
}

xhttpserver::XHttpServer *xhttpserver::XHttpServer::ptr = nullptr;

int main(int argc, char *argv[]) {
  try {
    xhttpserver::DemoServer server(30);
    server.init();
    server.run();
  }
  catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << "\n";
  }


  return 0;
}
