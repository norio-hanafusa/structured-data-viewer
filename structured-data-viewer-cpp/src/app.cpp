#include "app.h"
#include "data/json_parser.h"
#include "data/yaml_parser.h"
#include "data/xml_parser.h"
#include "data/html_parser.h"
#include "data/format_detect.h"
#include "theme/catppuccin.h"
#include "dnd/drag_drop.h"
#include "util/file_io.h"
#include "util/string_util.h"

#include "imgui.h"
#include <algorithm>
#include <filesystem>

// Sample data for each format
static const char* kSampleJSON = R"({
  "project": "Structured Data Viewer",
  "version": "1.0.0",
  "features": ["tree view", "table view", "editing", "drag-drop"],
  "author": {
    "name": "Developer",
    "active": true
  },
  "stars": 42,
  "license": null
})";

static const char* kSampleJSONL =
    R"({"id": 1, "name": "Alice", "role": "engineer"})"  "\n"
    R"({"id": 2, "name": "Bob", "role": "designer"})"    "\n"
    R"({"id": 3, "name": "Charlie", "role": "manager"})";

static const char* kSampleYAML = R"(project: Structured Data Viewer
version: 1.0.0
features:
  - tree view
  - table view
  - editing
  - drag-drop
author:
  name: Developer
  active: true
stars: 42
license: null
)";

static const char* kSampleXML = R"(<?xml version="1.0" encoding="UTF-8"?>
<project name="Structured Data Viewer" version="1.0.0">
  <features>
    <feature>tree view</feature>
    <feature>table view</feature>
    <feature>editing</feature>
    <feature>drag-drop</feature>
  </features>
  <author active="true">
    <name>Developer</name>
  </author>
  <stars>42</stars>
</project>)";

static const char* kSampleHTML = R"(<!DOCTYPE html>
<html>
<head>
  <title>Structured Data Viewer</title>
</head>
<body>
  <h1>Hello World</h1>
  <p>This is a sample HTML document.</p>
  <ul>
    <li>Feature 1</li>
    <li>Feature 2</li>
  </ul>
</body>
</html>)";

App::App() {
    // Wire editor callbacks
    editor_.onParse  = [this]() { doParse(); };
    editor_.onFormat = [this]() { doFormat(); };
    editor_.onExport = [this]() { doExport(); };
    editor_.onClear  = [this]() { doClear(); };
    editor_.onSample = [this]() { doSample(); };
    editor_.onUndo   = [this]() { doUndo(); };
    editor_.onRedo   = [this]() { doRedo(); };

    // Wire tree view callbacks
    treeView_.onEdit = [this](const DataNode::Path& p) {
        if (auto* node = parsedData_.getAt(p))
            editDialog_.open(p, *node);
    };
    treeView_.onAdd = [this](const DataNode::Path& p) {
        if (auto* node = parsedData_.getAt(p))
            addDialog_.open(p, node->type() == NodeType::Array);
    };
    treeView_.onDelete = [this](const DataNode::Path& p) {
        deleteDialog_.open(p);
    };
    treeView_.onMove = [this](const DataNode::Path& src, const DataNode::Path& dst, int zone) {
        pushHistory();
        if (dnd::moveItem(parsedData_, src, dst, static_cast<dnd::Zone>(zone))) {
            syncEditorFromData();
            currentStats_ = stats::compute(parsedData_, editor_.text.size());
            markDirty();
        }
    };

    // Wire table view callbacks
    tableView_.onEdit = treeView_.onEdit;
    tableView_.onAdd  = treeView_.onAdd;
    tableView_.onDelete = treeView_.onDelete;

    // Wire dialog callbacks
    editDialog_.onSave = [this](const DataNode::Path& p, DataNode val) {
        pushHistory();
        parsedData_.setAt(p, std::move(val));
        syncEditorFromData();
        currentStats_ = stats::compute(parsedData_, editor_.text.size());
        markDirty();
    };
    addDialog_.onAdd = [this](const DataNode::Path& p, const std::string& key, DataNode val) {
        pushHistory();
        parsedData_.insertAt(p, std::move(val), key);
        syncEditorFromData();
        currentStats_ = stats::compute(parsedData_, editor_.text.size());
        markDirty();
    };
    deleteDialog_.onDelete = [this](const DataNode::Path& p) {
        pushHistory();
        parsedData_.removeAt(p);
        syncEditorFromData();
        currentStats_ = stats::compute(parsedData_, editor_.text.size());
        markDirty();
    };

}

