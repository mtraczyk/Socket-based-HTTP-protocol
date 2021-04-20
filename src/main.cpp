#include "server.h"
#include "err.h"
#include "file_system_functionalities.h"
#include <unistd.h>

constexpr static uint32_t decimalBasis = 10;

int main(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fatal("Usage: %s <nazwa-katalogu-z-plikami> "
          "<plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>] ...\n", argv[0]);
  }

  if (checkWhetherGivenPathExists(argv[1]) && checkWhetherGivenPathExists(argv[2])
      && checkWhetherAccessToAPathIsAcquired(argv[1]) && checkWhetherAccessToAPathIsAcquired(argv[2])) {
    if (argc == 3) {
      start_server(argv[1], argv[2]);
    } else if (argc == 4) {
      char *endPtr;
      uint32_t portNum = strtol(argv[3], &endPtr, decimalBasis);
      if (*endPtr != '\0' || endPtr == argv[3]) {
        syserr("strtol");
      }

      start_server(argv[1], argv[2], portNum);
    }
  } else {
    syserr("Incorrect parameters/access to parameters denied.");
  }

  return 0;
}
