#pragma once
#include "data/data_node.h"
#include "data/format_detect.h"
#include <vector>
#include <string>
#include <optional>

struct Snapshot {
    DataNode data;
    std::string editorText;
    Format format;
    std::string fileName;
};

class UndoRedo {
public:
    static constexpr int kMaxHistory = 50;

    void push(const Snapshot& snap);
    std::optional<Snapshot> undo(const Snapshot& current);
    std::optional<Snapshot> redo(const Snapshot& current);
    void clear();

    int undoCount() const { return static_cast<int>(undoStack_.size()); }
    int redoCount() const { return static_cast<int>(redoStack_.size()); }

private:
    std::vector<Snapshot> undoStack_;
    std::vector<Snapshot> redoStack_;
};
