#include "dnd/drag_drop.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace dnd {

// ---------------------------------------------------------------------------
// computeZone
// ---------------------------------------------------------------------------

Zone computeZone(float mouseY, float rectTop, float rectBottom, bool isContainer, bool isRoot) {
    if (isRoot) return Zone::Into;

    float height = rectBottom - rectTop;
    if (height <= 0.0f) return Zone::Before;

    float ratio = (mouseY - rectTop) / height;

    if (isContainer) {
        if (ratio < 0.3f) return Zone::Before;
        if (ratio > 0.7f) return Zone::After;
        return Zone::Into;
    } else {
        return ratio < 0.5f ? Zone::Before : Zone::After;
    }
}

// ---------------------------------------------------------------------------
// serializePath / deserializePath
// ---------------------------------------------------------------------------

std::string serializePath(const DataNode::Path& path) {
    std::string result;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) result += '/';
        const auto& seg = path[i];
        if (auto* s = std::get_if<std::string>(&seg)) {
            result += *s;
        } else {
            result += std::to_string(std::get<size_t>(seg));
        }
    }
    return result;
}

DataNode::Path deserializePath(const std::string& s) {
    DataNode::Path path;
    if (s.empty()) return path;

    std::istringstream stream(s);
    std::string token;
    while (std::getline(stream, token, '/')) {
        if (token.empty()) continue;
        // Check if all digits -> size_t index
        bool allDigits = !token.empty() && std::all_of(token.begin(), token.end(),
            [](unsigned char c) { return std::isdigit(c); });
        if (allDigits) {
            path.emplace_back(static_cast<size_t>(std::stoull(token)));
        } else {
            path.emplace_back(token);
        }
    }
    return path;
}

// ---------------------------------------------------------------------------
// isDescendantOrSelf
// ---------------------------------------------------------------------------

