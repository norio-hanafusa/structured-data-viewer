#pragma once
#include "imgui.h"

namespace theme {

// Catppuccin accent colors (available in both themes)
struct AccentColors {
    ImVec4 blue, purple, green, orange, pink, cyan, red, yellow;
};

void applyMocha(ImGuiStyle& style);   // Dark theme
void applyLatte(ImGuiStyle& style);   // Light theme

const AccentColors& getMochaAccents();
const AccentColors& getLatteAccents();

// Convenience: type-based colors from current accent set
ImVec4 colorForType(const char* type, bool isDark);

} // namespace theme
