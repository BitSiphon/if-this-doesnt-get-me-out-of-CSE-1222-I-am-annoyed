#pragma once

#include <stdexcept>
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
enum class HTTP_METHOD {
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
    case HTTP_METHOD::GET:
        return "GET";
    case HTTP_METHOD::HEAD:
        return "HEAD";
    case HTTP_METHOD::OPTIONS:
        return "OPTIONS";
    case HTTP_METHOD::TRACE:
        return "TRACE";
    case HTTP_METHOD::DELETE:
        return "DELETE";
    case HTTP_METHOD::PUT:
        return "PUT";
    case HTTP_METHOD::POST:
        return "POST";
    case HTTP_METHOD::CONNECT:
        return "CONNECT";
    }
    throw std::invalid_argument("Unknown HTTP method");
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