bool isDescendantOrSelf(const DataNode::Path& srcPath, const DataNode::Path& testPath) {
    if (testPath.size() < srcPath.size()) return false;
    for (size_t i = 0; i < srcPath.size(); ++i) {
        if (srcPath[i] != testPath[i]) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Helper: get parent path and last segment
// ---------------------------------------------------------------------------

static DataNode::Path parentOf(const DataNode::Path& path) {
    if (path.empty()) return {};
    return DataNode::Path(path.begin(), path.end() - 1);
}

// ---------------------------------------------------------------------------
// Helper: find index of a child within its parent container
// ---------------------------------------------------------------------------

static int findChildIndex(DataNode& root, const DataNode::Path& childPath) {
    if (childPath.empty()) return -1;
    DataNode::Path pp = parentOf(childPath);
    DataNode* parent = root.getAt(pp);
    if (!parent) return -1;

    const auto& lastSeg = childPath.back();
    if (parent->type() == NodeType::Array) {
        if (auto* idx = std::get_if<size_t>(&lastSeg)) {
            return static_cast<int>(*idx);
        }
        return -1;
    } else if (parent->type() == NodeType::Object) {
        if (auto* key = std::get_if<std::string>(&lastSeg)) {
            auto& obj = parent->asObject();
            for (size_t i = 0; i < obj.size(); ++i) {
                if (obj[i].first == *key) return static_cast<int>(i);
            }
        }
        return -1;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Helper: extract key from path (for objects)
// ---------------------------------------------------------------------------

static std::string keyFromPath(const DataNode::Path& path) {
    if (path.empty()) return "";
    if (auto* s = std::get_if<std::string>(&path.back())) {
        return *s;
    }
    return "";
}

// ---------------------------------------------------------------------------
// Helper: generate unique key for object if conflict
// ---------------------------------------------------------------------------

static std::string uniqueKey(const DataObject& obj, const std::string& desired) {
    std::string key = desired;
    bool exists = std::any_of(obj.begin(), obj.end(),
        [&](const auto& p) { return p.first == key; });
    int suffix = 1;
    while (exists) {
        key = desired + "_" + std::to_string(suffix++);
        exists = std::any_of(obj.begin(), obj.end(),
            [&](const auto& p) { return p.first == key; });
    }
    return key;
}

// ---------------------------------------------------------------------------
// moveItem
// ---------------------------------------------------------------------------

bool moveItem(DataNode& root, const DataNode::Path& srcPath, const DataNode::Path& dstPath, Zone zone) {
    // Cannot move root
    if (srcPath.empty()) return false;

    // Determine effective target path for ancestor check
    DataNode::Path targetCheckPath;
    if (zone == Zone::Into) {
        targetCheckPath = dstPath;
    } else {
        targetCheckPath = parentOf(dstPath);
    }

    // Prevent moving into self or descendant
    if (isDescendantOrSelf(srcPath, targetCheckPath)) return false;

    // Get the source item (clone it before removal)
    DataNode* srcNode = root.getAt(srcPath);
    if (!srcNode) return false;
    DataNode item = srcNode->clone();

    // Remember the source key (for objects)
    std::string srcKey = keyFromPath(srcPath);

    // Get source parent info for same-parent adjustment
    DataNode::Path srcParentPath = parentOf(srcPath);
    DataNode::Path dstParentPath;
    if (zone == Zone::Into) {
        dstParentPath = dstPath;
    } else {
        dstParentPath = parentOf(dstPath);
    }

    int srcIndex = findChildIndex(root, srcPath);

    // Remove source
    root.removeAt(srcPath);

    // Insert at destination
    if (zone == Zone::Into) {
        // Append to destination container
        DataNode* dst = root.getAt(dstPath);
        if (!dst || !dst->isContainer()) return false;

        if (dst->type() == NodeType::Array) {
            dst->asArray().push_back(std::move(item));
        } else {
            // Object: use source key, handle conflicts
            std::string key = uniqueKey(dst->asObject(), srcKey.empty() ? "item" : srcKey);
            dst->asObject().emplace_back(key, std::move(item));
        }
    } else {
        // Before/After: insert relative to dstPath sibling
        DataNode* parent = root.getAt(dstParentPath);
        if (!parent || !parent->isContainer()) return false;

        int dstIndex = -1;

        if (parent->type() == NodeType::Array) {
            // Find destination index in the (post-removal) array
            if (auto* idx = std::get_if<size_t>(&dstPath.back())) {
                dstIndex = static_cast<int>(*idx);
            }

            // Adjust if source was removed from the same parent before the dst
            bool sameParent = (srcParentPath == dstParentPath);
            if (sameParent && srcIndex >= 0 && srcIndex < dstIndex) {
                dstIndex--;
            }

            if (zone == Zone::After) dstIndex++;

            auto& arr = parent->asArray();
            if (dstIndex < 0) dstIndex = 0;
            if (dstIndex > static_cast<int>(arr.size())) dstIndex = static_cast<int>(arr.size());
            arr.insert(arr.begin() + dstIndex, std::move(item));
        } else if (parent->type() == NodeType::Object) {
            // Find destination index in object
            auto& obj = parent->asObject();
            if (auto* key = std::get_if<std::string>(&dstPath.back())) {
                for (size_t i = 0; i < obj.size(); ++i) {
                    if (obj[i].first == *key) {
                        dstIndex = static_cast<int>(i);
                        break;
                    }
                }
            }

            if (zone == Zone::After && dstIndex >= 0) dstIndex++;

            // Handle key conflict
            std::string insertKey = uniqueKey(obj, srcKey.empty() ? "item" : srcKey);

            if (dstIndex < 0) dstIndex = 0;
            if (dstIndex > static_cast<int>(obj.size())) dstIndex = static_cast<int>(obj.size());
            obj.insert(obj.begin() + dstIndex, std::make_pair(insertKey, std::move(item)));
        }
    }

    return true;
}

} // namespace dnd
