#include "dialogs/add_dialog.h"
#include "imgui.h"
#include <cstring>
#include <cstdlib>
#include <string>

void AddDialog::open(const DataNode::Path& parentPath, bool parentIsArray) {
    parentPath_ = parentPath;
    parentIsArray_ = parentIsArray;
    pathLabel_ = pathToString(parentPath);
    if (pathLabel_.empty()) pathLabel_ = "(root)";

    std::memset(keyBuf_, 0, sizeof(keyBuf_));
    std::memset(valueBuf_, 0, sizeof(valueBuf_));
    boolValue_ = false;
    typeIndex_ = 0;
    open_ = true;
    needsOpen_ = true;
}

void AddDialog::render(bool isDark) {
    if (!open_) return;

    if (needsOpen_) {
        ImGui::OpenPopup("Add Item");
        needsOpen_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    float dlgW = ImGui::GetMainViewport()->WorkSize.x * 0.3f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(dlgW, 0), ImGuiCond_Always);

    if (!ImGui::BeginPopupModal("Add Item", &open_, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    // Parent path label
    ImGui::TextWrapped("Parent: %s", pathLabel_.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Key input (hidden for arrays)
    if (!parentIsArray_) {
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("Key", keyBuf_, sizeof(keyBuf_));
        ImGui::Spacing();
    }

    // Type combo
    const char* types[] = { "string", "number", "boolean", "null", "object", "array" };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("Type", &typeIndex_, types, 6);

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
        case 4: // object
            ImGui::BeginDisabled();
            ImGui::Text("{}");
            ImGui::EndDisabled();
            break;
        case 5: // array
            ImGui::BeginDisabled();
            ImGui::Text("[]");
            ImGui::EndDisabled();
            break;
    }

    ImGui::Spacing();

    // Validation message
    bool valid = true;
    if (!parentIsArray_ && std::strlen(keyBuf_) == 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Key must not be empty.");
        valid = false;
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    float buttonWidth = 100.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float totalWidth = buttonWidth * 2 + spacing;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);

    ImGui::BeginDisabled(!valid);
    if (ImGui::Button("Add", ImVec2(buttonWidth, 0))) {
        if (onAdd) {
            DataNode newValue;
            switch (typeIndex_) {
                case 0: // string
                    newValue = DataNode(std::string(valueBuf_));
                    break;
                case 1: { // number
                    std::string numStr(valueBuf_);
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
                case 4: // object
                    newValue = DataNode(DataObject{});
                    break;
                case 5: // array
                    newValue = DataNode(DataArray{});
                    break;
            }
            std::string key = parentIsArray_ ? "" : std::string(keyBuf_);
            onAdd(parentPath_, key, std::move(newValue));
        }
        open_ = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
        open_ = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
