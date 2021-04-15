#include "parsing_functionalities.h"
#include <iostream>
#include <string>

constexpr static int32_t firstIndexOfAString = 0;

void lineIsParsed(HTTPRequestParser *requestParserInstance, int32_t splitPosition, std::string const &requestPart) {
  requestParserInstance->lineParsed = true;
  requestParserInstance->currentLine += requestPart.substr(firstIndexOfAString, splitPosition);
  requestParserInstance->nextPartOfARequest = requestPart.substr(splitPosition);
}

void parseRequestLine(HTTPRequestParser *requestParserInstance, std::string const &requestPart) {
  int32_t positionOfNewLine = requestPart.find('\n');
  int32_t positionOfCarriageReturn = requestPart.find('\r');

  if (positionOfNewLine == std::string::npos && positionOfCarriageReturn == std::string::npos) {
    requestParserInstance->currentLine += requestPart;
  } else {
    int32_t splitPosition = std::min(
      ((positionOfNewLine != std::string::npos) ? positionOfNewLine : requestPart.size()),
      ((positionOfCarriageReturn != std::string::npos) ? positionOfCarriageReturn : requestPart.size()));

    lineIsParsed(requestParserInstance, splitPosition, requestPart);

    if (!std::regex_match(requestParserInstance->currentLine, HTTPRequestPatterns::requestLine)) {
      requestParserInstance->lineCorrect = false;
    }
  }
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  switch (partOfAMessageWhichIsBeingParsed) {
    case requestLine:
      parseRequestLine(this, requestPart);
      break;
  }
}

bool HTTPRequestParser::isALineParsed() const noexcept {
  return lineParsed;
}

bool HTTPRequestParser::isALineCorrect() const {
  if (!isALineParsed()) {
    throw std::logic_error(
      "Prior to calling HTTPRequestParser::isALineCorrect, HTTPRequestParser::isALineParsed should return true.");
  }

  return lineCorrect;
}

std::string const &HTTPRequestParser::getFullyParsedLine() const {
  if (!isALineParsed()) {
    throw std::logic_error(
      "Prior to calling HTTPRequestParser::getFullyParsedLine, HTTPRequestParser::isALineParsed should return true.");
  }

  return currentLine;
}

void HTTPRequestParser::prepareForParsingNextLine() noexcept {
  std::swap(currentLine, nextPartOfARequest);
  nextPartOfARequest.clear();
  lineParsed = false;
  lineCorrect = true;
}

void HTTPRequestParser::reset() noexcept {
  currentLine.clear();
  nextPartOfARequest.clear();
  lineParsed = false;
  partOfAMessageWhichIsBeingParsed = requestLine;
  lineCorrect = true;
}
