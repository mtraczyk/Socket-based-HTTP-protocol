#include "server.h"
#include "parsing_functionalities.h"
#include "file_system_functionalities.h"
#include "err.h"
#include <unordered_map>
#include <filesystem>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace {
  constexpr static uint32_t bufferSize = 2000;
  constexpr static uint32_t queueLength = 5;

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
}

void start_server(std::string mainCatalog, std::string const &correlatedServers, uint32_t portNum) {
  std::unordered_map<std::string, std::pair<std::string, std::string>> resourcesToAcquireWithCorrelatedServers;
  getResourcesFromAFile(correlatedServers, resourcesToAcquireWithCorrelatedServers);
  convertToAbsolutePath(mainCatalog);

  int32_t sock, msg_sock;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
  socklen_t client_address_len;

  char buffer[bufferSize];
  ssize_t len, snd_len;

  sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
  if (sock < 0) {
    syserr("socket");
  }
  // after socket() call; we should close(sock) on any execution path;
  // since all execution paths exit immediately, sock would be closed when program terminates

  server_address.sin_family = AF_INET; // IPv4
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  server_address.sin_port = htons(portNum); // listening on port PORT_NUM

  // bind the socket to a concrete address
  if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    syserr("bind");
  }

  // switch to listening (passive open)
  if (listen(sock, queueLength) < 0) {
    syserr("listen");
  }

  printf("accepting client connections on port %hu\n", ntohs(server_address.sin_port));
  for (;;) {
    client_address_len = sizeof(client_address);
    // get client connection from the socket
    msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
    if (msg_sock < 0)
      syserr("accept");
    do {
      len = read(msg_sock, buffer, sizeof(buffer));
      if (len < 0)
        syserr("reading from client socket");
      else {
        printf("read from socket: %zd bytes: %.*s\n", len, (int) len, buffer);
        snd_len = write(msg_sock, buffer, len);
        if (snd_len != len)
          syserr("writing to client socket");
      }
    } while (len > 0);
    printf("ending connection\n");
    if (close(msg_sock) < 0)
      syserr("close");
  }
}