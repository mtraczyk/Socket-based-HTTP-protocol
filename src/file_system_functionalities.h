#ifndef FILE_SYSTEM_FUNCTIONALITIES_H
#define FILE_SYSTEM_FUNCTIONALITIES_H

#include <iostream>
#include <unordered_map>

bool checkWhetherGivenPathExists(std::string const &);
bool checkWhetherAccessToAPathIsAcquired(std::string const &);
void getResourcesFromAFile(std::string const &, std::unordered_map<std::string, std::pair<std::string, std::string>> &);

#endif /* FILE_SYSTEM_FUNCTIONALITIES_H */
