#include "server.h"
#include "file_system_functionalities.h"
#include <unistd.h>

constexpr static uint32_t decimalBasis = 10;

int main(int argc, char **argv) {
  if (argc < 3) {
    exit(EXIT_FAILURE);
  }

  if (checkWhetherGivenPathExists(argv[1]) && checkWhetherGivenPathExists(argv[2])
      && checkWhetherAccessToAPathIsAcquired(argv[1]) && checkWhetherAccessToAPathIsAcquired(argv[2])) {
    if (argc == 3) {
      start_server(argv[1], argv[2]);
    } else if (argc == 4) {
      char *endPtr;
      uint32_t portNum = strtol(argv[3], &endPtr, decimalBasis);
      if (*endPtr != '\0' || endPtr == argv[4]) {
        exit(EXIT_FAILURE);
      }

      start_server(argv[1], argv[2], portNum);
    } else {
      exit(EXIT_FAILURE);
    }
  } else {
    exit(EXIT_FAILURE);
  }

  return 0;
}
