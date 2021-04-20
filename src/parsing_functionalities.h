#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

class HTTPRequestParser {
  public:
    using requestInfo = std::tuple<uint8_t, uint8_t, std::string>;

    constexpr static uint8_t requestGET = 1;
    constexpr static uint8_t requestHEAD = 2;
    constexpr static uint8_t noRequest = 0;
    constexpr static uint8_t connectionDefault = 0;
    constexpr static uint8_t connectionClose = 1;
    constexpr static uint8_t sizeOfCRLFBlock = 2;

    HTTPRequestParser() = default;
    void parsePartOfARequest(std::string const &);
    bool isALineParsed() const noexcept;
    bool hasAnErrorOccurred() const noexcept;
    std::pair<bool, requestInfo> getFullyParsedRequest() noexcept;
    void cleanAfterParsingWholeRequest() noexcept;
    void reset() noexcept;
    ~HTTPRequestParser() = default;
  private:
    friend void parseRequestLine(HTTPRequestParser *, uint8_t) noexcept;
    friend void lineIsParsed(HTTPRequestParser *);
    void prepareForParsingNextLine() noexcept;

    std::string currentLine;
    std::string nextPartOfARequest;
    std::string resourcePath;
    int32_t nextPartOfARequestsIndexPosition = 0;
    uint8_t requestType = noRequest;
    uint8_t connection = connectionDefault;
    bool lineParsed = false;
    bool errorOccurred = false;
    bool requestParsed = false;
    bool contentLengthHeaderOccurred = false;
    requestInfo finalRequestInfo = {noRequest, connectionDefault, ""};
};


#endif /* PARSING_FUNCTIONALITIES_H */
