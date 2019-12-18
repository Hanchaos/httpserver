// by ze.chen@hobot.cc

#include "xhttpserver/header_parser.hpp"

#include <fstream>

namespace xhttpserver {
HeaderParser::HeaderParser()
    : state_(method_start) {
}

void HeaderParser::reset() {
  state_ = method_start;
}

HeaderParser::result_type HeaderParser::consume(XHttpRequest &req, char input) {
  switch (state_) {
    case method_start:
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        state_ = method;
        req.method_.push_back(tolower(input));
        return indeterminate;
      }
    case method:
      if (input == ' ') {
        // Check method if support
        if (valid_method(req.method_)) {
          state_ = uri;
          return indeterminate;
        } else {
          return bad;
        }
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        req.method_.push_back(tolower(input));
        return indeterminate;
      }
    case uri:
      if (input == ' ') {
        state_ = http_version_h;
        // parse query_uri and query_string
        std::string req_uri = req.uri_;
        //
        std::size_t query_string_start = req_uri.find("?");
        if(query_string_start != std::string::npos && query_string_start != req_uri.length() - 1) {
          req.uri_ = req_uri.substr(0, query_string_start);
          req.query_string_ = req_uri.substr(query_string_start + 1);
          parse_query_string(req);
        }
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        req.uri_.push_back(input);
        return indeterminate;
      }
    case http_version_h:
      if (input == 'H') {
        state_ = http_version_t_1;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_t_1:
      if (input == 'T') {
        state_ = http_version_t_2;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_t_2:
      if (input == 'T') {
        state_ = http_version_p;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_p:
      if (input == 'P') {
        state_ = http_version_slash;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_slash:
      if (input == '/') {
        req.http_version_major_ = 0;
        req.http_version_minor_ = 0;
        state_ = http_version_major_start;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_major_start:
      if (is_digit(input)) {
        req.http_version_major_ = req.http_version_major_ * 10 + input - '0';
        state_ = http_version_major;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_major:
      if (input == '.') {
        state_ = http_version_minor_start;
        return indeterminate;
      } else if (is_digit(input)) {
        req.http_version_major_ = req.http_version_major_ * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_minor_start:
      if (is_digit(input)) {
        req.http_version_minor_ = req.http_version_minor_ * 10 + input - '0';
        state_ = http_version_minor;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_minor:
      if (input == '\r') {
        state_ = expecting_newline_1;
        return indeterminate;
      } else if (is_digit(input)) {
        req.http_version_minor_ = req.http_version_minor_ * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case expecting_newline_1:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    case header_line_start:
      if (input == '\r') {
        state_ = expecting_newline_3;
        return indeterminate;
      } else if (!req.headers_.empty() && (input == ' ' || input == '\t')) {
        state_ = header_lws;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        req.headers_.push_back(Header());
        req.headers_.back().name.push_back(input);
        state_ = header_name;
        return indeterminate;
      }
    case header_lws:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      } else if (input == ' ' || input == '\t') {
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        state_ = header_value;
        req.headers_.back().value.push_back(input);
        return indeterminate;
      }
    case header_name:
      if (input == ':') {
        state_ = space_before_header_value;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        req.headers_.back().name.push_back(input);
        return indeterminate;
      }
    case space_before_header_value:
      if (input == ' ') {
        state_ = header_value;
        return indeterminate;
      } else {
        return bad;
      }
    case header_value:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        req.headers_.back().value.push_back(input);
        return indeterminate;
      }
    case expecting_newline_2:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    case expecting_newline_3:
      if (input == '\n') {
        return good;
      } else {
        return bad;
      }
    default:return bad;
  }
}

void HeaderParser::parse_query_string(XHttpRequest &req) {
  if(req.query_string_.empty()) {
    return;
  }
  std::string query_string = req.query_string_;
  std::size_t name_pos = 0;
  auto name_end_pos = std::string::npos;
  auto value_pos = std::string::npos;
  for(std::size_t c = 0; c < query_string.size(); ++c) {
    if(query_string[c] == '&') {
      auto name = query_string.substr(name_pos, (name_end_pos == std::string::npos ? c : name_end_pos) - name_pos);
      if(!name.empty()) {
        auto value = value_pos == std::string::npos ? std::string() : query_string.substr(value_pos, c - value_pos);
        req.query_string_map_.emplace(std::move(name), uri_decode(value));
      }
      name_pos = c + 1;
      name_end_pos = std::string::npos;
      value_pos = std::string::npos;
    } else if(query_string[c] == '=') {
      name_end_pos = c;
      value_pos = c + 1;
    }
  }
  if(name_pos < query_string.size()) {
    auto name = query_string.substr(name_pos, name_end_pos - name_pos);
    if(!name.empty()) {
      auto value = value_pos >= query_string.size() ? std::string() : query_string.substr(value_pos);
      req.query_string_map_.emplace(std::move(name), uri_decode(value));
    }
  }
}

std::string HeaderParser::uri_decode(const std::string &value) {
  std::string result;
  result.reserve(value.size() / 3 + (value.size() % 3)); // Minimum size of result
  for(std::size_t i = 0; i < value.size(); ++i) {
    auto &chr = value[i];
    if(chr == '%' && i + 2 < value.size()) {
      auto hex = value.substr(i + 1, 2);
      auto decoded_chr = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
      result += decoded_chr;
      i += 2;
    } else if(chr == '+')
      result += ' ';
    else
      result += chr;
  }
  return result;
}

bool HeaderParser::is_char(int c) {
  return c >= 0 && c <= 127;
}

bool HeaderParser::is_ctl(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}

bool HeaderParser::is_tspecial(int c) {
  switch (c) {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case ' ':
    case '\t':return true;
    default:return false;
  }
}

bool HeaderParser::is_digit(int c) {
  return c >= '0' && c <= '9';
}

bool HeaderParser::valid_method(const std::string &method) {
  return method.compare("get") == 0 || method.compare("post") == 0 \
 || method.compare("head") == 0;
}

} // namespace xhttpserver
