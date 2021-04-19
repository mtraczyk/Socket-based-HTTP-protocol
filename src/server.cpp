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

namespace {
  using correlatedServersInfoMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;
  constexpr static uint32_t bufferSize = 2000;
  constexpr static uint32_t queueLength = 5;

  int32_t sock, msgSock;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen;
  ssize_t len;

  void convertToAbsolutePath(std::string &pathString) noexcept {
    using namespace std::filesystem;

    path path(pathString);
    if (path.is_relative()) {
      std::string currentPath = current_path();
      if (!pathString.empty() && pathString[0] != '/') {
        pathString = currentPath + '/' + pathString;
      } else {
        pathString = currentPath + pathString;
      }
    }
  }

  void printRequestInfo(HTTPRequestParser::requestInfo requestInfo) {
    std::cout << "REQUEST TYPE: ";
    if (std::get<0>(requestInfo) == HTTPRequestParser::requestGET) {
      std::cout << "GET" << " ";
    } else if (std::get<0>(requestInfo) == HTTPRequestParser::requestHEAD) {
      std::cout << "HEAD" << " ";
    }
    std::cout << std::endl;

    if (std::get<1>(requestInfo) == HTTPRequestParser::connectionDefault) {
      std::cout << "CONNECTION: DEFAULT" << " ";
    } else if (std::get<1>(requestInfo) == HTTPRequestParser::connectionClose) {
      std::cout << "CONNECTION: CLOSE" << " ";
    }
    std::cout << std::endl;

    std::cout << "RESOURCE PATH: " << std::get<2>(requestInfo) << std::endl;
  }

  bool parseReadInfo(std::string const &buffer, HTTPRequestParser &requestParser) {
    try {
      requestParser.parsePartOfARequest(buffer);

      while (requestParser.isALineParsed()) {
        if (requestParser.hasAnErrorOccurred()) {
          incorrectRequestAnswer();
          return false;
        }

        const auto parsedRequestInfo = requestParser.getFullyParsedRequest();
        if (parsedRequestInfo.first) {
          printRequestInfo(parsedRequestInfo.second);
          correctRequestAnswer();
          requestParser.cleanAfterParsingWholeRequest();
        }

        requestParser.parsePartOfARequest("");
      }
    } catch (std::regex_error const &e) {
      std::cout << "regex_error caught:" << e.what() << std::endl;
      if (e.code() == std::regex_constants::error_brack) {
        std::cout << "The code was error_brack" << std::endl;
      }

      serverErrorAnswer();
      return false;
    }

    return true;
  }

  void contactWithClient(char *buffer, HTTPRequestParser &requestParser,
                         correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
    do {
      len = read(msgSock, buffer, sizeof(buffer));
      if (len < 0) {
        syserr("reading from client socket");
      } else {
        printf("read from socket: %zd bytes: %.*s\n", len, (int) len, buffer);

        if (!parseReadInfo(buffer, requestParser)) {
          return;
        }
      }
      memset(buffer, 0, len);
    } while (len > 0);
  }
}

void start_server(std::string mainCatalog, std::string const &correlatedServers, uint32_t portNum) {
  correlatedServersInfoMap resourcesToAcquireWithCorrelatedServers;
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

  printf("accepting client connections on port %hu\n", ntohs(serverAddress.sin_port));
  HTTPRequestParser requestParser = HTTPRequestParser();
  for (;;) {
    clientAddressLen = sizeof(clientAddress);
    // get client connection from the socket
    msgSock = accept(sock, (struct sockaddr *) &clientAddress, &clientAddressLen);
    if (msgSock < 0) {
      syserr("accept");
    }

    memset(buffer, 0, sizeof(buffer));
    contactWithClient(buffer, requestParser, resourcesToAcquireWithCorrelatedServers);

    printf("ending connection\n");
    if (close(msgSock) < 0) {
      syserr("close");
    }
    requestParser.reset();
  }
}