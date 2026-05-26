#include "http.h"
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

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
    int sockfd = http::connect_tcp(addr_string, addr_port);
    if (sockfd == -1) {
        std::cout << "Failed to connect to " << addr_string << ":" << addr_port << "\n"
                  << std::strerror(errno) << "\n";
    }

    std::string hello_str = "GET /headers HTTP/1.1\r\n"
                            "Host: localhost:8080\r\n"
                            "Connection: close\r\n"
                            "\r\n";
    int bytes_sent = send(sockfd, hello_str.c_str(), hello_str.length(), 0);
    std::cout << "Sent " << bytes_sent << " bytes\n";

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
