#include "server.h"
#include "file_system_functionalities.h"
#include <unordered_map>
#include <filesystem>

namespace {
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

  std::cout << mainCatalog << " " << correlatedServers << std::endl;
}