#pragma once

#include "data/data_node.h"
#include "data/format_detect.h"
#include "views/editor_panel.h"
#include "views/tree_view.h"
#include "views/table_view.h"
#include "dialogs/edit_dialog.h"
#include "dialogs/add_dialog.h"
#include "dialogs/delete_dialog.h"
#include "history/undo_redo.h"
#include "stats/statistics.h"

#include <string>
#include <vector>

class App {
public:
    App();

    void render();           // Called each frame
    void onFileDrop(const std::vector<std::string>& paths);

private:
    // Data
    DataNode parsedData_;
    bool hasParsedData_ = false;

    // Dirty / save state
    bool dirty_ = false;
    std::string currentFilePath_;   // empty = untitled (never saved / not from file)

    // Save confirmation dialog
    bool showSaveAlert_ = false;
    std::function<void()> pendingAction_;
    bool closeRequested_ = false;

    // UI state
    bool isDark_ = true;
    bool showTree_ = true;
    float leftPanelWidth_ = 450.0f;
    bool resizing_ = false;

    // Modules
    EditorPanel editor_;
    TreeView treeView_;
    TableView tableView_;
    EditDialog editDialog_;
    AddDialog addDialog_;
    DeleteDialog deleteDialog_;
    UndoRedo history_;
    stats::Stats currentStats_;

    // Toast
    std::string toastMsg_;
    float toastTimer_ = 0.0f;
    bool toastIsError_ = false;

    // Actions
    void doParse();
    void doFormat();
    void doExport();
    void doClear();
    void doSample();
    void doUndo();
    void doRedo();
    void pushHistory();

    void showToast(const std::string& msg, bool isError = false);
    void renderHeader();
    void renderToast();
    void renderSaveAlert();
    void handleKeyboardShortcuts();

    void syncEditorFromData();
    void doSave();
    void confirmIfDirty(std::function<void()> action);
    void markDirty();

public:
    bool shouldClose() const { return closeRequested_ && !showSaveAlert_; }
    void requestClose();
};
