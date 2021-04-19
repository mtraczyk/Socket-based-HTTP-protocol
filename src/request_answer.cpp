#include "request_answer.h"
#include "file_system_functionalities.h"

void incorrectRequestAnswer() noexcept {

}

void serverErrorAnswer() noexcept {

}

bool correctRequestAnswer(std::string const &mainCatalogFullPath, requestData::requestInfo const &parsedRequestInfo,
                          requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
  std::vector<uint8_t> bytes;
  std::string path = mainCatalogFullPath + std::get<2>(parsedRequestInfo);

  if (checkWhetherGivenPathExists(path)) {
    if (checkWhetherAccessToAPathIsAcquired(path) &&
        getApplicationOctetStreamRepresentationOfAFile(mainCatalogFullPath, bytes)) {
#warning 200
      for (auto const &u : bytes) {
        std::cout << u;
      }

      std::cout << std::endl;
      return true;
    } else {
#warning 500
      serverErrorAnswer();
      return false;
    }
  }

  if (resourcesToAcquireWithCorrelatedServers.find(path) != resourcesToAcquireWithCorrelatedServers.end()) {
#warning 302
  } else {
#warning 404
  }

  return true;
}