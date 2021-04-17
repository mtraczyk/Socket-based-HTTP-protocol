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
    using requestInfo = std::tuple<uint8_t, uint8_t, std::string>;

    HTTPRequestParser() = default;
    void parsePartOfARequest(std::string const &);
    bool isALineParsed() const noexcept;
    bool hasAnErrorOccurred() const noexcept;
    std::pair<bool, requestInfo> getFullyParsedRequest() noexcept;
    void prepareForParsingNextLine() noexcept;
    void reset() noexcept;
    ~HTTPRequestParser() = default;
  private:
    friend void parseRequestLine(HTTPRequestParser *, uint8_t) noexcept;
    friend void lineIsParsed(HTTPRequestParser *);

    constexpr static uint8_t requestGET = 1;
    constexpr static uint8_t requestHEAD = 2;
    constexpr static uint8_t noRequest = 0;
    constexpr static uint8_t connectionDefault = 0;
    constexpr static uint8_t connectionClose = 1;
    constexpr static uint8_t minimumCRLFEndingBlockSize = 2;

    std::string currentLine;
    std::string nextPartOfARequest;
    std::string resourcePath;
    uint32_t nextPartOfARequestsIndexPosition = 0;
    uint32_t CRLFBlockSize = 0;
    uint8_t requestType = noRequest;
    uint8_t connection = connectionDefault;
    bool contentLengthDifferentThanZero = false;
    bool lineParsed = false;
    bool errorOccurred = false;
    requestInfo finalRequestInfo = {noRequest, connectionDefault, ""};
};


#endif /* PARSING_FUNCTIONALITIES_H */
