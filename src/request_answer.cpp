#include "request_answer.h"
#include "file_system_functionalities.h"
#include "parsing_functionalities.h"
#include "err.h"
#include <unistd.h>

namespace {
  bool writeMessageToADescriptor(int32_t msgSock, std::string const &message) {
    return write(msgSock, message.c_str(), message.size()) >= 0;
  }

  bool sendAnswerForFileFoundWithinGivenCatalog(int32_t msgSock, uint8_t requestType,
                                                std::vector<uint8_t> const &bytes) {
    std::string answer = "HTTP/1.1 200 answer for a correct request\r\n";
    answer += "Content-Type: application/octet-stream\r\n";
    answer += "Content-Length: " + std::to_string(bytes.size()) + "\r\n\r\n";

    if (requestType == HTTPRequestParser::requestGET) {
      for (auto const &u : bytes) {
        answer += u;
      }
    }

    return writeMessageToADescriptor(msgSock, answer);
  }

  bool sendAnswerForFileFoundWithCorrelatedServers(int32_t msgSock) {
    return true;
  }

  bool sendAnswerForNotFoundFile(int32_t msgSock) {
    return true;
  }
}

void incorrectRequestAnswer(int32_t msgSock) noexcept {

}

void serverErrorAnswer(int32_t msgSock) noexcept {

}

bool correctRequestAnswer(int32_t msgSock, std::string const &mainCatalogAbsolutePath,
                          requestData::requestInfo const &parsedRequestInfo,
                          requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
  std::vector<uint8_t> bytes;
  std::string path = mainCatalogAbsolutePath;
  path += "/" + std::get<2>(parsedRequestInfo);

  if (!isFileContainedWithinGivenDirectory(mainCatalogAbsolutePath, path)) {
    if (!sendAnswerForNotFoundFile(msgSock)) {
      return false;
    }

    return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
  }

  if (checkWhetherGivenPathExists(path)) {
    if (checkWhetherAccessToAPathIsAcquired(path) &&
        getApplicationOctetStreamRepresentationOfAFile(mainCatalogAbsolutePath, bytes)) {
      if (!sendAnswerForFileFoundWithinGivenCatalog(msgSock, std::get<0>(parsedRequestInfo), bytes)) {
        return false;
      }

      return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
    } else {
      serverErrorAnswer(msgSock);

      return false;
    }
  }

  if (resourcesToAcquireWithCorrelatedServers.find(path) != resourcesToAcquireWithCorrelatedServers.end()) {
    if (!sendAnswerForFileFoundWithCorrelatedServers(msgSock)) {
      return false;
    }
  } else {
    if (!sendAnswerForNotFoundFile(msgSock)) {
      return false;
    }
  }

  return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
}