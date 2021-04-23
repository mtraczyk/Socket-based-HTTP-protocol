#include "parsing_functionalities.h"
#include <string>

constexpr static uint32_t firstIndexOfAString = 0;

namespace HTTPRequestPatterns {
  using std::regex;

  regex supportedRequestLine("(GET|HEAD)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex unsupportedRequestLine(R"([a-zA-Z0-9.-/\s]+\s/[a-zA-Z0-9.-/]*\s(HTTP/1.1))");
  regex headerConnection("(Connection):(\\s)*(close|keep-alive)(\\s)*");
  regex contentLength("(Content-Length):(\\s)*(0)+(\\s)*");
  regex unsupportedHeader(R"([a-zA-Z0-9.-/\s]+:(\s)*([a-zA-Z0-9.-/\s])+(\s)*)");
}

namespace {
  HTTPRequestParser::parserCodesType getRequestType(std::string const &requestLine) {
    if (requestLine.substr(firstIndexOfAString, HTTPRequestParser::lengthOfGETString) == "GET") {
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

    if (connectionHeaderLine == "close") {
      return HTTPRequestParser::connectionClose;
    }

    return HTTPRequestParser::connectionKeepAlive;
  }
}

void getInfoAboutParsedLine(HTTPRequestParser *requestParserInstance, uint8_t requestType) noexcept {
  if (requestParserInstance->requestType) {
    requestParserInstance->errorOccurred = true;
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
  auto const &currentLine = requestParserInstance->currentLine;

  if (std::regex_match(currentLine, HTTPRequestPatterns::supportedRequestLine)) {
    // Check if it is a GET or HEAD request.
    auto requestType = getRequestType(currentLine);
    getInfoAboutParsedLine(requestParserInstance, requestType);
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::headerConnection)) {
    requestParserInstance->errorOccurred =
      ((requestParserInstance->connection) ? true : requestParserInstance->errorOccurred);
    requestParserInstance->connection = getConnectionType(currentLine);
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::contentLength)) {
    requestParserInstance->errorOccurred =
      ((requestParserInstance->contentLengthHeaderOccurred) ? true : requestParserInstance->errorOccurred);
    requestParserInstance->contentLengthHeaderOccurred = true;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::unsupportedRequestLine)
             || std::regex_match(currentLine, HTTPRequestPatterns::unsupportedHeader)) {
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::unsupportedFunctionalityError;
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
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
  requestType = noRequest;
  connection = connectionKeepAlive;
  errorType = noError;
}