void App::render() {
    // Apply theme on first frame
    static bool firstFrame = true;
    if (firstFrame) {
        theme::applyMocha(ImGui::GetStyle());
        firstFrame = false;
    }

    handleKeyboardShortcuts();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("##MainWindow", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    renderHeader();

    float headerHeight = ImGui::GetCursorPosY();
    float availH = viewport->WorkSize.y - headerHeight - ImGui::GetStyle().WindowPadding.y;
    float availW = viewport->WorkSize.x - ImGui::GetStyle().WindowPadding.x * 2;

    // Clamp panel width
    leftPanelWidth_ = std::clamp(leftPanelWidth_, 250.0f, std::max(251.0f, availW - 300.0f));

    // Left panel: Editor
    ImGui::BeginChild("##EditorPanel", ImVec2(leftPanelWidth_, availH), ImGuiChildFlags_Borders);
    editor_.undoCount = history_.undoCount();
    editor_.redoCount = history_.redoCount();
    editor_.render(leftPanelWidth_, availH);
    ImGui::EndChild();

    // Resize handle
    ImGui::SameLine();
    ImGui::InvisibleButton("##Splitter", ImVec2(6.0f, availH));
    if (ImGui::IsItemActive()) {
        leftPanelWidth_ += ImGui::GetIO().MouseDelta.x;
        resizing_ = true;
    } else {
        resizing_ = false;
    }
    if (ImGui::IsItemHovered() || resizing_) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    // Right panel: Preview
    ImGui::SameLine();
    float rightW = std::max(100.0f, availW - leftPanelWidth_ - 6.0f);
    ImGui::BeginChild("##PreviewPanel", ImVec2(rightW, availH), ImGuiChildFlags_Borders);

    if (hasParsedData_) {
        // View toggle tabs
        if (ImGui::BeginTabBar("##ViewTabs")) {
            if (ImGui::BeginTabItem("Tree")) {
                showTree_ = true;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Table")) {
                showTree_ = false;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::BeginChild("##ViewContent", ImVec2(0, 0), ImGuiChildFlags_None);
        if (showTree_) {
            treeView_.render(parsedData_, isDark_);
        } else {
            tableView_.render(parsedData_, isDark_);
        }
        ImGui::EndChild();
    } else {
        ImGui::TextDisabled("Paste data and click Parse to display.");
    }

    ImGui::EndChild();

    // Dialogs
    editDialog_.render(isDark_);
    addDialog_.render(isDark_);
    deleteDialog_.render();

    // Save alert dialog
    renderSaveAlert();

    // Toast
    renderToast();

    ImGui::End();
}

void App::renderHeader() {
    // Logo + title
    ImGui::PushStyleColor(ImGuiCol_Text,
        isDark_ ? ImVec4(0.537f, 0.705f, 0.980f, 1.0f)
                : ImVec4(0.118f, 0.400f, 0.961f, 1.0f));
    ImGui::Text("Structured Data Viewer");
    ImGui::PopStyleColor();

    // Action buttons on the same line as title
    ImGui::SameLine();
    ImGui::Spacing(); ImGui::SameLine();

    if (ImGui::SmallButton("Save")) { doSave(); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Export")) { doExport(); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Sample")) { confirmIfDirty([this]() { doSample(); }); }
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) { confirmIfDirty([this]() { doClear(); }); }

    ImGui::SameLine();
    ImGui::Spacing(); ImGui::SameLine();

    {
        char label[64];
        snprintf(label, sizeof(label), "Undo(%d)", history_.undoCount());
        bool disabled = (history_.undoCount() <= 0);
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::SmallButton(label)) { doUndo(); }
        if (disabled) ImGui::EndDisabled();
    }
    ImGui::SameLine();
    {
        char label[64];
        snprintf(label, sizeof(label), "Redo(%d)", history_.redoCount());
        bool disabled = (history_.redoCount() <= 0);
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::SmallButton(label)) { doRedo(); }
        if (disabled) ImGui::EndDisabled();
    }

    // Theme toggle (right-aligned)
    ImGui::SameLine();
    float themeW = ImGui::CalcTextSize(isDark_ ? "Light" : "Dark").x + ImGui::GetStyle().FramePadding.x * 2;
    ImGui::SameLine(ImGui::GetContentRegionMax().x - themeW);
    if (ImGui::SmallButton(isDark_ ? "Light" : "Dark")) {
        isDark_ = !isDark_;
        if (isDark_)
            theme::applyMocha(ImGui::GetStyle());
        else
            theme::applyLatte(ImGui::GetStyle());
    }

    // Stats on second row
    if (hasParsedData_) {
        char statsBuf[256];
        snprintf(statsBuf, sizeof(statsBuf), "%d keys | depth %d | %d nodes | %d unique | %s",
            currentStats_.keyCount, currentStats_.maxDepth,
            currentStats_.nodeCount, currentStats_.uniqueKeyCount,
            stats::formatBytes(currentStats_.byteSize).c_str());
        float statsW = ImGui::CalcTextSize(statsBuf).x;
        float rightX = ImGui::GetContentRegionMax().x - statsW;
        if (rightX > 0) {
            ImGui::SameLine(rightX);
            ImGui::TextDisabled("%s", statsBuf);
        }
    }

    ImGui::Separator();
}

