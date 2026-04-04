#include "views/tree_view.h"
#include "theme/catppuccin.h"
#include "imgui.h"
#include <cstring>
#include <sstream>

// ---------------------------------------------------------------------------
// DnD payload identifier
// ---------------------------------------------------------------------------
static constexpr const char* kDndType = "TREE_NODE_PATH";

// Serialize a path to a string for DnD payloads
static std::string serializePath(const DataNode::Path& path) {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) oss << '/';
        if (auto* s = std::get_if<std::string>(&path[i]))
            oss << "k:" << *s;
        else
            oss << "i:" << std::get<size_t>(path[i]);
    }
    return oss.str();
}

// Deserialize a path from a DnD payload string
static DataNode::Path deserializePath(const std::string& str) {
    DataNode::Path path;
    if (str.empty()) return path;

    std::istringstream iss(str);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
        if (segment.size() > 2 && segment[1] == ':') {
            if (segment[0] == 'k')
                path.emplace_back(segment.substr(2));
            else
                path.emplace_back(static_cast<size_t>(std::stoull(segment.substr(2))));
        }
    }
    return path;
}

// ---------------------------------------------------------------------------
// Row highlight: detect hover on current frame, draw on next frame (behind text)
// ---------------------------------------------------------------------------
static ImVec2 s_highlightMin = {0, 0};
static ImVec2 s_highlightMax = {0, 0};
static bool   s_hasHighlight = false;

static void checkRowHover() {
    ImVec2 rowMin = ImVec2(ImGui::GetWindowPos().x, ImGui::GetItemRectMin().y);
    ImVec2 rowMax = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetItemRectMax().y);
    ImVec2 mouse = ImGui::GetMousePos();
    if (mouse.x >= rowMin.x && mouse.x <= rowMax.x &&
        mouse.y >= rowMin.y && mouse.y <= rowMax.y) {
        s_highlightMin = rowMin;
        s_highlightMax = rowMax;
        s_hasHighlight = true;
    }
}

static void flushRowHighlight() {
    if (s_hasHighlight) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            s_highlightMin, s_highlightMax,
            ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered)));
        s_hasHighlight = false;
    }
}

// ---------------------------------------------------------------------------
// TreeView
// ---------------------------------------------------------------------------

void TreeView::render(const DataNode& root, bool isDark) {
    ImGui::BeginChild("TreeView", ImVec2(0, 0), false);
    flushRowHighlight();
    DataNode::Path rootPath;
    renderNode(root, "root", rootPath, true, isDark);
    ImGui::EndChild();
}

void TreeView::renderNode(const DataNode& node, const std::string& key,
                           const DataNode::Path& path, bool isRoot, bool isDark) {
    ImGui::PushID(static_cast<int>(std::hash<std::string>{}(pathToString(path))));

    if (node.isContainer()) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                 | ImGuiTreeNodeFlags_AllowOverlap
                                 | ImGuiTreeNodeFlags_SpanFullWidth;
        if (isRoot) flags |= ImGuiTreeNodeFlags_DefaultOpen;

        bool opened = ImGui::TreeNodeEx(key.c_str(), flags);
        checkRowHover();

        ImGui::SameLine();
        renderTypeBadge(node, isDark);

        ImGui::SameLine();
        renderActionButtons(path, true, isRoot);

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string payload = serializePath(path);
            ImGui::SetDragDropPayload(kDndType, payload.data(), payload.size() + 1);
            ImGui::Text("Move %s", key.c_str());
            ImGui::EndDragDropSource();
        }

        // Drop target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(kDndType)) {
                DataNode::Path srcPath = deserializePath(std::string(static_cast<const char*>(pl->Data)));
                float relY = (ImGui::GetMousePos().y - ImGui::GetItemRectMin().y) / ImGui::GetItemRectSize().y;
                int zone = relY < 0.25f ? 0 : relY > 0.75f ? 1 : 2;
                if (onMove) onMove(srcPath, path, zone);
            }
            ImGui::EndDragDropTarget();
        }

        if (opened) {
            if (node.type() == NodeType::Object) {
                const auto& obj = node.asObject();
                for (size_t i = 0; i < obj.size(); ++i) {
                    DataNode::Path cp = path;
                    cp.emplace_back(obj[i].first);
                    renderNode(obj[i].second, obj[i].first, cp, false, isDark);
                }
            } else {
                const auto& arr = node.asArray();
                for (size_t i = 0; i < arr.size(); ++i) {
                    DataNode::Path cp = path;
                    cp.emplace_back(i);
                    renderNode(arr[i], "[" + std::to_string(i) + "]", cp, false, isDark);
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf
                                 | ImGuiTreeNodeFlags_NoTreePushOnOpen
                                 | ImGuiTreeNodeFlags_AllowOverlap
                                 | ImGuiTreeNodeFlags_SpanFullWidth;

        ImGui::TreeNodeEx(key.c_str(), flags);
        checkRowHover();

        ImGui::SameLine();
        ImGui::TextUnformatted(":");
        ImGui::SameLine();
        renderLeafValue(node, isDark);

        ImGui::SameLine();
        renderTypeBadge(node, isDark);

        ImGui::SameLine();
        renderActionButtons(path, false, isRoot);

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string payload = serializePath(path);
            ImGui::SetDragDropPayload(kDndType, payload.data(), payload.size() + 1);
            ImGui::Text("Move %s", key.c_str());
            ImGui::EndDragDropSource();
        }

        // Drop target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(kDndType)) {
                DataNode::Path srcPath = deserializePath(std::string(static_cast<const char*>(pl->Data)));
                float relY = (ImGui::GetMousePos().y - ImGui::GetItemRectMin().y) / ImGui::GetItemRectSize().y;
                int zone = relY < 0.5f ? 0 : 1;
                if (onMove) onMove(srcPath, path, zone);
            }
            ImGui::EndDragDropTarget();
        }
    }

    ImGui::PopID();
}

