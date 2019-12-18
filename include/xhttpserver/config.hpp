// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_CONFIG_H
#define X_HTTP_SERVER_CONFIG_H

#include <string>
#include <memory>

#include "glog/logging.h"
#include "xhttpserver/server_discovery.hpp"
namespace xhttpserver {

// Config object just using by framework.
class Config {
 public:
  const std::string &get_request_id_header_name() const {
    return request_id_header_name_;
  }
  int get_keep_alive_timeout() const {
    return keep_alive_timeout_;
  }
  int get_request_read_timeout() const {
    return request_read_timeout_;
  }
  int get_request_content_read_timeout() const {
    return request_content_read_timeout_;
  }
  int get_max_connection_num() const {
    return max_connection_num_;
  }
  int get_max_request_size_bytes() const {
    return max_request_size_bytes_;
  }
  const std::string &get_doc_root() const {
    return doc_root_;
  }
  bool get_close_connection_after_response() const {
    return close_connection_after_response_;
  }
  int get_response_send_timeout() const {
    return response_send_timeout_;
  }

  //
  std::string get_zookeeper_url() const {
    return zookeeper_url_;
  }
  std::string get_zookeeper_root() const {
    return zookeeper_root_;
  }
  int get_zookeeper_timeout() const {
    return zookeeper_timeout_;
  }
  std::string get_server_zookeeper_data() const {
    return server_zookeeper_data_;
  }
  ServerType get_server_zookeeper_type() const {
    return server_zookeeper_type_;
  }


 public:
  void set_request_id_header_name(const std::string &request_id_header_name) {
    request_id_header_name_ = request_id_header_name;
  }
  void set_keep_alive_timeout(int keep_alive_timeout) {
    keep_alive_timeout_ = keep_alive_timeout;
  }
  void set_request_read_timeout(int request_read_timeout) {
    request_read_timeout_ = request_read_timeout;
  }
  void set_request_content_read_timeout(int request_content_read_timeout) {
    request_content_read_timeout_ = request_content_read_timeout;
  }
  void set_response_send_timeout(int response_send_timeout) {
    response_send_timeout_ = response_send_timeout;
  }
  void set_max_connection_num(int max_connection_num) {
    max_connection_num_ = max_connection_num;
  }
  void set_max_request_size_bytes(int max_request_size_bytes) {
    max_request_size_bytes_ = max_request_size_bytes;
  }
  void set_doc_root(const std::string &doc_root) {
    doc_root_ = doc_root;
  }
  void set_close_connection_after_response(bool close_connection_after_response) {
    close_connection_after_response_ = close_connection_after_response;
  }
  //server discovery
  void set_zookeeper_root(const std::string zookeeper_root){
    zookeeper_root_ = zookeeper_root;
  }
  void set_zookeeper_url(const std::string zookeeper_url){
    zookeeper_url_ = zookeeper_url;
  }
  void set_zookeeper_timeout(const int zookeeper_timeout){
    zookeeper_timeout_ = zookeeper_timeout;
  }
  void set_server_zookeeper_data(const std::string server_zookeeper_data){
    server_zookeeper_data_ = server_zookeeper_data;
  }
  void set_server_zookeeper_type(const ServerType server_zookeeper_type){
    server_zookeeper_type_ = server_zookeeper_type;
  }
 private:
  std::string request_id_header_name_;
  std::string doc_root_;
  //
  std::string zookeeper_url_;
  std::string zookeeper_root_;
  int         zookeeper_timeout_;
  std::string server_zookeeper_data_;
  ServerType  server_zookeeper_type_;

  int keep_alive_timeout_;
  int request_read_timeout_;
  int request_content_read_timeout_;
  int response_send_timeout_;
  int max_connection_num_;
  int max_request_size_bytes_;
  bool close_connection_after_response_;
};
typedef std::shared_ptr<Config> config_ptr;
}
#endif //X_HTTP_SERVER_CONFIG_H
