#include "parsing_functionalities.h"
#include <string>

constexpr static size_t firstIndexOfAString = 0;

namespace HTTPRequestPatterns {
  using std::regex;

  regex supportedRequestLine(R"((GET|HEAD)\s\/[a-zA-Z0-9.\-\/]*\s(HTTP\/1\.1))");
  regex unsupportedRequestLine(R"([^\s]+\s/[a-zA-Z0-9.\-\/]*\s(HTTP\/1\.1))");
  regex headerConnection(R"((C|c)(onnection):(\s)*(close|keep\-alive)(\s)*)");
  regex contentLength(R"((Content\-Length):(\s)*(0)+(\s)*)");
  regex incorrectContentLength(R"((Content\-Length):(\s)*.*(\s)*)");
  regex unsupportedHeader(R"(.+:(\s)*.*(\s)*)");
}

namespace {
  constexpr size_t lengthOfGETString = 3;

  HTTPRequestParser::parserCodesType getRequestType(std::string const &requestLine) {
    if (requestLine.substr(firstIndexOfAString, lengthOfGETString) == "GET") {
      return HTTPRequestParser::requestGET;
    }

    return HTTPRequestParser::requestHEAD;
  }

  HTTPRequestParser::parserCodesType getConnectionType(std::string const &connectionHeaderLine) {
    auto index = connectionHeaderLine.find(':') + 1; // OWS starting index.
    std::string connectionTypeString;

    while (connectionHeaderLine[index] == ' ') {
      index++;
    }

    while (std::isalpha(connectionHeaderLine[index]) && index < connectionHeaderLine.size()) {
      connectionTypeString += connectionHeaderLine[index];
      index++;
    }

    if (connectionTypeString == "close") {
      return HTTPRequestParser::connectionClose;
    }

    return HTTPRequestParser::connectionKeepAlive;
  }
}

void getInfoAboutParsedLine(HTTPRequestParser *requestParserInstance, uint8_t requestType) noexcept {
  if (requestParserInstance->requestType) {
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  } else {
    requestParserInstance->requestType = requestType;
    requestParserInstance->resourcePath.clear();

    auto const &currentLine = requestParserInstance->currentLine;
    for (auto i = currentLine.find('/'); currentLine[i] != ' '; i++) {
      requestParserInstance->resourcePath += currentLine[i];
    }
  }
}

void processAParsedLine(HTTPRequestParser *requestParserInstance) {
  requestParserInstance->lineParsed = true;
  bool hasRequestOccurred = (requestParserInstance->requestType != HTTPRequestParser::noRequest);
  auto const &currentLine = requestParserInstance->currentLine;

  if (std::regex_match(currentLine, HTTPRequestPatterns::supportedRequestLine)) {
    // Check if it is a GET or HEAD request.
    auto requestType = getRequestType(currentLine);
    getInfoAboutParsedLine(requestParserInstance, requestType);
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::headerConnection) && hasRequestOccurred) {
    requestParserInstance->errorOccurred = requestParserInstance->connectionHeaderOccurred;
    setWrongFormatError(requestParserInstance);
    requestParserInstance->connection = getConnectionType(currentLine);
    requestParserInstance->connectionHeaderOccurred = true;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::contentLength) && hasRequestOccurred) {
    requestParserInstance->errorOccurred = requestParserInstance->contentLengthHeaderOccurred;
    setWrongFormatError(requestParserInstance);
    requestParserInstance->contentLengthHeaderOccurred = true;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::unsupportedRequestLine)) {
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::unsupportedFunctionalityError;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::incorrectContentLength)) {
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  } else if (!std::regex_match(currentLine, HTTPRequestPatterns::unsupportedHeader)) {
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  }

  requestParserInstance->currentLine.clear();
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  nextPartOfARequest += requestPart;
  auto positionOfCRLF = nextPartOfARequest.find("\r\n", nextPartOfARequestsIndexPosition);

  if (positionOfCRLF == std::string::npos) {
    lineParsed = false;
    if (nextPartOfARequest[nextPartOfARequest.size() - 1] != '\r') {
      prepareForParsingNextLine();
    }
  } else {
    if (currentLine.empty() && nextPartOfARequestsIndexPosition == positionOfCRLF) {
      nextPartOfARequestsIndexPosition += sizeOfCRLFBlock;

      if (requestType) {
        lineParsed = true;
        requestParsed = true;
      } else {
        errorOccurred = true;
      }

      return;
    }

    currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition,
                                             positionOfCRLF - nextPartOfARequestsIndexPosition);
    processAParsedLine(this);
    nextPartOfARequestsIndexPosition = positionOfCRLF + sizeOfCRLFBlock;
  }
}

inline void setWrongFormatError(HTTPRequestParser *parserInstance) noexcept {
  if (parserInstance->errorOccurred) {
    parserInstance->errorType = HTTPRequestParser::wrongFormatError;
  }
}

bool HTTPRequestParser::isALineParsed() const noexcept {
  return lineParsed;
}

bool HTTPRequestParser::hasAnErrorOccurred() const noexcept {
  return errorOccurred;
}

std::pair<bool, HTTPRequestParser::requestInfo> HTTPRequestParser::getFullyParsedRequest() noexcept {
  finalRequestInfo = {requestType, connection, resourcePath};

  return {requestParsed, finalRequestInfo};
}

HTTPRequestParser::errorCodeType HTTPRequestParser::getErrorType() const noexcept {
  return errorType;
}

void HTTPRequestParser::prepareForParsingNextLine() noexcept {
  currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition);
  nextPartOfARequest.clear();
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
}

void HTTPRequestParser::cleanAfterParsingWholeRequest() noexcept {
  lineParsed = false;
  errorOccurred = false;
  requestParsed = false;
  contentLengthHeaderOccurred = false;
  connectionHeaderOccurred = false;
  requestType = noRequest;
  connection = connectionKeepAlive;
  errorType = noError;
}

void HTTPRequestParser::reset() noexcept {
  currentLine.clear();
  nextPartOfARequest.clear();
  resourcePath.clear();
  lineParsed = false;
  errorOccurred = false;
  requestParsed = false;
  contentLengthHeaderOccurred = false;
  connectionHeaderOccurred = false;
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
  requestType = noRequest;
  connection = connectionKeepAlive;
  errorType = noError;
}




