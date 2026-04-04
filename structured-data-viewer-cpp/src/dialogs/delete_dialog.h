#pragma once
#include "data/data_node.h"
#include <string>
#include <functional>

class DeleteDialog {
public:
    void open(const DataNode::Path& path);
    void render();
    bool isOpen() const { return open_; }

    std::function<void(const DataNode::Path&)> onDelete;

private:
    bool open_ = false;
    bool needsOpen_ = false;
    DataNode::Path path_;
    std::string pathLabel_;
};
