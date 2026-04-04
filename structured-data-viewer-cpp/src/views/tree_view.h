#pragma once
#include "data/data_node.h"
#include <functional>
#include <string>

class TreeView {
public:
    void render(const DataNode& root, bool isDark);

    // Action callbacks
    std::function<void(const DataNode::Path&)> onEdit;
    std::function<void(const DataNode::Path&)> onAdd;
    std::function<void(const DataNode::Path&)> onDelete;
    std::function<void(const DataNode::Path&, const DataNode::Path&, int zone)> onMove;
    // zone: 0=before, 1=after, 2=into

private:
    void renderNode(const DataNode& node, const std::string& key,
                    const DataNode::Path& path, bool isRoot, bool isDark);
    void renderLeafValue(const DataNode& node, bool isDark);
    void renderTypeBadge(const DataNode& node, bool isDark);
    void renderActionButtons(const DataNode::Path& path, bool isContainer, bool isRoot);
};
