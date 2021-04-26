#include "server.h"
#include "err.h"
#include "file_system_functionalities.h"
#include "parsing_functionalities.h"
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fatal("Usage: %s <nazwa-katalogu-z-plikami> "
          "<plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>] ...\n", argv[0]);
  }

  if (checkWhetherGivenPathExists(argv[1]) && checkWhetherGivenPathExists(argv[2])
      && checkWhetherAccessToAPathIsAcquired(argv[1]) && checkWhetherAccessToAPathIsAcquired(argv[2])) {
    if (isDirectory(argv[1])) {
      if (argc == 3) {
        // Port number is set to the default value of 8080.
        startServer(argv[1], argv[2]);
      } else if (argc == 4) {
        // Exits with failure when port isn't correct.
        uint16_t portNum = portFromArg(argv[3]);

        // Port number is specified.
        startServer(argv[1], argv[2], portNum);
      }
    } else {
      fatal("First parameter must be a directory.");
    }
  } else {
    syserr("Incorrect parameters/access to parameters denied.");
  }

  return 0;
}
