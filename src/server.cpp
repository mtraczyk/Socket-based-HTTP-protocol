#include <unordered_map>
#include "server.h"
#include "file_system_functionalities.h"

void start_server(std::string const &mainCatalog, std::string const &correlatedServers) {
  std::unordered_map<std::string, std::pair<std::string, std::string>> resourcesToAcquireWithCorrelatedServers;
  getResourcesFromAFile(correlatedServers, resourcesToAcquireWithCorrelatedServers);
}