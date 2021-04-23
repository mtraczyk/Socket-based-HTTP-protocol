#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

class HTTPRequestParser {
  public:
    // It is used to store info in the form of RequestType, connection, path in the request.
    using requestInfo = std::tuple<uint8_t, uint8_t, std::string>;

    constexpr static uint8_t noRequest = 0;
    constexpr static uint8_t requestGET = 1;
    constexpr static uint8_t requestHEAD = 2;
    constexpr static uint8_t connectionDefault = 0;
    constexpr static uint8_t connectionKeepAlive = 0;
    constexpr static uint8_t connectionClose = 1;
    constexpr static uint8_t sizeOfCRLFBlock = 2;

    HTTPRequestParser() = default;

    // Parses part of a request given as a string of any size.
    void parsePartOfARequest(std::string const &);

    // Returns true if a line was correctly parsed.
    bool isALineParsed() const noexcept;

    // Returns true if an error in a request occurred.
    bool hasAnErrorOccurred() const noexcept;

    // Returns info of a fully parsed request.
    std::pair<bool, requestInfo> getFullyParsedRequest() noexcept;

    // Prepares an instance to parse another request from the same client.
    void cleanAfterParsingWholeRequest() noexcept;

    // Resets an instance to prepare for parsing requests from other clients.
    void reset() noexcept;

    ~HTTPRequestParser() = default;
  private:
    // Used to set info about a correctly parsed request line.
    friend void getInfoAboutParsedLine(HTTPRequestParser *, uint8_t) noexcept;

    // Used to process a fully parsed line.
    friend void processAParsedLine(HTTPRequestParser *);

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
