#pragma once
#include "data/data_node.h"
#include <string>

namespace stats {

struct Stats {
    int keyCount = 0;
    int maxDepth = 0;
    int nodeCount = 0;
    int uniqueKeyCount = 0;
    size_t byteSize = 0;
};

Stats compute(const DataNode& root, size_t rawSize = 0);
std::string formatBytes(size_t bytes);

} // namespace stats
