#pragma once
#include <string>
#include <vector>

namespace util {

std::string trim(const std::string& s);
std::string toLower(const std::string& s);
bool startsWith(const std::string& s, const std::string& prefix);
bool startsWithCI(const std::string& s, const std::string& prefix); // case-insensitive
std::vector<std::string> splitLines(const std::string& s);
std::string escapeString(const std::string& s);   // for display: escape \n, \t, etc.

} // namespace util
