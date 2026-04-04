#include "views/editor_panel.h"
#include "data/format_detect.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>

// ---------------------------------------------------------------------------
// EditorPanel
// ---------------------------------------------------------------------------

EditorPanel::EditorPanel() : buf_(kMaxBufSize, '\0') {}

void EditorPanel::syncBuf() {
    if (!bufDirty_) return;
    size_t len = std::min(text.size(), kMaxBufSize - 1);
    std::memcpy(buf_.data(), text.data(), len);
    buf_[len] = '\0';
    bufDirty_ = false;
}

void EditorPanel::render(float /*width*/, float /*height*/) {
    syncBuf();

    // Note: caller already wraps this in BeginChild/EndChild

    // File name header (if set)
    if (!fileName.empty()) {
        ImGui::TextDisabled("File: %s", fileName.c_str());
        ImGui::Separator();
    }

    // Format tabs
    renderFormatTabs();
    ImGui::Separator();

    // Calculate remaining height for the text editor area
    // Reserve space for the action buttons row at the bottom
    float buttonRowHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    float editorHeight = ImGui::GetContentRegionAvail().y - buttonRowHeight;

    // Multiline text editor
    if (editorHeight > 0) {
        if (ImGui::InputTextMultiline("##editor", buf_.data(), kMaxBufSize,
                                       ImVec2(-1.0f, editorHeight),
                                       ImGuiInputTextFlags_AllowTabInput)) {
            text = buf_.data();
            // Auto-detect format from content
            if (!text.empty()) {
                format = detectFormat(text);
            }
        }
    }

    // Action buttons
    renderActionButtons();
}

void EditorPanel::renderFormatTabs() {
    struct FormatTab {
        const char* label;
        Format fmt;
    };
    static const FormatTab tabs[] = {
        { "JSON",  Format::JSON  },
        { "JSONL", Format::JSONL },
        { "YAML",  Format::YAML  },
        { "XML",   Format::XML   },
        { "HTML",  Format::HTML  },
    };

    for (int i = 0; i < 5; ++i) {
        if (i > 0) ImGui::SameLine();

        bool isActive = (format == tabs[i].fmt);

        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        if (ImGui::SmallButton(tabs[i].label)) {
            format = tabs[i].fmt;
            bufDirty_ = true;
        }

        if (isActive) {
            ImGui::PopStyleColor();
        }
    }
}

void EditorPanel::renderActionButtons() {
    // Parse button (primary action)
    if (ImGui::Button("Parse (Ctrl+Enter)")) {
        if (onParse) onParse();
    }

    ImGui::SameLine();
    if (ImGui::Button("Format")) {
        if (onFormat) onFormat();
        bufDirty_ = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Export")) {
        if (onExport) onExport();
    }

    ImGui::SameLine();
    if (ImGui::Button("Sample")) {
        if (onSample) onSample();
        bufDirty_ = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        if (onClear) onClear();
        bufDirty_ = true;
    }

    ImGui::SameLine();

    // Undo with count
    {
        char label[64];
        snprintf(label, sizeof(label), "Undo (%d)", undoCount);
        bool disabled = (undoCount <= 0);
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::Button(label)) {
            if (onUndo) onUndo();
            bufDirty_ = true;
        }
        if (disabled) ImGui::EndDisabled();
    }

    ImGui::SameLine();

    // Redo with count
    {
        char label[64];
        snprintf(label, sizeof(label), "Redo (%d)", redoCount);
        bool disabled = (redoCount <= 0);
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::Button(label)) {
            if (onRedo) onRedo();
            bufDirty_ = true;
        }
        if (disabled) ImGui::EndDisabled();
    }
}
