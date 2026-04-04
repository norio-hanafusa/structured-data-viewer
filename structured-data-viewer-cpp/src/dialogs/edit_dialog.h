#pragma once
#include "data/data_node.h"
#include <string>
#include <functional>

class EditDialog {
public:
    void open(const DataNode::Path& path, const DataNode& currentValue);
    void render(bool isDark);
    bool isOpen() const { return open_; }

    std::function<void(const DataNode::Path&, DataNode newValue)> onSave;

private:
    bool open_ = false;
    bool needsOpen_ = false;
    DataNode::Path path_;
    int typeIndex_ = 0;       // 0=string, 1=number, 2=boolean, 3=null
    char valueBuf_[4096] = {};
    bool boolValue_ = false;
    std::string pathLabel_;
};
