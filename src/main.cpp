#include "server.h"
#include "file_system_functionalities.h"

int main(int argc, char **argv) {
  if (checkWhetherGivenPathExists(argv[1]) && checkWhetherGivenPathExists(argv[2])
      && checkWhetherAccessToAPathIsAcquired(argv[1]) && checkWhetherAccessToAPathIsAcquired(argv[2])) {
    start_server();
  } else {
    exit(EXIT_FAILURE);
  }

  return 0;
}
