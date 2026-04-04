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

    // UI state
    bool isDark_ = true;
    bool showTree_ = true;   // true = tree view, false = table view
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
    void handleKeyboardShortcuts();

    // Sync editor text from parsedData
    void syncEditorFromData();
};
