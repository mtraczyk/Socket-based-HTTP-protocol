#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

namespace HTTPRequestPatterns {
  using std::regex;

  regex requestLine("(GET|HEAD) [a-zA-Z0-9.-]+ (HTTP/1.1)");
}

class HTTPRequestParser {
  public:
    HTTPRequestParser() = default;
    void parsePartOfARequest(std::string const &);
    bool isALineParsed() const noexcept;
    bool isALineCorrect() const;
    std::string const &getFullyParsedLine() const;
    void prepareForParsingNextLine() noexcept;
    void reset() noexcept;
    ~HTTPRequestParser() = default;
  private:
    friend void parseRequestLine(HTTPRequestParser *, std::string const &);
    friend void lineIsParsed(HTTPRequestParser *, int32_t, std::string const &);

    constexpr static uint8_t requestLine = 1;
    constexpr static uint8_t headerField = 2;

    std::string currentLine;
    std::string nextPartOfARequest;
    bool lineParsed = false;
    bool lineCorrect = true;
    uint8_t partOfAMessageWhichIsBeingParsed = requestLine;
};


#endif /* PARSING_FUNCTIONALITIES_H */
