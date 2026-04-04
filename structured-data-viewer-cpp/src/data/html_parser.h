#pragma once
#include "data/data_node.h"
#include <string>

namespace parsers {

DataNode parseHTML(const std::string& input);
std::string formatHTML(const std::string& input);

} // namespace parsers
