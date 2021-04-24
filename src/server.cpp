#include "server.h"
#include "parsing_functionalities.h"
#include "file_system_functionalities.h"
#include "request_answer.h"
#include "err.h"
#include <unordered_map>
#include <filesystem>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace {
  constexpr static uint32_t bufferSize = 4096;
  constexpr static uint32_t queueLength = 5;

  int32_t sock, msgSock;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen;
  ssize_t len;

  bool parseReadInfo(std::string const &buffer, HTTPRequestParser &requestParser,
                     std::string const &mainCatalogAbsolutePath,
                     requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
    try {
      // Parse part of a request.
      requestParser.parsePartOfARequest(buffer);

      while (requestParser.isALineParsed()) {
        // Parse lines as long as there is a need to do that.
        if (requestParser.hasAnErrorOccurred()) {
          incorrectRequestAnswer(msgSock, requestParser.getErrorType());
          return false;
        }

        const auto parsedRequestInfo = requestParser.getFullyParsedRequest();
        if (parsedRequestInfo.first) {
          if (!correctRequestAnswer(msgSock, mainCatalogAbsolutePath, parsedRequestInfo.second,
                                    resourcesToAcquireWithCorrelatedServers)) {
            return false;
          }
          requestParser.cleanAfterParsingWholeRequest();
        }

        /* With one part of a request, a lot of lines could have been read.
         * All of them need to be processed, that is why the function is being invoked with an empty argument.
         */
        requestParser.parsePartOfARequest("");
      }
    } catch (std::regex_error const &e) {
      // Regex may throw errors. They need to be caught.
      std::cerr << "regex_error caught:" << e.what() << std::endl;
      serverErrorAnswer(msgSock);

      return false;
    }

    return true;
  }

  bool contactWithClient(char *buffer, HTTPRequestParser &requestParser, std::string const &mainCatalogAbsolutePath,
                         requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
    do {
      len = read(msgSock, buffer, sizeof(buffer));
      if (len <= 0) {
        // The connection was finished by the client or an error connected with read function occurred.
        return false;
      } else {
        if (!parseReadInfo(buffer, requestParser, mainCatalogAbsolutePath, resourcesToAcquireWithCorrelatedServers)) {
          // An error occurred that implies the need of closing the connection.
          return true;
        }
      }
      memset(buffer, 0, len);
    } while (len > 0);

    return false;
  }
}

void startServer(std::string mainCatalog, std::string const &correlatedServers, uint32_t portNum) {
  // Map used in order to cache data from correlated servers's file.
  requestData::correlatedServersInfoMap resourcesToAcquireWithCorrelatedServers;
  // Caches info from correlated servers.
  getResourcesFromAFile(correlatedServers, resourcesToAcquireWithCorrelatedServers);
  convertToAbsolutePath(mainCatalog);

  char buffer[bufferSize];
  sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
  if (sock < 0) {
    syserr("socket");
  }
  // after socket() call; we should close(sock) on any execution path;
  // since all execution paths exit immediately, sock would be closed when program terminates

  serverAddress.sin_family = AF_INET; // IPv4
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  serverAddress.sin_port = htons(portNum); // listening on port PORT_NUM

  // bind the socket to a concrete address
  if (bind(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    syserr("bind");
  }

  // switch to listening (passive open)
  if (listen(sock, queueLength) < 0) {
    syserr("listen");
  }

  std::cout << "accepting client connections on port: " << ntohs(serverAddress.sin_port) << std::endl << std::endl;
  HTTPRequestParser requestParser = HTTPRequestParser();
  for (;;) {
    clientAddressLen = sizeof(clientAddress);
    // get client connection from the socket
    msgSock = accept(sock, (struct sockaddr *) &clientAddress, &clientAddressLen);
    if (msgSock < 0) {
      // There is no need to shut down the server, as a result of a connection failure with a certain client.
      requestParser.reset();
      continue;
    }

    std::cout << "starting connection" << std::endl;
    std::cout << "IP address is: " << inet_ntoa(clientAddress.sin_addr) << std::endl;
    std::cout << "port is: " << (size_t) ntohs(clientAddress.sin_port) << std::endl << std::endl;
    memset(buffer, 0, sizeof(buffer));

    if (contactWithClient(buffer, requestParser, mainCatalog, resourcesToAcquireWithCorrelatedServers)) {
      void(close(msgSock));
    }

    std::cout << "ending connection" << std::endl << std::endl;
    requestParser.reset();
  }
}