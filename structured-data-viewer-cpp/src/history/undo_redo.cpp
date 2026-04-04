#include "history/undo_redo.h"

void UndoRedo::push(const Snapshot& snap) {
    // Clone the data so we hold an independent copy
    Snapshot copy;
    copy.data = snap.data.clone();
    copy.editorText = snap.editorText;
    copy.format = snap.format;
    copy.fileName = snap.fileName;

    undoStack_.push_back(std::move(copy));

    // Trim to max history
    if (static_cast<int>(undoStack_.size()) > kMaxHistory) {
        undoStack_.erase(undoStack_.begin());
    }

    // Any new push invalidates the redo stack
    redoStack_.clear();
}

std::optional<Snapshot> UndoRedo::undo(const Snapshot& current) {
    if (undoStack_.empty()) return std::nullopt;

    // Push current state onto redo stack
    Snapshot currentCopy;
    currentCopy.data = current.data.clone();
    currentCopy.editorText = current.editorText;
    currentCopy.format = current.format;
    currentCopy.fileName = current.fileName;
    redoStack_.push_back(std::move(currentCopy));

    // Pop from undo stack
    Snapshot restored = std::move(undoStack_.back());
    undoStack_.pop_back();
    return restored;
}

std::optional<Snapshot> UndoRedo::redo(const Snapshot& current) {
    if (redoStack_.empty()) return std::nullopt;

    // Push current state onto undo stack
    Snapshot currentCopy;
    currentCopy.data = current.data.clone();
    currentCopy.editorText = current.editorText;
    currentCopy.format = current.format;
    currentCopy.fileName = current.fileName;
    undoStack_.push_back(std::move(currentCopy));

    // Pop from redo stack
    Snapshot restored = std::move(redoStack_.back());
    redoStack_.pop_back();
    return restored;
}

void UndoRedo::clear() {
    undoStack_.clear();
    redoStack_.clear();
}
