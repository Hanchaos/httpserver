// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_ROUTER_H
#define X_HTTP_SERVER_ROUTER_H

#include <list>

#include "xhttpserver/config.hpp"
#include "xhttpserver/filter.hpp"
#include "xhttpserver/control.hpp"

namespace xhttpserver {
class XHttpRouter {
 public:
  XHttpRouter(const XHttpRouter &) = delete;
  XHttpRouter &operator=(const XHttpRouter &) = delete;
  XHttpRouter(config_ptr config) : config_(config) {
  }

 public:
  // Add filter API
  // The add filter' order is very important
  XHttpRouter * filter(XHttpFilter *imp, XHttpFilter::FilterType filter_type) {
    switch (filter_type) {
      case XHttpFilter::pre_filter:
        LOG(INFO) << "add new filter: " << imp->get_name() << "[pre]";
        pre_filters_.push_back(imp);
        break;
      case XHttpFilter::post_filter:
        LOG(INFO) << "add new filter: " << imp->get_name() << "[post]";
        post_filters_.push_back(imp);
        break;
    }
    return this;
  }

  // Add filter API
  // The add filter' order is very important
  XHttpRouter * filter(XHttpFilter *imp) {
    return filter(imp, XHttpFilter::pre_filter);
  }

  // Add control API
  XHttpRouter * add(XHttpControl *control) {
    LOG(INFO) << "add new control list: " << control->get_controls().size();
    controls_.insert(control->get_controls().begin(), control->get_controls().end());
    return this;
  }

 public:
  // Get pre_filter API
  const std::list<XHttpFilter *> &get_pre_filters() const {
    return pre_filters_;
  }

  // Get post_filter API
  const std::list<XHttpFilter *> &get_post_filters() const {
    return post_filters_;
  }

  // Get control API
  const std::set<std::tuple<std::string, std::string, XHttpControlInterface *>> &get_controls() const {
    return controls_;
  }

 private:
  // The server config object
  config_ptr config_;

  // PRE Filter list
  std::list<XHttpFilter *> pre_filters_;

  // POST Filter list
  std::list<XHttpFilter *> post_filters_;

  // Control set <method, uri, control>
  std::set<std::tuple<std::string, std::string, XHttpControlInterface *>> controls_;
};

typedef std::shared_ptr<XHttpRouter> router_ptr;
}
#endif //X_HTTP_SERVER_ROUTER_H
