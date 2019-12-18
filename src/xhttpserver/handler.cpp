// by ze.chen@hobot.cc

#include "xhttpserver/handler.hpp"

namespace xhttpserver {

Handler::Handler(config_ptr config, router_ptr router)
    : config_(config), router_(router) {
  static_handler_ = std::make_shared<StaticHandler>(config_);
}

void Handler::handle_request(XHttpRequest &req, XHttpResponse &rep) {
  auto filters = router_->get_pre_filters();
  XHttpFilter::FilterResult result;
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  std::chrono::system_clock::time_point step_start;
  double step_cost = 0.0;
  for (auto filter: filters) {
    step_start = std::chrono::system_clock::now();
    result = filter->filter(req, rep);
    step_cost = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - step_start)).count());
    step_cost = step_cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
    req.step_cost_.push_back(std::make_pair("filter-" + filter->get_name(), step_cost));
    if (result == XHttpFilter::stop) {
      req.computer_cost_ = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start)).count());
      req.computer_cost_ = req.computer_cost_ * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
      return;
    }
  }

  auto controls = router_->get_controls();
  bool match = false;
  std::string method;
  std::string uri;
  XHttpControlInterface *controlInterface = nullptr;
  for (auto control: controls) {
    std::tie(method, uri, controlInterface) = control;
    if (req.method_.compare(method) == 0 && req.uri_.compare(uri) == 0) {
      match = true;
      step_start = std::chrono::system_clock::now();
      rep = controlInterface->apply(req);
      step_cost = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - step_start)).count());
      step_cost = step_cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
      req.step_cost_.push_back(std::make_pair("control", step_cost));
      break;
    }
  }

  if (match == false) {
    rep = static_handler_->handle_request(req);
  }

  filters = router_->get_post_filters();
  for (auto filter: filters) {
    step_start = std::chrono::system_clock::now();
    result = filter->filter(req, rep);
    step_cost = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - step_start)).count());
    step_cost = step_cost * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
    req.step_cost_.push_back(std::make_pair(filter->get_name(), step_cost));
    if (result == XHttpFilter::stop) {
      req.computer_cost_ = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start)).count());
      req.computer_cost_ = req.computer_cost_ * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
      return;
    }
  }
  req.computer_cost_ = double((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start)).count());
  req.computer_cost_ = req.computer_cost_ * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
}

} // namespace xhttpserver