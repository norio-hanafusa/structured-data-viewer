#pragma once
#include "data/data_node.h"
#include <string>

namespace dnd {

enum class Zone { Before = 0, After = 1, Into = 2 };

// Compute drop zone from mouse Y position relative to item rect
Zone computeZone(float mouseY, float rectTop, float rectBottom, bool isContainer, bool isRoot);

// Serialize/deserialize path for ImGui drag-drop payload
std::string serializePath(const DataNode::Path& path);
DataNode::Path deserializePath(const std::string& s);

// Check if testPath is descendant of or equal to srcPath
bool isDescendantOrSelf(const DataNode::Path& srcPath, const DataNode::Path& testPath);

// Execute move operation on the data tree
// Returns true if move was performed
bool moveItem(DataNode& root, const DataNode::Path& srcPath, const DataNode::Path& dstPath, Zone zone);

} // namespace dnd
