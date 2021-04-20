#include "request_answer.h"
#include "file_system_functionalities.h"
#include "parsing_functionalities.h"
#include "err.h"
#include <unistd.h>

namespace {
  inline bool writeMessageToADescriptor(int32_t msgSock, std::string const &message) {
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

  bool sendAnswerForFileFoundWithCorrelatedServers(int32_t msgSock, std::string const &pathFromRequest,
                                                   std::pair<std::string, std::string> const &correlatedServersAddress) {
    std::string answer = "HTTP/1.1 302 file found found within correlated servers\r\n";
    answer += "Location: http://" + correlatedServersAddress.first + ":" +
              correlatedServersAddress.second + pathFromRequest + "\r\n\r\n";

    return writeMessageToADescriptor(msgSock, answer);
  }

  bool sendAnswerForNotFoundFile(int32_t msgSock) {
    std::string answer = "HTTP/1.1 404 file not found\r\n\r\n";

    return writeMessageToADescriptor(msgSock, answer);
  }
}

void incorrectRequestAnswer(int32_t msgSock) noexcept {
  std::string answer = "HTTP/1.1 400 incorrect request format\r\n\r\n";
  writeMessageToADescriptor(msgSock, answer);
}

void serverErrorAnswer(int32_t msgSock) noexcept {
  std::string answer = "HTTP/1.1 500 server error answer\r\n\r\n";
  writeMessageToADescriptor(msgSock, answer);
}

bool correctRequestAnswer(int32_t msgSock, std::string const &mainCatalogAbsolutePath,
                          requestData::requestInfo const &parsedRequestInfo,
                          requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
  auto const &pathFromRequest = std::get<2>(parsedRequestInfo);
  std::vector<uint8_t> bytes;
  std::string path = mainCatalogAbsolutePath;
  path += "/" + pathFromRequest;

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

  if (resourcesToAcquireWithCorrelatedServers.find(pathFromRequest) != resourcesToAcquireWithCorrelatedServers.end()) {
    auto const &correlatedServersAddress = resourcesToAcquireWithCorrelatedServers.find(pathFromRequest)->second;
    if (!sendAnswerForFileFoundWithCorrelatedServers(msgSock, pathFromRequest, correlatedServersAddress)) {
      return false;
    }
  } else {
    if (!sendAnswerForNotFoundFile(msgSock)) {
      return false;
    }
  }

  return std::get<1>(parsedRequestInfo) != HTTPRequestParser::connectionClose;
}