#pragma once
#include "data/data_node.h"
#include "data/format_detect.h"
#include <string>
#include <functional>
#include <vector>

class EditorPanel {
public:
    EditorPanel();

    void render(float width, float height);

    // State
    std::string text;
    Format format = Format::JSON;

    // Callbacks
    std::function<void()> onParse;
    std::function<void()> onFormat;
    std::function<void()> onExport;
    std::function<void()> onClear;
    std::function<void()> onSample;
    std::function<void()> onUndo;
    std::function<void()> onRedo;

    int undoCount = 0;
    int redoCount = 0;
    std::string fileName;

    void markDirty() { bufDirty_ = true; }

private:
    static constexpr size_t kMaxBufSize = 4 * 1024 * 1024; // 4MB
    std::vector<char> buf_;  // heap-allocated buffer
    bool bufDirty_ = true;

    void syncBuf();
    void renderFormatTabs();
    void renderActionButtons();
};
