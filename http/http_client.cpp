#include "http.h"
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

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

    // TODO: create a response class
    http::get(addr_string, addr_port);
}