void App::doParse() {
    const auto& input = editor_.text;
    if (input.empty()) return;

    try {
        switch (editor_.format) {
            case Format::JSON:  parsedData_ = parsers::parseJSON(input); break;
            case Format::JSONL: parsedData_ = parsers::parseJSONL(input); break;
            case Format::YAML:  parsedData_ = parsers::parseYAML(input); break;
            case Format::XML:   parsedData_ = parsers::parseXML(input); break;
            case Format::HTML:  parsedData_ = parsers::parseHTML(input); break;
        }
        hasParsedData_ = true;
        currentStats_ = stats::compute(parsedData_, input.size());
        history_.clear();
        markDirty();
    } catch (const std::exception& e) {
        hasParsedData_ = false;
        showToast(std::string("Parse error: ") + e.what(), true);
    }
}

void App::doFormat() {
    if (editor_.text.empty()) return;
    pushHistory();
    try {
        switch (editor_.format) {
            case Format::JSON:  editor_.text = parsers::formatJSON(editor_.text); break;
            case Format::JSONL: editor_.text = parsers::formatJSONL(editor_.text); break;
            case Format::YAML:  editor_.text = parsers::formatYAML(editor_.text); break;
            case Format::XML:   editor_.text = parsers::formatXML(editor_.text); break;
            case Format::HTML:  editor_.text = parsers::formatHTML(editor_.text); break;
        }
    } catch (...) {}
}

void App::doExport() {
    if (editor_.text.empty()) {
        showToast("Nothing to export", true);
        return;
    }
    std::string ext = formatExtension(editor_.format);
    std::string desc = std::string(formatName(editor_.format)) + " file";

    std::string defaultName;
    if (!editor_.fileName.empty()) {
        defaultName = editor_.fileName + "." + ext;
    } else {
        // Use timestamp
        auto t = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
        defaultName = std::string(buf) + "." + ext;
    }

    std::string path = fileio::saveFileDialog(defaultName, desc, "*." + ext);
    if (!path.empty()) {
        if (fileio::writeFile(path, editor_.text)) {
            showToast("Exported: " + std::filesystem::path(path).filename().string());
        } else {
            showToast("Export failed", true);
        }
    }
}

void App::doClear() {
    if (editor_.text.empty() && !hasParsedData_) return;
    pushHistory();
    editor_.text.clear();
    editor_.fileName.clear();
    editor_.markDirty();
    parsedData_ = DataNode();
    hasParsedData_ = false;
    currentStats_ = {};
    currentFilePath_.clear();
    dirty_ = false;
}