void TreeView::renderLeafValue(const DataNode& node, bool isDark) {
    ImVec4 color = theme::colorForType(node.typeName(), isDark);

    switch (node.type()) {
        case NodeType::String:
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("\"%s\"", node.asString().c_str());
            ImGui::PopStyleColor();
            break;

        case NodeType::Integer:
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%lld", static_cast<long long>(node.asInt()));
            ImGui::PopStyleColor();
            break;

        case NodeType::Float:
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%g", node.asFloat());
            ImGui::PopStyleColor();
            break;

        case NodeType::Boolean:
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(node.asBool() ? "true" : "false");
            ImGui::PopStyleColor();
            break;

        case NodeType::Null:
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::PushFont(nullptr); // Use default; no italic API in ImGui directly
            ImGui::TextUnformatted("null");
            ImGui::PopFont();
            ImGui::PopStyleColor();
            break;

        default:
            ImGui::TextUnformatted(node.displayValue().c_str());
            break;
    }
}

void TreeView::renderTypeBadge(const DataNode& node, bool isDark) {
    ImVec4 color = theme::colorForType(node.typeName(), isDark);

    ImGui::PushStyleColor(ImGuiCol_Text, color);

    char badge[64];
    switch (node.type()) {
        case NodeType::Object:
            snprintf(badge, sizeof(badge), "Object (%zu)", node.childCount());
            break;
        case NodeType::Array:
            snprintf(badge, sizeof(badge), "Array (%zu)", node.childCount());
            break;
        default:
            snprintf(badge, sizeof(badge), "%s", node.typeName());
            break;
    }

    ImGui::SmallButton(badge);
    ImGui::PopStyleColor();
}

void TreeView::renderActionButtons(const DataNode::Path& path, bool isContainer, bool isRoot) {
    // Right-align action buttons by pushing the cursor
    float availWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = 0.0f;

    // Estimate width needed for buttons
    if (isContainer && !isRoot) {
        buttonWidth = ImGui::CalcTextSize("Add").x + ImGui::CalcTextSize("Delete").x
                    + ImGui::GetStyle().FramePadding.x * 4
                    + ImGui::GetStyle().ItemSpacing.x;
    } else if (isContainer && isRoot) {
        buttonWidth = ImGui::CalcTextSize("Add").x
                    + ImGui::GetStyle().FramePadding.x * 2;
    } else if (!isRoot) {
        buttonWidth = ImGui::CalcTextSize("Edit").x + ImGui::CalcTextSize("Delete").x
                    + ImGui::GetStyle().FramePadding.x * 4
                    + ImGui::GetStyle().ItemSpacing.x;
    }

    if (buttonWidth > 0.0f && availWidth > buttonWidth) {
        ImGui::SameLine(ImGui::GetCursorPosX() + availWidth - buttonWidth);
    }

    if (isContainer) {
        ImGui::PushID("add");
        if (ImGui::SmallButton("Add")) {
            if (onAdd) onAdd(path);
        }
        ImGui::PopID();

        if (!isRoot) {
            ImGui::SameLine();
            ImGui::PushID("del");
            if (ImGui::SmallButton("Delete")) {
                if (onDelete) onDelete(path);
            }
            ImGui::PopID();
        }
    } else {
        if (!isRoot) {
            ImGui::PushID("edit");
            if (ImGui::SmallButton("Edit")) {
                if (onEdit) onEdit(path);
            }
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::PushID("del");
            if (ImGui::SmallButton("Delete")) {
                if (onDelete) onDelete(path);
            }
            ImGui::PopID();
        }
    }
}
