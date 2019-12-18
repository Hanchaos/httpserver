// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_RESPONSE_HPP
#define X_HTTP_SERVER_RESPONSE_HPP

#include <string>
#include <vector>

#include "boost/asio.hpp"

#include "xhttpserver/header.hpp"

namespace xhttpserver {

const char http_name_value_separator[] = {':', ' '};
const char http_crlf[] = {'\r', '\n'};

namespace http_stock_content {
const char ok[] = "";
const char created[] =
    "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
const char accepted[] =
    "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
const char no_content[] =
    "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
const char multiple_choices[] =
    "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
const char moved_permanently[] =
    "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
const char moved_temporarily[] =
    "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
const char not_modified[] =
    "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
const char bad_request[] =
    "<html>"
        "<head><title>Bad XHttpRequest</title></head>"
        "<body><h1>400 Bad XHttpRequest</h1></body>"
        "</html>";
const char unauthorized[] =
    "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
const char forbidden[] =
    "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
const char not_found[] =
    "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
const char internal_server_error[] =
    "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
const char not_implemented[] =
    "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
const char bad_gateway[] =
    "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
const char service_unavailable[] =
    "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";
} // namespace http_stock_content

namespace http_stock_header {
const std::string ok = "HTTP/1.0 200 OK\r\n";
const std::string created = "HTTP/1.0 201 Created\r\n";
const std::string accepted = "HTTP/1.0 202 Accepted\r\n";
const std::string no_content = "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices = "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently = "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified = "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request = "HTTP/1.0 400 Bad XHttpRequest\r\n";
const std::string unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden = "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found = "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented = "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway = "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable = "HTTP/1.0 503 Service Unavailable\r\n";
} // namespace http_stock_header

// Response  API
// A XHttpResponse to be sent to a client.
class XHttpResponse {
  friend class Connection;
 public:
  enum http_code {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  };
 private:
  // Http code to stock content
  static std::string http_stock_content(http_code status) {
    switch (status) {
      case http_code::ok:return http_stock_content::ok;
      case http_code::created:return http_stock_content::created;
      case http_code::accepted:return http_stock_content::accepted;
      case http_code::no_content:return http_stock_content::no_content;
      case http_code::multiple_choices:return http_stock_content::multiple_choices;
      case http_code::moved_permanently:return http_stock_content::moved_permanently;
      case http_code::moved_temporarily:return http_stock_content::moved_temporarily;
      case http_code::not_modified:return http_stock_content::not_modified;
      case http_code::bad_request:return http_stock_content::bad_request;
      case http_code::unauthorized:return http_stock_content::unauthorized;
      case http_code::forbidden:return http_stock_content::forbidden;
      case http_code::not_found:return http_stock_content::not_found;
      case http_code::internal_server_error:return http_stock_content::internal_server_error;
      case http_code::not_implemented:return http_stock_content::not_implemented;
      case http_code::bad_gateway:return http_stock_content::bad_gateway;
      case http_code::service_unavailable:return http_stock_content::service_unavailable;
      default:return http_stock_content::internal_server_error;
    }
  }
  // Http code to stock header
  static boost::asio::const_buffer http_stock_header(http_code status) {
    switch (status) {
      case http_code::ok:return boost::asio::buffer(http_stock_header::ok);
      case http_code::created:return boost::asio::buffer(http_stock_header::created);
      case http_code::accepted:return boost::asio::buffer(http_stock_header::accepted);
      case http_code::no_content:return boost::asio::buffer(http_stock_header::no_content);
      case http_code::multiple_choices:return boost::asio::buffer(http_stock_header::multiple_choices);
      case http_code::moved_permanently:return boost::asio::buffer(http_stock_header::moved_permanently);
      case http_code::moved_temporarily:return boost::asio::buffer(http_stock_header::moved_temporarily);
      case http_code::not_modified:return boost::asio::buffer(http_stock_header::not_modified);
      case http_code::bad_request:return boost::asio::buffer(http_stock_header::bad_request);
      case http_code::unauthorized:return boost::asio::buffer(http_stock_header::unauthorized);
      case http_code::forbidden:return boost::asio::buffer(http_stock_header::forbidden);
      case http_code::not_found:return boost::asio::buffer(http_stock_header::not_found);
      case http_code::internal_server_error:return boost::asio::buffer(http_stock_header::internal_server_error);
      case http_code::not_implemented:return boost::asio::buffer(http_stock_header::not_implemented);
      case http_code::bad_gateway:return boost::asio::buffer(http_stock_header::bad_gateway);
      case http_code::service_unavailable:return boost::asio::buffer(http_stock_header::service_unavailable);
      default:return boost::asio::buffer(http_stock_header::internal_server_error);
    }
  }

 public:
  static XHttpResponse error(http_code status) {
    XHttpResponse rep;
    rep.status_ = status;
    rep.content_ = http_stock_content(status);
    rep.header("Content-Length", std::to_string(rep.content_.size()));
    rep.header("Content-Type", "text/html");
    return rep;
  }

  static XHttpResponse error(http_code status, std::string content, std::string extension = "text/html") {
    XHttpResponse rep;
    rep.status_ = status;
    rep.content_ = content;
    rep.header("Content-Length", std::to_string(rep.content_.size()));
    rep.header("Content-Type", extension);
    return rep;
  }

  static XHttpResponse success(std::string content, std::string extension = "text/html") {
    XHttpResponse rep;
    rep.status_ = http_code::ok;
    rep.content_ = content;
    rep.header("Content-Length", std::to_string(rep.content_.size()));
    rep.header("Content-Type", extension);
    return rep;
  }

  static XHttpResponse redirect(std::string url) {
    XHttpResponse rep;
    rep.status_ = http_code::moved_temporarily;
    rep.header("Location", url);
    return rep;
  }

  XHttpResponse &header(std::string key, std::string value) {
    headers_.push_back(Header{key, value});
    return *this;
  }

 private:
  // The status of the XHttpResponse.
  http_code status_ = http_code::ok;

  // The headers to be included in the XHttpResponse.
  std::vector<Header> headers_;

  // The content to be sent in the XHttpResponse.
  std::string content_;

  // Convert the XHttpResponse into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the XHttpResponse object must remain valid and
  // not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> to_buffers() {
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(http_stock_header(status_));
    for (std::size_t i = 0; i < headers_.size(); ++i) {
      Header &h = headers_[i];
      buffers.push_back(boost::asio::buffer(h.name));
      buffers.push_back(boost::asio::buffer(http_name_value_separator));
      buffers.push_back(boost::asio::buffer(h.value));
      buffers.push_back(boost::asio::buffer(http_crlf));
    }
    buffers.push_back(boost::asio::buffer(http_crlf));
    buffers.push_back(boost::asio::buffer(content_));
    return buffers;
  }
};
} // namespace xhttpserver

#endif // X_HTTP_SERVER_RESPONSE_HPP
