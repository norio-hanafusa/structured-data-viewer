#include "dialogs/delete_dialog.h"
#include "imgui.h"

void DeleteDialog::open(const DataNode::Path& path) {
    path_ = path;
    pathLabel_ = pathToString(path);
    if (pathLabel_.empty()) pathLabel_ = "(root)";

    open_ = true;
    needsOpen_ = true;
}

void DeleteDialog::render() {
    if (!open_) return;

    if (needsOpen_) {
        ImGui::OpenPopup("Delete Confirm");
        needsOpen_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    float dlgW = ImGui::GetMainViewport()->WorkSize.x * 0.25f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(dlgW, 0), ImGuiCond_Always);

    if (!ImGui::BeginPopupModal("Delete Confirm", &open_, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    ImGui::Text("Are you sure you want to delete:");
    ImGui::Spacing();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.Size > 1 ? ImGui::GetIO().Fonts->Fonts[1] : nullptr);
    ImGui::TextWrapped("%s", pathLabel_.c_str());
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Text("This action cannot be undone (except via Undo).");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    float buttonWidth = 100.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float totalWidth = buttonWidth * 2 + spacing;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Delete", ImVec2(buttonWidth, 0))) {
        if (onDelete) {
            onDelete(path_);
        }
        open_ = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
        open_ = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