void App::doSample() {
    pushHistory();
    switch (editor_.format) {
        case Format::JSON:  editor_.text = kSampleJSON;  break;
        case Format::JSONL: editor_.text = kSampleJSONL; break;
        case Format::YAML:  editor_.text = kSampleYAML;  break;
        case Format::XML:   editor_.text = kSampleXML;   break;
        case Format::HTML:  editor_.text = kSampleHTML;   break;
    }
    editor_.markDirty();
    doParse();
}

void App::doUndo() {
    Snapshot current{
        hasParsedData_ ? parsedData_.clone() : DataNode(),
        editor_.text, editor_.format, editor_.fileName
    };
    if (auto snap = history_.undo(current)) {
        parsedData_ = std::move(snap->data);
        editor_.text = std::move(snap->editorText);
        editor_.format = snap->format;
        editor_.fileName = std::move(snap->fileName);
        editor_.markDirty();
        hasParsedData_ = parsedData_.type() != NodeType::Null || parsedData_.isContainer();
        if (hasParsedData_)
            currentStats_ = stats::compute(parsedData_, editor_.text.size());
    }
}

void App::doRedo() {
    Snapshot current{
        hasParsedData_ ? parsedData_.clone() : DataNode(),
        editor_.text, editor_.format, editor_.fileName
    };
    if (auto snap = history_.redo(current)) {
        parsedData_ = std::move(snap->data);
        editor_.text = std::move(snap->editorText);
        editor_.format = snap->format;
        editor_.fileName = std::move(snap->fileName);
        editor_.markDirty();
        hasParsedData_ = parsedData_.type() != NodeType::Null || parsedData_.isContainer();
        if (hasParsedData_)
            currentStats_ = stats::compute(parsedData_, editor_.text.size());
    }
}

void App::pushHistory() {
    Snapshot snap{
        hasParsedData_ ? parsedData_.clone() : DataNode(),
        editor_.text, editor_.format, editor_.fileName
    };
    history_.push(snap);
}

void App::syncEditorFromData() {
    if (!hasParsedData_) return;
    try {
        switch (editor_.format) {
            case Format::JSON:
                editor_.text = parsers::serializeJSON(parsedData_, 2);
                break;
            case Format::JSONL:
                editor_.text = parsers::serializeJSONL(parsedData_);
                break;
            case Format::YAML:
                editor_.text = parsers::serializeYAML(parsedData_);
                break;
            default:
                break;
        }
        editor_.markDirty();
    } catch (...) {}
}

void App::onFileDrop(const std::vector<std::string>& paths) {
    if (paths.empty()) return;

    auto loadFile = [this, filePath = paths[0]]() {
        auto content = fileio::readFile(filePath);
        if (!content) {
            showToast("Failed to read file", true);
            return;
        }

        pushHistory();

        std::filesystem::path fp(filePath);
        Format fmt = detectFormatFromExtension(fp.extension().string());
        editor_.format = fmt;
        editor_.text = *content;
        editor_.fileName = fp.stem().string();
        editor_.markDirty();
        currentFilePath_ = filePath;

        doParse();
        showToast("Loaded: " + fp.filename().string());
    };

    confirmIfDirty(loadFile);
}

void App::showToast(const std::string& msg, bool isError) {
    toastMsg_ = msg;
    toastTimer_ = 2.5f;
    toastIsError_ = isError;
}

