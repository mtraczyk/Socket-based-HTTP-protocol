#include "request_answer.h"
#include "file_system_functionalities.h"
#include <unistd.h>

namespace {
  using byte = uint8_t;

  inline bool writeMessageToADescriptor(int32_t msgSock, std::string const &message) {
    auto numberOfBytesYetToBeWritten = message.size();
    auto messageAsACString = message.c_str();
    auto len = write(msgSock, messageAsACString, numberOfBytesYetToBeWritten);
    if (len <= 0) {
      return false;
    }
    auto numberOfBytesAlreadyWritten = len;
    numberOfBytesYetToBeWritten -= len;

    // Size of the message might be bigger than the buffer's size.
    while (numberOfBytesYetToBeWritten != 0) {
      len = write(msgSock, &messageAsACString[numberOfBytesAlreadyWritten], numberOfBytesYetToBeWritten);
      if (len < 0) {
        return false;
      }

      numberOfBytesAlreadyWritten += len;
      numberOfBytesYetToBeWritten -= len;
    }


    return true;
  }

  // Auxiliary function which checks if the connection close header needs to be added to the server's response.
  inline void checkConnectionStatus(std::string &answer, HTTPRequestParser::parserCodesType connection) {
    if (connection == HTTPRequestParser::connectionClose) {
      answer += "Connection: close\r\n";
    }
  }

  bool sendAnswerForFileFoundWithinGivenCatalog(int32_t msgSock, HTTPRequestParser::parserCodesType requestType,
                                                HTTPRequestParser::parserCodesType connection,
                                                std::vector<byte> const &bytes) {
    std::string answer = "HTTP/1.1 200 answer for a correct request\r\n";
    checkConnectionStatus(answer, connection);
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
                                                   HTTPRequestParser::parserCodesType connection,
                                                   std::pair<std::string, std::string> const &correlatedServersAddress) {
    std::string answer = "HTTP/1.1 302 file found found within correlated servers\r\n";
    checkConnectionStatus(answer, connection);
    answer += "Location: http://" + correlatedServersAddress.first + ":" +
              correlatedServersAddress.second + pathFromRequest + "\r\n\r\n";

    return writeMessageToADescriptor(msgSock, answer);
  }

  bool sendAnswerForNotFoundFile(int32_t msgSock, HTTPRequestParser::parserCodesType connection) {
    std::string answer = "HTTP/1.1 404 file not found\r\n";
    checkConnectionStatus(answer, connection);
    answer += "\r\n";

    return writeMessageToADescriptor(msgSock, answer);
  }
}

void incorrectRequestAnswer(int32_t msgSock, HTTPRequestParser::errorCodeType errorType) noexcept {
  std::string answer;
  if (errorType == HTTPRequestParser::wrongFormatError) {
    answer = "HTTP/1.1 400 incorrect request format\r\nConnection: close\r\n\r\n";
  } else if (errorType == HTTPRequestParser::unsupportedFunctionalityError) {
    answer = "HTTP/1.1 501 unsupported functionality\r\nConnection: close\r\n\r\n";
  } else if (errorType == HTTPRequestParser::wrongPathError) {
    answer = "HTTP/1.1 404 unsupported functionality\r\nConnection: close\r\n\r\n";
  }
  writeMessageToADescriptor(msgSock, answer);
}

void serverErrorAnswer(int32_t msgSock) noexcept {
  std::string answer = "HTTP/1.1 500 server error answer\r\nConnection: close\r\n\r\n";
  writeMessageToADescriptor(msgSock, answer);
}

bool correctRequestAnswer(int32_t msgSock, std::string const &mainCatalogAbsolutePath,
                          requestData::requestInfo const &parsedRequestInfo,
                          requestData::correlatedServersInfoMap const &resourcesToAcquireWithCorrelatedServers) {
  auto requestType = std::get<0>(parsedRequestInfo);
  auto connection = std::get<1>(parsedRequestInfo);
  auto const &pathFromRequest = std::get<2>(parsedRequestInfo);
  std::vector<byte> bytes;
  std::string path = mainCatalogAbsolutePath + "/" + pathFromRequest;

  try {
    if (!isFileContainedWithinGivenDirectory(mainCatalogAbsolutePath, path) || isDirectory(path)) {
      /* If a path represents a directory not a file or a file is not contained within directory
       * Send response with 404 error code.
       */
      if (!sendAnswerForNotFoundFile(msgSock, connection)) {
        // An error occurred and the connection needs to be closed.
        return false;
      }
      // If the connection close header appeared then the connection has to be closed.
      return connection != HTTPRequestParser::connectionClose;
    }

    if (checkWhetherGivenPathExists(path)) {
      // The path is a file, it is contained within the directory and exists.
      if (checkWhetherAccessToAPathIsAcquired(path) &&
          getApplicationOctetStreamRepresentationOfAFile(path, bytes)) {
        if (!sendAnswerForFileFoundWithinGivenCatalog(msgSock, requestType, connection, bytes)) {
          // Connection has to be closed.
          return false;
        }
        // If the connection close header appeared then the connection has to be closed.
        return connection != HTTPRequestParser::connectionClose;
      } else {
        // The file can't be accessed or reading from it didn't finish successfully.
        serverErrorAnswer(msgSock);
        return false;
      }
    }

    if (resourcesToAcquireWithCorrelatedServers.find(pathFromRequest) !=
        resourcesToAcquireWithCorrelatedServers.end()) {
      // Resource is present within the data cached from the correlated servers' file.
      auto const &correlatedServersAddress = resourcesToAcquireWithCorrelatedServers.find(pathFromRequest)->second;
      if (!sendAnswerForFileFoundWithCorrelatedServers(msgSock, pathFromRequest, connection,
                                                       correlatedServersAddress)) {
        // Connection has to be closed.
        return false;
      }
    } else {
      if (!sendAnswerForNotFoundFile(msgSock, connection)) {
        return false;
      }
    }
  } catch (std::exception const &e) {
    // std::filesystem may throw errors, for instance when the path is too long.
    std::cerr << "error caught:" << e.what() << std::endl;
    sendAnswerForNotFoundFile(msgSock, connection);

    return connection != HTTPRequestParser::connectionClose;
  }

  // If the connection close header appeared then the connection has to be closed.
  return connection != HTTPRequestParser::connectionClose;
}
