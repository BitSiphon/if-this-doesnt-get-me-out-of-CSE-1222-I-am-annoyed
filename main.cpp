#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

const std::string DEFAULT_PORT = "8080";
const int MAX_DATA_SIZE = 1024; // bytes

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

int main(int argc, char *argv[]) {
  // Parse Args
  std::string addr_string;
  std::string addr_port = DEFAULT_PORT;
  for (int i = 1; i < argc; i++) {

    std::string argument = argv[i];
    if (argument == "-h") {
      std::cout << "Usage: " << argv[0] << " [-c address] [-p port]\n";
      return EXIT_SUCCESS;
    }
    if (argument == "-c") {
      if (i + 1 < argc) {
        addr_string = argv[i + 1];
        i++;
      } else {
        std::cout << "Argument '-c' called, but no address specified\n";
        return EXIT_FAILURE;
      }
    } else if (argument == "-p") {
      if (i + 1 < argc) {
        addr_port = argv[i + 1];
        i++;
      } else {
        std::cout << "Argument '-p' called, but no port specified\n";
        return EXIT_FAILURE;
      }
    }
  }

  // Establish connection
  int sockfd = connect_tcp(addr_string, addr_port);
  if (sockfd == -1) {
    std::cout << "Failed to connect to " << addr_string << ":" << addr_port
              << "\n"
              << std::strerror(errno) << "\n";
  }

  std::string hello_str = "GET /headers HTTP/1.1\r\n"
                          "Host: localhost:8080\r\n"
                          "Connection: close\r\n"
                          "\r\n";
  int bytes_sent = send(sockfd, hello_str.c_str(), hello_str.length(), 0);

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
