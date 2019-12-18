// by ze.chen@hobot.cc

#ifndef X_HTTP_SERVER_REQUEST_HEADER_PARSER_HPP
#define X_HTTP_SERVER_REQUEST_HEADER_PARSER_HPP

#include <tuple>
#include <string>

#include "xhttpserver/request.hpp"

namespace xhttpserver {

class XHttpRequest;

// Parser for incoming requests.
class HeaderParser {
 public:
  // Construct ready to parse the XHttpRequest method.
  HeaderParser();

  // Reset to initial parser state.
  void reset();

  // Result of parse.
  enum result_type { good, bad, indeterminate };

  // Parse some data. The enum return value is good when a complete XHttpRequest has
  // been parsed, bad if the data is invalid, indeterminate when more data is
  // required. The InputIterator return value indicates how much of the input
  // has been consumed.
  template<typename InputIterator>
  std::tuple<result_type, InputIterator> parse(XHttpRequest &req, InputIterator begin, InputIterator end) {
    while (begin != end) {
      result_type result = consume(req, *begin++);
      if (result == good) {
        std::for_each(req.headers_.begin(), req.headers_.end(), [&result, &req](Header &header) {
          std::transform(header.name.begin(),header.name.end(),header.name.begin(),::toupper);
          if (header.name.compare("CONTENT-LENGTH") == 0 ) {
            sscanf(header.value.c_str(), "%zu", &(req.content_length_));
          }
          if (header.name.compare("Connection") == 0) {
            req.keep_alive_ = (header.value.compare("keep-alive") == 0);
          }
        });
      }
      if (result == good || result == bad)
        return std::make_tuple(result, begin);
    }
    return std::make_tuple(indeterminate, begin);
  };

 private:
  // Handle the next character of input.
  result_type consume(XHttpRequest &req, char input);

  // URI decode
  std::string uri_decode(const std::string &value);

  // Build Query String Map
  void parse_query_string(XHttpRequest &req);

  // Check if a byte is an HTTP character.
  static bool is_char(int c);

  // Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  // Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  // Check if a byte is a digit.
  static bool is_digit(int c);

  // Check if method is GET/POST/HEAD
  static bool valid_method(const std::string &method);

  // The current state of the parser.
  enum state {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
  } state_;
};

} // namespace xhttpserver

#endif // X_HTTP_SERVER_REQUEST_HEADER_PARSER_HPP
