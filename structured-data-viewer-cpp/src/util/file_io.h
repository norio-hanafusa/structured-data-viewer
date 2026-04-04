#pragma once
#include <string>
#include <optional>

namespace fileio {

std::optional<std::string> readFile(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
std::string openFileDialog(const std::string& title = "Open File");
std::string saveFileDialog(const std::string& defaultName,
                           const std::string& filterDesc,
                           const std::string& filterExt);

} // namespace fileio
