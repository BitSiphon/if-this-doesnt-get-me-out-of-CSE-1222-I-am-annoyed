#pragma once

#include <string>
#include <string_view>
#include <sys/socket.h>

/*
 * Constants
 */

const std::string DEFAULT_PORT = "8080";
constexpr int MAX_DATA_SIZE = 1024; // bytes

// Standard HTTP methods
//
// SECURITY: TRACE should be avoided when implementing server.
enum HTTP_METHOD {
  GET,
  HEAD,
  OPTIONS,
  TRACE,
  DELETE,
  PUT,
  POST,
  CONNECT,
};

constexpr std::string_view to_string(HTTP_METHOD method) {
  switch (method) {
  case GET:
    return "GET";
  case HEAD:
    return "HEAD";
  case OPTIONS:
    return "OPTIONS";
  case TRACE:
    return "TRACE";
  case DELETE:
    return "DELETE";
  case PUT:
    return "PUT";
  case POST:
    return "POST";
  case CONNECT:
    return "CONNECT";
  default:
    return "UNKNOWN";
  }
}

namespace http {
/*
 * HTTP Parsing
 */

/*
 * Core Functions
 */

int connect_tcp(std::string addr_string, std::string addr_port);
} // namespace http
