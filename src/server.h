#ifndef SERWER_H
#define SERWER_H

#include <string>

constexpr static uint32_t standardPortNum = 8080;

void start_server(std::string, std::string const &, uint32_t portNum = standardPortNum);

#endif /* SERWER_H */
