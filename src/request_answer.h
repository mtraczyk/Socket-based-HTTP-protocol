#ifndef REQUEST_ANSWER_H
#define REQUEST_ANSWER_H

#include <iostream>
#include <string>
#include <unordered_map>

namespace requestData {
  using correlatedServersInfoMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;
  using requestInfo = std::tuple<uint8_t, uint8_t, std::string>;
}

void incorrectRequestAnswer() noexcept;
void serverErrorAnswer() noexcept;
bool correctRequestAnswer(std::string const &, requestData::requestInfo const &,
                          requestData::correlatedServersInfoMap const &);

#endif /* REQUEST_ANSWER_H */
