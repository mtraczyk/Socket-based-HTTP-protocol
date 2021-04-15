#ifndef PARSING_FUNCTIONALITIES_H
#define PARSING_FUNCTIONALITIES_H

#include <regex>
#include <iostream>

namespace HTTPRequestPatterns {
  using std::regex;

  regex requestLine("(GET|HEAD) [a-zA-Z0-9.-]+ (HTTP/1.1)[\r|\n]*");
}

class HTTPRequestParser {
  public:
    HTTPRequestParser() = default;
    void parsePartOfARequest(std::string &);
    bool isALineParsed() const;
    std::string const &getFullyParsedLine() const;
    ~HTTPRequestParser() = default;
  private:
    std::string currentLine;
    std::string nextPartOfARequest;
};


#endif /* PARSING_FUNCTIONALITIES_H */
