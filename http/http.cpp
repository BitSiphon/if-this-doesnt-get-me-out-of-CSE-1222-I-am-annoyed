#include "http.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

// =============================================================================
// Private Helper Functions (Internal Linkage)
// =============================================================================
namespace {

// get sockaddr, IPv4 or IPv6
void *get_in_addr(sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in *)sa)->sin_addr);

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Converts a sockaddr into a human-readable IP string
std::string to_string(const addrinfo *p) {
  if (!p)
    return "Unknown Address";

  char parsed_str[INET6_ADDRSTRLEN];
  if (inet_ntop(
          p->ai_family,
          get_in_addr(p->ai_addr),
          parsed_str,
          sizeof(parsed_str))) {
    return std::string(parsed_str);
  }
  return "Invalid Address";
}

// Create standardized HTTP 1.1 header
std::string build_message_header(
    std::string start_line,
    std::optional<std::unordered_map<std::string, std::string>> headers) {
  std::string result{start_line + "\r\n"};

  if (headers) {
    for (const auto &[key, value] : headers.value())
      result += key + ": " + value + "\r\n";
  }

  return result + "\r\n";
}

// Parse standardized HTTP 1.1 header
http::HTTPResponse parse_message_header(std::string_view msg) {
  std::string status_line;
  http::header_map headers{};

  size_t pos;
  size_t prev_pos{0};
  bool is_first_line = true;

  while ((pos = msg.find("\r\n", prev_pos)) != std::string_view::npos) {
    std::string_view line = msg.substr(prev_pos, pos - prev_pos);

    if (line.empty())
      break;

    if (is_first_line) {
      is_first_line = false;
      // TEMPORARY: This code only handles 1.1
      assert(line.substr(0, 8) == "HTTP/1.1");
      status_line = std::string(line);
    }

    size_t end_key_index = line.find(":");
    std::string_view key = line.substr(0, end_key_index);
    std::string_view value = line.substr(end_key_index + 1, line.length());

    headers[std::string(key)] = std::string(value);
    prev_pos = pos + 2;
  }

  return http::HTTPResponse(status_line, headers);
}

} // namespace

// =============================================================================
// Public HTTP Namespace Implementation
// =============================================================================
namespace http {

// -----------------------------------------------------------------------------
// HTTPResponse Class Methods
// -----------------------------------------------------------------------------
HTTPResponse::HTTPResponse(
    const std::string &status, const http::header_map &headers)
    : status(status), headers(headers) {
  size_t first_space_idx = status.find(" ");

  if (first_space_idx != std::string::npos) {
    std::string sub = status.substr(first_space_idx + 1, 3);
    status_code = std::stoi(sub);
  } else {
    status_code = 0; // malformed
  }
}

// -----------------------------------------------------------------------------
// HTTP Parsing & Header Generation
// -----------------------------------------------------------------------------

/// Creates a request HTTP header, copying the optional headers for safe re-use.
std::string create_request_message_header(
    HTTP_METHOD method,
    std::string_view host,
    std::string_view path,
    const std::optional<std::unordered_map<std::string, std::string>>
        &headers) {
  auto updated_headers =
      headers.value_or(std::unordered_map<std::string, std::string>{});

  updated_headers["Host"] = std::string(host);

  std::string start_line =
      std::string(to_string(method)) + " " + std::string(path) + " HTTP/1.1";
  return build_message_header(std::move(start_line), updated_headers);
}

// -----------------------------------------------------------------------------
// Network Core Functions
// -----------------------------------------------------------------------------

// Returns established socket file descriptor. Returns -1 on error.
int connect_tcp(std::string addr_string, std::string addr_port) {
  int sockfd;
  int result;
  addrinfo hints;
  addrinfo *resolvedinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // empty address use loopback (caused by nullptr)
  const char *addr = (addr_string.empty() ? nullptr : addr_string.c_str());

  result = getaddrinfo(addr, addr_port.c_str(), &hints, &resolvedinfo);
  if (result != 0) {
    std::cout << "Failed to parse address: " << gai_strerror(result) << "\n";
    return -1;
  }

  // loop through all the results and connect to the first we can
  for (p = resolvedinfo; p != nullptr; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1)
      continue;

    std::cout << "Attempting to connect: " << to_string(p) << "\n";

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == nullptr) {
    freeaddrinfo(
        resolvedinfo); // Make sure to clean up if loop fails completely
    return -1;
  }

  std::cout << "Established connection to " << to_string(p) << "\n";
  freeaddrinfo(resolvedinfo);

  return sockfd;
}

void get(std::string addr_string, std::string addr_port) {
  // Establish connection
  int sockfd = http::connect_tcp(addr_string, addr_port);
  if (sockfd == -1) {
    std::cout << "Failed to connect to " << addr_string << ":" << addr_port
              << "\n"
              << std::strerror(errno) << "\n";
  }

  std::unordered_map<std::string, std::string> headers{{"Connection", "close"}};
  std::string message_header = http::create_request_message_header(
      HTTP_METHOD::GET, addr_string + ":" + addr_port, "/headers", headers);

  int bytes_sent =
      send(sockfd, message_header.c_str(), message_header.length(), 0);
  std::cout << "Sent " << bytes_sent << " bytes\n";

  // buffer read header
  int numbytes;
  char buf[MAX_DATA_SIZE];
  if ((numbytes = recv(sockfd, buf, MAX_DATA_SIZE - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';
  std::cout << "Received " << buf << "\n";

  close(sockfd);
}

} // namespace http