void App::renderToast() {
    if (toastTimer_ <= 0.0f) return;
    toastTimer_ -= ImGui::GetIO().DeltaTime;

    float alpha = std::min(toastTimer_, 1.0f);
    ImVec4 bg = toastIsError_
        ? ImVec4(0.8f, 0.2f, 0.2f, 0.9f * alpha)
        : ImVec4(0.2f, 0.7f, 0.3f, 0.9f * alpha);

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 textSize = ImGui::CalcTextSize(toastMsg_.c_str());
    float padX = 20.0f, padY = 10.0f;
    float x = vp->WorkPos.x + (vp->WorkSize.x - textSize.x - padX * 2) * 0.5f;
    float y = vp->WorkPos.y + vp->WorkSize.y - textSize.y - padY * 2 - 30.0f;

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##Toast", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(
        ImVec2(p.x - padX, p.y - padY),
        ImVec2(p.x + textSize.x + padX, p.y + textSize.y + padY),
        ImGui::ColorConvertFloat4ToU32(bg), 8.0f);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
    ImGui::Text("%s", toastMsg_.c_str());
    ImGui::PopStyleColor();

    ImGui::End();
}

void App::handleKeyboardShortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    // Don't handle shortcuts when typing in a text input
    // (ImGui handles this via WantCaptureKeyboard, but we still want Ctrl+Enter in editor)
    bool ctrl = io.KeyCtrl;
    bool shift = io.KeyShift;

    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        doParse();
    }
    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        doUndo();
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        doRedo();
    }
    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        doRedo();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        // Close any open dialog
        if (editDialog_.isOpen() || addDialog_.isOpen() || deleteDialog_.isOpen()) {
            ImGui::CloseCurrentPopup();
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        doSave();
    }
}

void App::markDirty() {
    dirty_ = true;
}

void App::doSave() {
    if (editor_.text.empty()) return;

    if (!currentFilePath_.empty()) {
        // Overwrite existing file
        if (fileio::writeFile(currentFilePath_, editor_.text)) {
            dirty_ = false;
            showToast("Saved: " + std::filesystem::path(currentFilePath_).filename().string());
        } else {
            showToast("Save failed", true);
        }
    } else {
        // Save As
        std::string ext = formatExtension(editor_.format);
        std::string desc = std::string(formatName(editor_.format)) + " file";
        std::string defaultName;
        if (!editor_.fileName.empty()) {
            defaultName = editor_.fileName + "." + ext;
        } else {
            auto t = std::time(nullptr);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
            defaultName = std::string(buf) + "." + ext;
        }
        std::string path = fileio::saveFileDialog(defaultName, desc, "*." + ext);
        if (!path.empty()) {
            if (fileio::writeFile(path, editor_.text)) {
                currentFilePath_ = path;
                editor_.fileName = std::filesystem::path(path).stem().string();
                dirty_ = false;
                showToast("Saved: " + std::filesystem::path(path).filename().string());
            } else {
                showToast("Save failed", true);
            }
        }
    }
}

void App::confirmIfDirty(std::function<void()> action) {
    if (dirty_) {
        pendingAction_ = std::move(action);
        showSaveAlert_ = true;
    } else {
        action();
    }
}

void App::requestClose() {
    if (dirty_) {
        pendingAction_ = [this]() { closeRequested_ = true; };
        showSaveAlert_ = true;
    } else {
        closeRequested_ = true;
    }
}

void App::renderSaveAlert() {
    if (!showSaveAlert_) return;

    if (!ImGui::IsPopupOpen("Unsaved Changes"))
        ImGui::OpenPopup("Unsaved Changes");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    float dlgW = ImGui::GetMainViewport()->WorkSize.x * 0.25f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(dlgW, 0), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes.");
        ImGui::Text("Do you want to save before continuing?");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float bw = ImGui::CalcTextSize("Don't Save").x + ImGui::GetStyle().FramePadding.x * 2 + 8.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float total = bw * 3 + spacing * 2;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - total) * 0.5f);

        // Save button - blue/accent
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.3f, 0.7f, 1.0f));
        if (ImGui::Button("Save", ImVec2(bw, 0))) {
            doSave();
            if (!dirty_ && pendingAction_) {
                pendingAction_();
            }
            pendingAction_ = nullptr;
            showSaveAlert_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        // Don't Save button - red
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Don't Save", ImVec2(bw, 0))) {
            if (pendingAction_) pendingAction_();
            pendingAction_ = nullptr;
            showSaveAlert_ = false;
            dirty_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(bw, 0))) {
            pendingAction_ = nullptr;
            showSaveAlert_ = false;
            closeRequested_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
