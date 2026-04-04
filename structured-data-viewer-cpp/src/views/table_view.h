#pragma once
#include "data/data_node.h"
#include <vector>
#include <functional>
#include <string>

struct TableRow {
    std::string pathStr;
    DataNode::Path path;
    std::string key;
    std::string value;
    std::string type;
    bool isContainer;
};

class TableView {
public:
    void render(const DataNode& root, bool isDark);

    std::function<void(const DataNode::Path&)> onEdit;
    std::function<void(const DataNode::Path&)> onAdd;
    std::function<void(const DataNode::Path&)> onDelete;

private:
    std::vector<TableRow> rows_;
    void flatten(const DataNode& node, const DataNode::Path& path, const std::string& key);
};
