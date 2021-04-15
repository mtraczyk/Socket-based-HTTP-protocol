#include "server.h"
#include "file_system_functionalities.h"

int main(int argc, char **argv) {
  if (check_whether_given_path_exists(argv[1]) && check_whether_given_path_exists(argv[2])
      && check_whether_access_to_a_path_is_acquired(argv[1]) && check_whether_access_to_a_path_is_acquired(argv[2])) {
    start_server();
  } else {
    exit(EXIT_FAILURE);
  }

  return 0;
}
