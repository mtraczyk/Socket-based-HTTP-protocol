#ifndef FILE_SYSTEM_FUNCTIONALITIES_H
#define FILE_SYSTEM_FUNCTIONALITIES_H

#include <iostream>
#include <unordered_map>
#include <vector>

// Checks existence of a path given as a string attribute.
bool checkWhetherGivenPathExists(std::string const &);

// Checks whether read access to a path given as a string attribute is acquired.
bool checkWhetherAccessToAPathIsAcquired(std::string const &);

// Auxiliary function used to obtain and to cache correlated servers data.
void getResourcesFromAFile(std::string const &, std::unordered_map<std::string, std::pair<std::string, std::string>> &);

/* Writes octet-stream representation of a file to a vector given as a reference.
 * Returns true if and only if write was fully successful.
 */
bool getApplicationOctetStreamRepresentationOfAFile(std::string const &, std::vector<uint8_t> &);

// Checks whether given path is a directory.
bool isDirectory(std::string const &);

// Checks whether a file is contained inside some directory.
bool isFileContainedWithinGivenDirectory(std::string const &, std::string const &);

// Converts path to an absolute path.
void convertToAbsolutePath(std::string &) noexcept;

#endif /* FILE_SYSTEM_FUNCTIONALITIES_H */
