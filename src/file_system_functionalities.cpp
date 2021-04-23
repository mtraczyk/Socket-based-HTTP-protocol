#include "file_system_functionalities.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

// First std::string represents a path, pair of std::strings represents ip address and a port number.
using correlatedServersInfoMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;

namespace {
  // Stores bytes from a file in a given vector.
  inline auto readBytes(std::istream &is, std::vector<uint8_t> &bytes) -> std::istream & {
    return is.read(&reinterpret_cast<char &>(bytes[0]), bytes.size());
  }

  // Returns normalized version of a given path.
  inline std::filesystem::path normalizedPath(std::filesystem::path const &p) {
    return p.lexically_normal();
  }
}

bool checkWhetherGivenPathExists(std::string const &path) {
  return std::filesystem::exists(path);
}

bool checkWhetherAccessToAPathIsAcquired(std::string const &path) {
  namespace fs = std::filesystem;
  fs::perms p = fs::status(path).permissions();

  // Only check for read perms is performed.
  return (p & fs::perms::others_read) != fs::perms::none;
}

void getResourcesFromAFile(std::string const &path, correlatedServersInfoMap &resourcesMap) {
  // There is only need to read from this file.
  std::fstream fs(path, std::fstream::in);

  if (!fs.is_open()) {
    exit(EXIT_FAILURE);
  }

  char delimiter = '\t'; // ASCII for tab.
  std::string line, part;
  std::stringstream ss;
  std::vector<std::string> lineParts;

  // The file is read line by line and every line is divided into parts with tab as a delimiter.
  while (std::getline(fs, line)) {
    ss.clear();
    lineParts.clear();
    ss << line;

    while (std::getline(ss, part, delimiter)) {
      lineParts.push_back(part);
    }

    // Only the first occurrence of a path in the resources file needs to be cached.
    if (resourcesMap.find(lineParts[0]) == resourcesMap.end()) {
      resourcesMap[lineParts[0]] = {lineParts[1], lineParts[2]};
    }
  }

  fs.close();
}

bool getApplicationOctetStreamRepresentationOfAFile(std::string const &path, std::vector<uint8_t> &bytes) {
  auto fs = std::ifstream(path, std::ios_base::binary | std::ios_base::ate);
  auto const size = fs.tellg(); // Size of the file.
  bytes.resize(size); // Dynamically allocate the required space.
  fs.seekg(0); // Get to the beginning of a file.

  return !not readBytes(fs, bytes);
}

bool isDirectory(std::string const &path) {
  return std::filesystem::is_directory(path);
}

bool isFileContainedWithinGivenDirectory(std::string const &directory, std::string const &file) {
  // It's done in such a manner because paths "something/" and something are the same.
  auto b = normalizedPath(normalizedPath(directory).generic_string() + "/").generic_string();
  auto s = normalizedPath(normalizedPath(file).parent_path().generic_string() + "/").generic_string();
  // Check whether file's path is a prefix of directory's path.
  auto m = std::mismatch(b.begin(), b.end(), s.begin(), s.end());

  return m.first == b.end();
}

void convertToAbsolutePath(std::string &pathString) noexcept {
  std::filesystem::path path(pathString);
  if (path.is_relative()) {
    std::string currentPath = std::filesystem::current_path();
    pathString = currentPath + '/' + pathString;
  }
}