#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

class HTTPRequestParser {
  public:
    // It is used to store info in the form of RequestType, connection, path in the request.
    using parserCodesType = uint8_t;
    using errorCodeType = uint16_t;
    using requestInfo = std::tuple<parserCodesType, parserCodesType, std::string>;

    constexpr static parserCodesType noRequest = 0;
    constexpr static parserCodesType requestGET = 1;
    constexpr static parserCodesType requestHEAD = 2;
    constexpr static parserCodesType connectionKeepAlive = 0;
    constexpr static parserCodesType connectionClose = 1;
    constexpr static parserCodesType sizeOfCRLFBlock = 2;
    constexpr static parserCodesType lengthOfGETString = 3;
    constexpr static errorCodeType noError = 0;
    constexpr static errorCodeType wrongFormat = 400;
    constexpr static errorCodeType unsupportedFunctionalityError = 501;

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
    friend void getInfoAboutParsedLine(HTTPRequestParser *, parserCodesType) noexcept;

    // Used to process a fully parsed line.
    friend void processAParsedLine(HTTPRequestParser *);

    void prepareForParsingNextLine() noexcept;

    std::string currentLine;
    std::string nextPartOfARequest;
    std::string resourcePath;
    uint64_t nextPartOfARequestsIndexPosition = 0;
    errorCodeType errorType = noError;
    parserCodesType requestType = noRequest;
    parserCodesType connection = connectionKeepAlive;
    bool lineParsed = false;
    bool errorOccurred = false;
    bool requestParsed = false;
    bool contentLengthHeaderOccurred = false;
    requestInfo finalRequestInfo = {noRequest, connectionKeepAlive, ""};
};


#endif /* PARSING_FUNCTIONALITIES_H */
