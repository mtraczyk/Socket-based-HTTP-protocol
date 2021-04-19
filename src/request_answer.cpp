#include "request_answer.h"
#include "file_system_functionalities.h"
#include "parsing_functionalities.h"
#include "err.h"
#include <unistd.h>

namespace {
  void writeMessageToADescriptor(int32_t msgSock, std::string const &message) {
    if (write(msgSock, message.c_str(), message.size()) != message.size()) {
      syserr("partial / failed write");
    }
  }

  void sendAnswerForFileFoundWithinGivenCatalog(int32_t msgSock, uint8_t requestType,
                                                std::vector<uint8_t> const &bytes) {
    std::string answer = "HTTP/1.1 200 answer for a correct request\r\n";
    answer += "Content-Type: application/octet-stream\r\n";
    answer += "Content-Length: " + std::to_string(bytes.size()) + "\r\n\r\n";

    if (requestType == HTTPRequestParser::requestGET) {
      for (auto const &u : bytes) {
        answer += u;
      }
    }

    writeMessageToADescriptor(msgSock, answer);
  }

  void sendAnswerForFileFoundWithCorrelatedServers(int32_t msgSock) {

  }

  void sendAnswerForNotFoundFile(int32_t msgSock) {

  }
}

void incorrectRequestAnswer(int32_t msgSock) noexcept {

}

void serverErrorAnswer(int32_t msgSock) noexcept {

}

bool correctRequestAnswer(int32_t msgSock, std::string const &mainCatalogFullPath,
                          requestData::requestInfo const &parsedRequestInfo,
                          requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
  std::vector<uint8_t> bytes;
  std::string path = mainCatalogFullPath;

  if (std::get<2>(parsedRequestInfo) != "/") {
    path += std::get<2>(parsedRequestInfo);
  }

  if (checkWhetherGivenPathExists(path)) {
    if (checkWhetherAccessToAPathIsAcquired(path) &&
        getApplicationOctetStreamRepresentationOfAFile(mainCatalogFullPath, bytes)) {
      sendAnswerForFileFoundWithinGivenCatalog(msgSock, std::get<0>(parsedRequestInfo), bytes);

      return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
    } else {
      serverErrorAnswer(msgSock);

      return false;
    }
  }

  if (resourcesToAcquireWithCorrelatedServers.find(path) != resourcesToAcquireWithCorrelatedServers.end()) {
    sendAnswerForFileFoundWithCorrelatedServers(msgSock);
  } else {
    sendAnswerForNotFoundFile(msgSock);
  }

  return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
}