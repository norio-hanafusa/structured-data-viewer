#include "theme/catppuccin.h"
#include <cstring>

namespace theme {

static ImVec4 hex(unsigned int c, float a = 1.0f) {
    return ImVec4(
        ((c >> 16) & 0xFF) / 255.0f,
        ((c >> 8)  & 0xFF) / 255.0f,
        ( c        & 0xFF) / 255.0f,
        a
    );
}

// Catppuccin Mocha palette
static const AccentColors kMocha = {
    hex(0x89b4fa), // blue
    hex(0xcba6f7), // purple (Mauve)
    hex(0xa6e3a1), // green
    hex(0xfab387), // orange (Peach)
    hex(0xf5c2e7), // pink
    hex(0x74c7ec), // cyan (Sapphire)
    hex(0xf38ba8), // red
    hex(0xf9e2af), // yellow
};

// Catppuccin Latte palette
static const AccentColors kLatte = {
    hex(0x1e66f5), // blue
    hex(0x8839ef), // purple (Mauve)
    hex(0x40a02b), // green
    hex(0xfe640b), // orange (Peach)
    hex(0xea76cb), // pink
    hex(0x209fb5), // cyan (Sapphire)
    hex(0xd20f39), // red
    hex(0xdf8e1d), // yellow
};

const AccentColors& getMochaAccents()  { return kMocha; }
const AccentColors& getLatteAccents()  { return kLatte; }

void applyMocha(ImGuiStyle& style) {
    auto& c = style.Colors;
    c[ImGuiCol_WindowBg]           = hex(0x1e1e2e);        // Base
    c[ImGuiCol_ChildBg]            = hex(0x181825);         // Mantle
    c[ImGuiCol_PopupBg]            = hex(0x1e1e2e, 0.95f);
    c[ImGuiCol_Border]             = hex(0x45475a);         // Surface1
    c[ImGuiCol_FrameBg]            = hex(0x313244);         // Surface0
    c[ImGuiCol_FrameBgHovered]     = hex(0x45475a);
    c[ImGuiCol_FrameBgActive]      = hex(0x585b70);         // Surface2
    c[ImGuiCol_TitleBg]            = hex(0x11111b);         // Crust
    c[ImGuiCol_TitleBgActive]      = hex(0x181825);
    c[ImGuiCol_TitleBgCollapsed]   = hex(0x11111b);
    c[ImGuiCol_MenuBarBg]          = hex(0x181825);
    c[ImGuiCol_ScrollbarBg]        = hex(0x11111b);
    c[ImGuiCol_ScrollbarGrab]      = hex(0x45475a);
    c[ImGuiCol_ScrollbarGrabHovered] = hex(0x585b70);
    c[ImGuiCol_ScrollbarGrabActive]  = hex(0x6c7086);
    c[ImGuiCol_CheckMark]          = hex(0x89b4fa);
    c[ImGuiCol_SliderGrab]         = hex(0x89b4fa);
    c[ImGuiCol_SliderGrabActive]   = hex(0xb4d0fb);
    c[ImGuiCol_Button]             = hex(0x313244);
    c[ImGuiCol_ButtonHovered]      = hex(0x45475a);
    c[ImGuiCol_ButtonActive]       = hex(0x585b70);
    c[ImGuiCol_Header]             = hex(0x313244);
    c[ImGuiCol_HeaderHovered]      = hex(0x45475a);
    c[ImGuiCol_HeaderActive]       = hex(0x585b70);
    c[ImGuiCol_Separator]          = hex(0x45475a);
    c[ImGuiCol_SeparatorHovered]   = hex(0x89b4fa);
    c[ImGuiCol_SeparatorActive]    = hex(0x89b4fa);
    c[ImGuiCol_Tab]                = hex(0x181825);
    c[ImGuiCol_TabHovered]         = hex(0x313244);
    c[ImGuiCol_TabSelected]        = hex(0x313244);
    c[ImGuiCol_TabDimmed]          = hex(0x11111b);
    c[ImGuiCol_TabDimmedSelected]  = hex(0x181825);
    c[ImGuiCol_Text]               = hex(0xcdd6f4);         // Text
    c[ImGuiCol_TextDisabled]       = hex(0x7f849c);         // Overlay1
    c[ImGuiCol_TableHeaderBg]      = hex(0x181825);
    c[ImGuiCol_TableBorderStrong]  = hex(0x45475a);
    c[ImGuiCol_TableBorderLight]   = hex(0x313244);
    c[ImGuiCol_TableRowBg]         = hex(0x1e1e2e, 0.0f);
    c[ImGuiCol_TableRowBgAlt]      = hex(0x181825, 0.3f);
    c[ImGuiCol_DragDropTarget]     = hex(0x89b4fa, 0.9f);
    c[ImGuiCol_ModalWindowDimBg]   = hex(0x11111b, 0.6f);

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 6.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding     = 6.0f;
    style.ChildRounding     = 6.0f;
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
}

void applyLatte(ImGuiStyle& style) {
    auto& c = style.Colors;
    c[ImGuiCol_WindowBg]           = hex(0xeff1f5);         // Base
    c[ImGuiCol_ChildBg]            = hex(0xe6e9ef);         // Mantle
    c[ImGuiCol_PopupBg]            = hex(0xeff1f5, 0.95f);
    c[ImGuiCol_Border]             = hex(0xbcc0cc);         // Surface1
    c[ImGuiCol_FrameBg]            = hex(0xccd0da);         // Surface0
    c[ImGuiCol_FrameBgHovered]     = hex(0xbcc0cc);
    c[ImGuiCol_FrameBgActive]      = hex(0xacb0be);         // Surface2
    c[ImGuiCol_TitleBg]            = hex(0xdce0e8);         // Crust
    c[ImGuiCol_TitleBgActive]      = hex(0xe6e9ef);
    c[ImGuiCol_TitleBgCollapsed]   = hex(0xdce0e8);
    c[ImGuiCol_MenuBarBg]          = hex(0xe6e9ef);
    c[ImGuiCol_ScrollbarBg]        = hex(0xdce0e8);
    c[ImGuiCol_ScrollbarGrab]      = hex(0xbcc0cc);
    c[ImGuiCol_ScrollbarGrabHovered] = hex(0xacb0be);
    c[ImGuiCol_ScrollbarGrabActive]  = hex(0x9ca0b0);
    c[ImGuiCol_CheckMark]          = hex(0x1e66f5);
    c[ImGuiCol_SliderGrab]         = hex(0x1e66f5);
    c[ImGuiCol_SliderGrabActive]   = hex(0x4a80f7);
    c[ImGuiCol_Button]             = hex(0xccd0da);
    c[ImGuiCol_ButtonHovered]      = hex(0xbcc0cc);
    c[ImGuiCol_ButtonActive]       = hex(0xacb0be);
    c[ImGuiCol_Header]             = hex(0xccd0da);
    c[ImGuiCol_HeaderHovered]      = hex(0xbcc0cc);
    c[ImGuiCol_HeaderActive]       = hex(0xacb0be);
    c[ImGuiCol_Separator]          = hex(0xbcc0cc);
    c[ImGuiCol_SeparatorHovered]   = hex(0x1e66f5);
    c[ImGuiCol_SeparatorActive]    = hex(0x1e66f5);
    c[ImGuiCol_Tab]                = hex(0xe6e9ef);
    c[ImGuiCol_TabHovered]         = hex(0xccd0da);
    c[ImGuiCol_TabSelected]        = hex(0xccd0da);
    c[ImGuiCol_TabDimmed]          = hex(0xdce0e8);
    c[ImGuiCol_TabDimmedSelected]  = hex(0xe6e9ef);
    c[ImGuiCol_Text]               = hex(0x4c4f69);         // Text
    c[ImGuiCol_TextDisabled]       = hex(0x8c8fa1);         // Overlay1
    c[ImGuiCol_TableHeaderBg]      = hex(0xe6e9ef);
    c[ImGuiCol_TableBorderStrong]  = hex(0xbcc0cc);
    c[ImGuiCol_TableBorderLight]   = hex(0xccd0da);
    c[ImGuiCol_TableRowBg]         = hex(0xeff1f5, 0.0f);
    c[ImGuiCol_TableRowBgAlt]      = hex(0xe6e9ef, 0.3f);
    c[ImGuiCol_DragDropTarget]     = hex(0x1e66f5, 0.9f);
    c[ImGuiCol_ModalWindowDimBg]   = hex(0xdce0e8, 0.6f);

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 6.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding     = 6.0f;
    style.ChildRounding     = 6.0f;
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
}

ImVec4 colorForType(const char* type, bool isDark) {
    const auto& a = isDark ? kMocha : kLatte;
    if (std::strcmp(type, "string")  == 0) return a.green;
    if (std::strcmp(type, "number")  == 0) return a.blue;
    if (std::strcmp(type, "integer") == 0) return a.blue;
    if (std::strcmp(type, "float")   == 0) return a.blue;
    if (std::strcmp(type, "boolean") == 0) return a.orange;
    if (std::strcmp(type, "null")    == 0) return a.red;
    if (std::strcmp(type, "object")  == 0) return a.purple;
    if (std::strcmp(type, "array")   == 0) return a.cyan;
    return isDark ? hex(0xcdd6f4) : hex(0x4c4f69);
}

} // namespace theme
