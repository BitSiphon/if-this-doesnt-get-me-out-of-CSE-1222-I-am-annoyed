#pragma once

#include <string>
#include <sys/socket.h>

const std::string DEFAULT_PORT = "8080";
constexpr int MAX_DATA_SIZE = 1024; // bytes

void *get_in_addr(struct sockaddr *sa);

int connect_tcp(std::string addr_string, std::string addr_port);
