#include "file_system_functionalities.h"
#include <filesystem>

bool check_whether_given_path_exists(std::string const &path) {
  return std::filesystem::exists(path);
}

bool check_whether_access_to_a_path_is_acquired(std::string const &path) {
  namespace fs = std::filesystem;
  fs::perms p = fs::status(path).permissions();

  return (p & fs::perms::others_read) != fs::perms::none;
}