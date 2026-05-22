#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

const std::string DEFAULT_PORT = "8080";

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

  // Parse addr
  int status;
  addrinfo hints;
  addrinfo *resolvedinfo;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  const char *addr = (addr_string.empty() ? nullptr : addr_string.c_str());

  status = getaddrinfo(addr, addr_port.c_str(), &hints, &resolvedinfo);
  if (status != 0) {
    std::cout << "Failed to parse address: " << gai_strerror(status) << "\n";
    return EXIT_FAILURE;
  };

  int sockfd = socket(resolvedinfo->ai_family, resolvedinfo->ai_socktype,
                      resolvedinfo->ai_protocol);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket: " << std::strerror(errno) << "\n";
    freeaddrinfo(resolvedinfo);
    return EXIT_FAILURE;
  }

  // Cleanup

  freeaddrinfo(resolvedinfo);
}
