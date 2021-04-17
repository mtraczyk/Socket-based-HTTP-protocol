#include "parsing_functionalities.h"
#include <string>

void parseRequestLine(HTTPRequestParser *requestParserInstance, uint8_t requestType) noexcept {
  if (requestParserInstance->requestType) {
    if (requestParserInstance->CRLFBlockSize >= HTTPRequestParser::minimumCRLFEndingBlockSize) {
      requestParserInstance->finalRequestInfo =
        {requestParserInstance->requestType, requestParserInstance->connection, requestParserInstance->resourcePath};
    } else {
      requestParserInstance->errorOccurred = true;
      return;
    }

    requestParserInstance->connection = HTTPRequestParser::connectionDefault;
  }

  requestParserInstance->resourcePath.clear();
  requestParserInstance->requestType = requestType;
  for (uint32_t i = requestParserInstance->currentLine.find('/'); requestParserInstance->currentLine[i] != ' '; i++) {
    requestParserInstance->resourcePath += requestParserInstance->currentLine[i];
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
  } else if (!std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::contentLength)) {
    requestParserInstance->errorOccurred = true;
  }

  requestParserInstance->currentLine.clear();
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  nextPartOfARequest += requestPart;
  int32_t positionOfEndLine = nextPartOfARequest.find('\n', nextPartOfARequestsIndexPosition);
  int32_t positionOfCarriageReturn = nextPartOfARequest.find('\r', nextPartOfARequestsIndexPosition);

  if (positionOfEndLine == std::string::npos && positionOfCarriageReturn == std::string::npos) {
    prepareForParsingNextLine();
  } else {
    int32_t splitPosition = std::min(
      ((positionOfEndLine != std::string::npos) ? positionOfEndLine : nextPartOfARequest.size()),
      ((positionOfCarriageReturn != std::string::npos) ? positionOfCarriageReturn : nextPartOfARequest.size()));

    lineIsParsed(this);

    if (nextPartOfARequestsIndexPosition != splitPosition) {
      currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition, splitPosition - 1);
      CRLFBlockSize = 1;
    }

    nextPartOfARequestsIndexPosition = splitPosition;
    while (nextPartOfARequest[nextPartOfARequestsIndexPosition] == '\n'
           || nextPartOfARequest[nextPartOfARequestsIndexPosition] == '\r') {
      nextPartOfARequestsIndexPosition++;
      CRLFBlockSize++;
    }
  }
}

bool HTTPRequestParser::isALineParsed() const noexcept {
  return lineParsed;
}

bool HTTPRequestParser::hasAnErrorOccurred() const noexcept {
  return errorOccurred;
}

std::pair<bool, HTTPRequestParser::requestInfo> HTTPRequestParser::getFullyParsedRequest() noexcept {
  requestInfo finalRequestInfoCopy = finalRequestInfo;
  finalRequestInfo = {noRequest, connectionDefault, ""};

  return {std::get<0>(finalRequestInfoCopy) != noRequest, finalRequestInfoCopy};
}

void HTTPRequestParser::prepareForParsingNextLine() noexcept {
  lineParsed = false;
  currentLine = nextPartOfARequest.substr(nextPartOfARequestsIndexPosition);
  nextPartOfARequest.clear();
  nextPartOfARequestsIndexPosition = 0;

  if (!currentLine.empty()) {
    CRLFBlockSize = 0;
  }
}

void HTTPRequestParser::reset() noexcept {
  currentLine.clear();
  nextPartOfARequest.clear();
  resourcePath.clear();
  lineParsed = false;
  errorOccurred = false;
  nextPartOfARequestsIndexPosition = 0;
  CRLFBlockSize = 0;
  requestType = 0;
  requestType = 0;
  connection = 0;
  contentLengthDifferentThanZero = false;
}

