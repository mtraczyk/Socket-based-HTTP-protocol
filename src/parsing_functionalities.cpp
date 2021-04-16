#include "parsing_functionalities.h"
#include <iostream>
#include <string>

constexpr static int32_t firstIndexOfAString = 0;

void parseRequestLine(HTTPRequestParser *requestParserInstance, std::string const &requestPart) {

}

void lineIsParsed(HTTPRequestParser *requestParserInstance) {
  if (std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::requestLine)) {

  } else {

  }

  requestParserInstance->currentLine.clear();
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  nextPartOfARequest += requestPart;
  int32_t positionOfNewLine = nextPartOfARequest.find('\n', nextPartOfARequestsIndexPosition);
  int32_t positionOfCarriageReturn = nextPartOfARequest.find('\r', nextPartOfARequestsIndexPosition);

  if (positionOfNewLine == std::string::npos && positionOfCarriageReturn == std::string::npos) {
    prepareForParsingNextLine();
  } else {
    int32_t splitPosition = std::min(
      ((positionOfNewLine != std::string::npos) ? positionOfNewLine : nextPartOfARequest.size()),
      ((positionOfCarriageReturn != std::string::npos) ? positionOfCarriageReturn : nextPartOfARequest.size()));

    currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition, splitPosition - 1);

    nextPartOfARequestsIndexPosition = splitPosition;
    CRLFBlockSize = 1;
    while (nextPartOfARequest[nextPartOfARequestsIndexPosition] == '\n'
           || nextPartOfARequest[nextPartOfARequestsIndexPosition] == '\r') {
      nextPartOfARequestsIndexPosition++;
      CRLFBlockSize++;
    }

    lineIsParsed(this);
  }
}

bool HTTPRequestParser::isALineParsed() const noexcept {
  return lineParsed;
}

bool HTTPRequestParser::hasAnErrorOccurred() const noexcept {
  return errorOccurred;
}

void HTTPRequestParser::prepareForParsingNextLine() noexcept {
  lineParsed = false;
  currentLine = nextPartOfARequest.substr(nextPartOfARequestsIndexPosition);
  nextPartOfARequest.clear();
  nextPartOfARequestsIndexPosition = 0;
  CRLFBlockSize = 0;
}

void HTTPRequestParser::reset() noexcept {
  currentLine.clear();
  nextPartOfARequest.clear();
  lineParsed = false;
  errorOccurred = false;
  nextPartOfARequestsIndexPosition = 0;
  CRLFBlockSize = 0;
}
