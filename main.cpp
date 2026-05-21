#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char* argv[]) {

    // Parse Args
    std::string addr_string;
    for (int i=0 ; i<argc ; i++) {

        std::string argument = argv[i];
        if (argument == "-h") {
            std::cout << "HELP\n";
            continue;
        }
        if (argument == "-c") {
            if (i+1 < argc) {
                addr_string = argv[i+1];
                i++;
                continue;
            } else {
                std::cout << "Argument '-c' called, but no address specified";
                exit(-1);
            }
        }
    }

    // Check defaults
    if (addr_string == "") {
        std::cout << "No address specified";
        exit(-1);
    }


    int status;
    addrinfo hints;
    addrinfo *serverinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(addr_string.c_str(), "443", &hints, &serverinfo);
    if (status != 0) {
        std::cout << "Failed to parse address.\n\t" << gai_strerror(status) << "\n";
        exit(-1);
    };


    freeaddrinfo(serverinfo);
}
