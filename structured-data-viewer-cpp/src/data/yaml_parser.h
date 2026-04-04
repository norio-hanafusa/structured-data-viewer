#pragma once
#include "data/data_node.h"
#include <string>

namespace parsers {

DataNode parseYAML(const std::string& input);
std::string serializeYAML(const DataNode& node);
std::string formatYAML(const std::string& input);

} // namespace parsers
