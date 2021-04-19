#include "file_system_functionalities.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using correlatedServersInfoMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;

namespace {
  auto readBytes(std::istream &is, std::vector<uint8_t> &bytes) -> std::istream & {
    return is.read(&reinterpret_cast<char &>(bytes[0]), bytes.size());
  }
}

bool checkWhetherGivenPathExists(std::string const &path) {
  return std::filesystem::exists(path);
}

bool checkWhetherAccessToAPathIsAcquired(std::string const &path) {
  namespace fs = std::filesystem;
  fs::perms p = fs::status(path).permissions();

  return (p & fs::perms::others_read) != fs::perms::none;
}

void getResourcesFromAFile(std::string const &path, correlatedServersInfoMap &resourcesMap) {
  std::fstream fs(path, std::fstream::in);

  if (!fs.is_open()) {
    exit(EXIT_FAILURE);
  }

  char delimiter = '\t'; // ASCII for tab
  std::string line, part;
  std::stringstream ss;
  std::vector<std::string> lineParts;

  while (std::getline(fs, line)) {
    ss.clear();
    lineParts.clear();
    ss << line;

    while (std::getline(ss, part, delimiter)) {
      lineParts.push_back(part);
    }

    fs.close();
    if (resourcesMap.find(lineParts[0]) == resourcesMap.end()) {
      resourcesMap[lineParts[0]] = {lineParts[1], lineParts[2]};
    }
  }
}

bool getApplicationOctetStreamRepresentationOfAFile(std::string const &path, std::vector<uint8_t> &bytes) {
  auto fs = std::ifstream(path, std::ios_base::binary | std::ios_base::ate);
  auto const size = fs.tellg();
  bytes.resize(size);
  fs.seekg(0);

  return !not readBytes(fs, bytes);
}