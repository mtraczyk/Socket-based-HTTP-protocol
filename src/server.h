#ifndef SERWER_H
#define SERWER_H

#include <string>

constexpr static uint32_t standardPortNum = 8080;

/*
 * First string is a files' catalog directory, the second is a correlated servers file,
 * the third one is a port number of the server.
 */
void startServer(std::string, std::string const &, uint32_t portNum = standardPortNum);

#endif /* SERWER_H */
