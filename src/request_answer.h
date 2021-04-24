#ifndef REQUEST_ANSWER_H
#define REQUEST_ANSWER_H

#include "parsing_functionalities.h"
#include <iostream>
#include <string>
#include <unordered_map>

namespace requestData {
  // map type to store cached data from correlated servers' file.
  using correlatedServersInfoMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;
  // requestInfo type compatible with requestInfo type declared in parsing_functionalities.h.
  using requestInfo = std::tuple<uint8_t, uint8_t, std::string>;
}

// Prepare and send an answer for an incorrect request.
void incorrectRequestAnswer(int32_t, HTTPRequestParser::errorCodeType) noexcept;

// Prepare and send an answer for a server error.
void serverErrorAnswer(int32_t) noexcept;

// Prepare and send an answer for a correct request.
bool correctRequestAnswer(int32_t, std::string const &, requestData::requestInfo const &,
                          requestData::correlatedServersInfoMap const &);

#endif /* REQUEST_ANSWER_H */
