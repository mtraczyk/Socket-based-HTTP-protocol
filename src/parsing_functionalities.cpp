#include "parsing_functionalities.h"
#include <string>

constexpr static uint32_t firstIndexOfAString = 0;

namespace HTTPRequestPatterns {
  using std::regex;

  regex requestLineGET("(GET)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex requestLineHEAD("(HEAD)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex headerConnectionClose("(Connection):(\\s)*(close)(\\s)*");
  regex contentLength("(Content-Length):(\\s)*(0)(\\s)*");
}

void parseRequestLine(HTTPRequestParser *requestParserInstance, uint8_t requestType) noexcept {
  if (requestParserInstance->requestType) {
    requestParserInstance->errorOccurred = true;
  } else {
    requestParserInstance->requestType = requestType;
    requestParserInstance->resourcePath.clear();

    for (uint32_t i = requestParserInstance->currentLine.find('/'); requestParserInstance->currentLine[i] != ' '; i++) {
      requestParserInstance->resourcePath += requestParserInstance->currentLine[i];
    }
  }
}

void lineIsParsed(HTTPRequestParser *requestParserInstance) {
  requestParserInstance->lineParsed = true;

  if (std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::requestLineGET)) {
    parseRequestLine(requestParserInstance, HTTPRequestParser::requestGET);
  } else if (std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::requestLineHEAD)) {
    parseRequestLine(requestParserInstance, HTTPRequestParser::requestHEAD);
  } else if (std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::headerConnectionClose)) {
    requestParserInstance->errorOccurred =
      ((requestParserInstance->connection) ? true : requestParserInstance->errorOccurred);
    requestParserInstance->connection = HTTPRequestParser::connectionClose;
  } else if (std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::contentLength)) {
    requestParserInstance->errorOccurred =
      ((requestParserInstance->contentLengthHeaderOccurred) ? true : requestParserInstance->errorOccurred);
    requestParserInstance->contentLengthHeaderOccurred = true;
  } else {
    requestParserInstance->errorOccurred = true;
  }

  requestParserInstance->currentLine.clear();
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  nextPartOfARequest += requestPart;
  int32_t positionOfCRLF = nextPartOfARequest.find("\r\n", nextPartOfARequestsIndexPosition);

  if (positionOfCRLF == std::string::npos) {
    lineParsed = false;
    if (nextPartOfARequest[nextPartOfARequest.size() - 1] != '\r') {
      prepareForParsingNextLine();
    }
  } else {
    if (currentLine.empty() && nextPartOfARequestsIndexPosition == positionOfCRLF) {
      nextPartOfARequestsIndexPosition += sizeOfCRLFBlock;

      if (requestType) {
        requestParsed = true;
      } else {
        errorOccurred = true;
      }

      return;
    }

    currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition,
                                             positionOfCRLF - nextPartOfARequestsIndexPosition);
    lineIsParsed(this);
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
  currentLine = nextPartOfARequest.substr(nextPartOfARequestsIndexPosition);
  nextPartOfARequest.clear();
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
}

void HTTPRequestParser::cleanAfterParsingWholeRequest() noexcept {
  lineParsed = false;
  errorOccurred = false;
  requestParsed = false;
  contentLengthHeaderOccurred = false;
  requestType = noRequest;
  connection = connectionDefault;
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
  connection = connectionDefault;
}


