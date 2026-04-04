#include "util/file_io.h"
#include <fstream>
#include <sstream>
#include <portable-file-dialogs.h>

namespace fileio {

std::optional<std::string> readFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return std::nullopt;

    std::ostringstream oss;
    oss << ifs.rdbuf();
    if (ifs.bad())
        return std::nullopt;

    return oss.str();
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return false;

    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    return ofs.good();
}

std::string openFileDialog(const std::string& title) {
    auto selection = pfd::open_file(
        title,
        "",
        { "All Supported Files", "*.json *.jsonl *.yaml *.yml *.xml *.html *.htm",
          "JSON Files",  "*.json *.jsonl",
          "YAML Files",  "*.yaml *.yml",
          "XML Files",   "*.xml",
          "HTML Files",  "*.html *.htm",
          "All Files",   "*" }
    ).result();

    if (selection.empty())
        return {};
    return selection.front();
}

std::string saveFileDialog(const std::string& defaultName,
                           const std::string& filterDesc,
                           const std::string& filterExt) {
    return pfd::save_file(
        "Save File",
        defaultName,
        { filterDesc, filterExt,
          "All Files", "*" }
    ).result();
}

} // namespace fileio
