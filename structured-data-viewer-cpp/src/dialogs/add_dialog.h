#pragma once
#include "data/data_node.h"
#include <string>
#include <functional>

class AddDialog {
public:
    void open(const DataNode::Path& parentPath, bool parentIsArray);
    void render(bool isDark);
    bool isOpen() const { return open_; }

    std::function<void(const DataNode::Path&, const std::string& key, DataNode value)> onAdd;

private:
    bool open_ = false;
    bool needsOpen_ = false;
    DataNode::Path parentPath_;
    bool parentIsArray_ = false;
    char keyBuf_[256] = {};
    int typeIndex_ = 0;  // 0=string, 1=number, 2=boolean, 3=null, 4=object, 5=array
    char valueBuf_[4096] = {};
    bool boolValue_ = false;
    std::string pathLabel_;
};
