#include "util/string_util.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace util {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool startsWith(const std::string& s, const std::string& prefix) {
    if (prefix.size() > s.size()) return false;
    return s.compare(0, prefix.size(), prefix) == 0;
}

bool startsWithCI(const std::string& s, const std::string& prefix) {
    if (prefix.size() > s.size()) return false;
    return toLower(s.substr(0, prefix.size())) == toLower(prefix);
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> lines;
    std::istringstream stream(s);
    std::string line;
    while (std::getline(stream, line)) {
        // Remove trailing \r if present (for \r\n line endings)
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(std::move(line));
    }
    return lines;
}

std::string escapeString(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            case '\\': result += "\\\\"; break;
            case '"':  result += "\\\""; break;
            case '\0': result += "\\0";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Other control characters: hex escape
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\x%02x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

} // namespace util
