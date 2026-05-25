#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Returns established socket file descriptor or -1 on error.
int connect_tcp(std::string addr_string, std::string addr_port) {
  // Parse addr

  int sockfd;
  int result;
  addrinfo hints;
  addrinfo *resolvedinfo, *p;
  char parsed_str[INET6_ADDRSTRLEN];

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

    inet_ntop(p->ai_family, get_in_addr(p->ai_addr), parsed_str,
              sizeof parsed_str);
    std::cout << "Attempting to connect: " << parsed_str << "\n";

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == nullptr) {
    return -1;
  }

  inet_ntop(p->ai_family, get_in_addr(p->ai_addr), parsed_str,
            sizeof parsed_str);
  std::cout << "Established connection to " << parsed_str << "\n";

  // Cleanup
  freeaddrinfo(resolvedinfo);

  return sockfd;
}
