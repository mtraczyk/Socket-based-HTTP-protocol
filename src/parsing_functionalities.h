#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

namespace HTTPRequestPatterns {
  using std::regex;

  regex requestLineGET("(GET)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex requestLineHEAD("(HEAD)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex headerConnectionClose("(Connection):(\\s)*(close)(\\s)*");
  regex contentLength("(Content-Length):(\\s)*(0)(\\s)*");
}

class HTTPRequestParser {
  public:
    HTTPRequestParser() = default;
    void parsePartOfARequest(std::string const &);
    bool isALineParsed() const noexcept;
    bool hasAnErrorOccurred() const noexcept;
    void prepareForParsingNextLine() noexcept;
    void reset() noexcept;
    ~HTTPRequestParser() = default;
  private:
    friend void parseRequestLine(HTTPRequestParser *, std::string const &);
    friend void lineIsParsed(HTTPRequestParser *);

    constexpr static uint8_t requestGET = 1;
    constexpr static uint8_t requestHEAD = 2;
    constexpr static uint8_t connectionClose = 1;

    std::string currentLine;
    std::string nextPartOfARequest;
    uint32_t nextPartOfARequestsIndexPosition = 0;
    uint32_t CRLFBlockSize = 0;
    uint8_t requestType = 0;
    uint8_t connection = 0;
    bool contentLengthDifferentThanZero = false;
    bool lineParsed = false;
    bool errorOccurred = false;
};


#endif /* PARSING_FUNCTIONALITIES_H */
