#include "views/table_view.h"
#include "theme/catppuccin.h"
#include "imgui.h"
#include <sstream>

// ---------------------------------------------------------------------------
// Path formatting helper: "users[0].name"
// ---------------------------------------------------------------------------
static std::string formatPath(const DataNode::Path& path) {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); ++i) {
        if (auto* s = std::get_if<std::string>(&path[i])) {
            if (i > 0) oss << '.';
            oss << *s;
        } else {
            oss << '[' << std::get<size_t>(path[i]) << ']';
        }
    }
    return oss.str();
}

// ---------------------------------------------------------------------------
// TableView
// ---------------------------------------------------------------------------

void TableView::flatten(const DataNode& node, const DataNode::Path& path, const std::string& key) {
    TableRow row;
    row.path = path;
    row.pathStr = formatPath(path);
    row.key = key;
    row.type = node.typeName();
    row.isContainer = node.isContainer();

    if (node.type() == NodeType::Object) {
        char buf[32];
        snprintf(buf, sizeof(buf), "{%zu}", node.childCount());
        row.value = buf;
    } else if (node.type() == NodeType::Array) {
        char buf[32];
        snprintf(buf, sizeof(buf), "[%zu]", node.childCount());
        row.value = buf;
    } else {
        row.value = node.displayValue();
    }

    rows_.push_back(std::move(row));

    // Recurse into children
    if (node.type() == NodeType::Object) {
        const auto& obj = node.asObject();
        for (size_t i = 0; i < obj.size(); ++i) {
            DataNode::Path childPath = path;
            childPath.emplace_back(obj[i].first);
            flatten(obj[i].second, childPath, obj[i].first);
        }
    } else if (node.type() == NodeType::Array) {
        const auto& arr = node.asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            DataNode::Path childPath = path;
            childPath.emplace_back(i);
            std::string label = "[" + std::to_string(i) + "]";
            flatten(arr[i], childPath, label);
        }
    }
}

void TableView::render(const DataNode& root, bool isDark) {
    // Rebuild flat row list
    rows_.clear();
    DataNode::Path rootPath;
    flatten(root, rootPath, "root");

    if (rows_.empty()) {
        ImGui::TextDisabled("No data to display.");
        return;
    }

    // Table with 5 columns
    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Borders
      | ImGuiTableFlags_RowBg
      | ImGuiTableFlags_Resizable
      | ImGuiTableFlags_ScrollY
      | ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("DataTable", 5, tableFlags)) return;

    ImGui::TableSetupScrollFreeze(0, 1); // freeze header row
    ImGui::TableSetupColumn("Path",    ImGuiTableColumnFlags_WidthStretch, 2.0f);
    ImGui::TableSetupColumn("Key",     ImGuiTableColumnFlags_WidthStretch, 1.5f);
    ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch, 2.0f);
    ImGui::TableSetupColumn("Type",    ImGuiTableColumnFlags_WidthFixed,   70.0f);
    ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed,  120.0f);
    ImGui::TableHeadersRow();

    // Use clipper for large datasets
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rows_.size()));

    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
            const auto& row = rows_[static_cast<size_t>(i)];
            ImGui::TableNextRow();
            ImGui::PushID(i);

            // Column 0: Path (purple)
            ImGui::TableNextColumn();
            {
                ImVec4 purple = isDark
                    ? theme::getMochaAccents().purple
                    : theme::getLatteAccents().purple;
                ImGui::PushStyleColor(ImGuiCol_Text, purple);
                ImGui::TextUnformatted(row.pathStr.c_str());
                ImGui::PopStyleColor();
            }

            // Column 1: Key
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(row.key.c_str());

            // Column 2: Value (color-coded by type)
            ImGui::TableNextColumn();
            {
                ImVec4 color = theme::colorForType(row.type.c_str(), isDark);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(row.value.c_str());
                ImGui::PopStyleColor();
            }

            // Column 3: Type badge
            ImGui::TableNextColumn();
            {
                ImVec4 color = theme::colorForType(row.type.c_str(), isDark);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::SmallButton(row.type.c_str());
                ImGui::PopStyleColor();
            }

            // Column 4: Actions
            ImGui::TableNextColumn();
            if (row.isContainer) {
                if (ImGui::SmallButton("Add")) {
                    if (onAdd) onAdd(row.path);
                }
                if (!row.path.empty()) {
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Delete")) {
                        if (onDelete) onDelete(row.path);
                    }
                }
            } else {
                if (!row.path.empty()) {
                    if (ImGui::SmallButton("Edit")) {
                        if (onEdit) onEdit(row.path);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Delete")) {
                        if (onDelete) onDelete(row.path);
                    }
                }
            }

            ImGui::PopID();
        }
    }

    ImGui::EndTable();
}
