#pragma once
#include "data/data_node.h"
#include <string>

namespace parsers {

DataNode parseJSON(const std::string& input);
DataNode parseJSONL(const std::string& input);
std::string serializeJSON(const DataNode& node, int indent = 2);
std::string serializeJSONL(const DataNode& node);  // array of objects, one per line
std::string formatJSON(const std::string& input);
std::string formatJSONL(const std::string& input);

} // namespace parsers
