#pragma once
#include "data/data_node.h"
#include <string>

namespace parsers {

DataNode parseXML(const std::string& input);
std::string serializeXML(const DataNode& node, int indent = 2);
std::string formatXML(const std::string& input);

} // namespace parsers
