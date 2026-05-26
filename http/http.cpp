#include "http.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

/*
 * Private Helper Functions
 */
namespace {
// get sockaddr, IPv4 or IPv6
void *get_in_addr(sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Converts a sockaddr into a human-readable IP string
std::string to_string(const addrinfo *p) {
    if (!p)
        return "Unknown Address";

    char parsed_str[INET6_ADDRSTRLEN];
    if (inet_ntop(p->ai_family, get_in_addr(p->ai_addr), parsed_str, sizeof(parsed_str))) {
        return std::string(parsed_str);
    }
    return "Invalid Address";
}

// Create standardized HTTP 1.1 header
std::string create_head(
    std::string start_line, std::optional<std::unordered_map<std::string, std::string>> headers) {
    std::string result{start_line + "\r\n"};

    if (headers) {
        for (const auto &[key, value] : headers.value()) {
            result += key + ": " + value + "\r\n";
        }
    }

    return result + "\r\n";
}
} // namespace

namespace http {
/*
 * HTTP Parsing
 */

/// Creates a request HTTP header, copying the optional headers for safe re-use.
std::string create_client_head(
    HTTP_METHOD method,
    std::string_view host,
    std::string_view path,
    const std::optional<std::unordered_map<std::string, std::string>> &headers) {
    auto updated_headers = headers.value_or(std::unordered_map<std::string, std::string>{});

    updated_headers["Host"] = std::string(host);

    return create_head(std::string(to_string(method)) + " " + std::string(path), updated_headers);
}

/// Creates a request HTTP header, efficiently consuming/moving the optional
/// headers.
std::string create_client_head(
    HTTP_METHOD method,
    std::string_view host,
    std::string_view path,
    std::optional<std::unordered_map<std::string, std::string>> &&headers) {
    auto updated_headers =
        std::move(headers).value_or(std::unordered_map<std::string, std::string>{});

    updated_headers["Host"] = std::string(host);

    return create_head(
        std::string(to_string(method)) + " " + std::string(path), std::move(updated_headers));
}

/*
 * Core Functions
 */

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
    };

    // loop through all the results and connect to the first we can
    for (p = resolvedinfo; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        // Using the new helper function here
        std::cout << "Attempting to connect: " << to_string(p) << "\n";

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == nullptr) {
        freeaddrinfo(resolvedinfo); // Make sure to clean up if loop fails completely
        return -1;
    }

    // Using the new helper function here
    std::cout << "Established connection to " << to_string(p) << "\n";

    // Cleanup
    freeaddrinfo(resolvedinfo);

    return sockfd;
}
} // namespace http
