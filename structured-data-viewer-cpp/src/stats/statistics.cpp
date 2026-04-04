#include "stats/statistics.h"
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace stats {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static int countKeys(const DataNode& node) {
    int count = 0;
    switch (node.type()) {
        case NodeType::Object:
            for (const auto& [key, child] : node.asObject()) {
                count += 1;                   // this key
                count += countKeys(child);    // recurse
            }
            break;
        case NodeType::Array:
            for (const auto& child : node.asArray()) {
                count += countKeys(child);    // array indices are NOT keys
            }
            break;
        default:
            break;
    }
    return count;
}

static int calcMaxDepth(const DataNode& node) {
    switch (node.type()) {
        case NodeType::Object: {
            int deepest = 0;
            for (const auto& [key, child] : node.asObject())
                deepest = std::max(deepest, calcMaxDepth(child));
            return 1 + deepest;
        }
        case NodeType::Array: {
            int deepest = 0;
            for (const auto& child : node.asArray())
                deepest = std::max(deepest, calcMaxDepth(child));
            return 1 + deepest;
        }
        default:
            return 1;
    }
}

static int countNodes(const DataNode& node) {
    int count = 1; // this node
    switch (node.type()) {
        case NodeType::Object:
            for (const auto& [key, child] : node.asObject())
                count += countNodes(child);
            break;
        case NodeType::Array:
            for (const auto& child : node.asArray())
                count += countNodes(child);
            break;
        default:
            break;
    }
    return count;
}

static void collectUniqueKeys(const DataNode& node, std::set<std::string>& keys) {
    switch (node.type()) {
        case NodeType::Object:
            for (const auto& [key, child] : node.asObject()) {
                keys.insert(key);
                collectUniqueKeys(child, keys);
            }
            break;
        case NodeType::Array:
            for (const auto& child : node.asArray())
                collectUniqueKeys(child, keys);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

Stats compute(const DataNode& root, size_t rawSize) {
    Stats s;
    s.keyCount  = countKeys(root);
    s.maxDepth  = calcMaxDepth(root);
    s.nodeCount = countNodes(root);

    std::set<std::string> keys;
    collectUniqueKeys(root, keys);
    s.uniqueKeyCount = static_cast<int>(keys.size());

    s.byteSize = rawSize;
    return s;
}

std::string formatBytes(size_t bytes) {
    std::ostringstream oss;

    if (bytes < 1024) {
        oss << bytes << " B";
    } else if (bytes < 1024 * 1024) {
        double kb = static_cast<double>(bytes) / 1024.0;
        if (kb < 10.0)
            oss << std::fixed << std::setprecision(1) << kb << " KB";
        else
            oss << static_cast<int>(kb) << " KB";
    } else if (bytes < 1024ULL * 1024 * 1024) {
        double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
        if (mb < 10.0)
            oss << std::fixed << std::setprecision(1) << mb << " MB";
        else
            oss << static_cast<int>(mb) << " MB";
    } else {
        double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
        if (gb < 10.0)
            oss << std::fixed << std::setprecision(1) << gb << " GB";
        else
            oss << static_cast<int>(gb) << " GB";
    }

    return oss.str();
}

} // namespace stats
