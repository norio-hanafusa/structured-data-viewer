#include "dialogs/edit_dialog.h"
#include "imgui.h"
#include <cstring>
#include <cstdlib>
#include <string>

void EditDialog::open(const DataNode::Path& path, const DataNode& currentValue) {
    path_ = path;
    pathLabel_ = pathToString(path);
    if (pathLabel_.empty()) pathLabel_ = "(root)";

    std::memset(valueBuf_, 0, sizeof(valueBuf_));
    boolValue_ = false;

    switch (currentValue.type()) {
        case NodeType::String:
            typeIndex_ = 0;
            std::strncpy(valueBuf_, currentValue.asString().c_str(), sizeof(valueBuf_) - 1);
            break;
        case NodeType::Integer:
            typeIndex_ = 1;
            std::strncpy(valueBuf_, std::to_string(currentValue.asInt()).c_str(), sizeof(valueBuf_) - 1);
            break;
        case NodeType::Float:
            typeIndex_ = 1;
            std::strncpy(valueBuf_, std::to_string(currentValue.asFloat()).c_str(), sizeof(valueBuf_) - 1);
            break;
        case NodeType::Boolean:
            typeIndex_ = 2;
            boolValue_ = currentValue.asBool();
            break;
        case NodeType::Null:
            typeIndex_ = 3;
            break;
        default:
            // Arrays/Objects should not be edited as values; treat as null
            typeIndex_ = 3;
            break;
    }

    open_ = true;
    needsOpen_ = true;
}

void EditDialog::render(bool isDark) {
    if (!open_) return;

    if (needsOpen_) {
        ImGui::OpenPopup("Edit Value");
        needsOpen_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    float dlgW = ImGui::GetMainViewport()->WorkSize.x * 0.3f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(dlgW, 0), ImGuiCond_Always);

    if (!ImGui::BeginPopupModal("Edit Value", &open_, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    // Path label
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.Size > 1 ? ImGui::GetIO().Fonts->Fonts[1] : nullptr);
    ImGui::TextWrapped("Path: %s", pathLabel_.c_str());
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();

    // Type combo
    const char* types[] = { "string", "number", "boolean", "null" };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("Type", &typeIndex_, types, 4);

    ImGui::Spacing();

    // Value input
    switch (typeIndex_) {
        case 0: // string
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextMultiline("##value", valueBuf_, sizeof(valueBuf_),
                ImVec2(-1, ImGui::GetTextLineHeight() * 4));
            break;
        case 1: // number
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##value", valueBuf_, sizeof(valueBuf_));
            break;
        case 2: // boolean
            ImGui::Checkbox("Value", &boolValue_);
            break;
        case 3: // null
            ImGui::BeginDisabled();
            ImGui::TextColored(isDark ? ImVec4(0.6f, 0.6f, 0.6f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "null");
            ImGui::EndDisabled();
            break;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    float buttonWidth = 100.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float totalWidth = buttonWidth * 2 + spacing;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);

    if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
        if (onSave) {
            DataNode newValue;
            switch (typeIndex_) {
                case 0: // string
                    newValue = DataNode(std::string(valueBuf_));
                    break;
                case 1: { // number
                    std::string numStr(valueBuf_);
                    // Try integer first, then float
                    bool isFloat = numStr.find('.') != std::string::npos ||
                                   numStr.find('e') != std::string::npos ||
                                   numStr.find('E') != std::string::npos;
                    if (isFloat) {
                        char* end = nullptr;
                        double d = std::strtod(valueBuf_, &end);
                        if (end != valueBuf_) {
                            newValue = DataNode(d);
                        } else {
                            newValue = DataNode(static_cast<int64_t>(0));
                        }
                    } else {
                        char* end = nullptr;
                        long long ll = std::strtoll(valueBuf_, &end, 10);
                        if (end != valueBuf_) {
                            newValue = DataNode(static_cast<int64_t>(ll));
                        } else {
                            newValue = DataNode(static_cast<int64_t>(0));
                        }
                    }
                    break;
                }
                case 2: // boolean
                    newValue = DataNode(boolValue_);
                    break;
                case 3: // null
                    newValue = DataNode(nullptr);
                    break;
            }
            onSave(path_, std::move(newValue));
        }
        open_ = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
        open_ = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
