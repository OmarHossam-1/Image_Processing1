// Theme.h - Pixel Lab visual design system
// ApplyPixelLabTheme() is called once after ImGui::CreateContext() to replace
// the default ImGui style with the dark, violet-accented Pixel Lab theme.
#pragma once
#include "imgui/imgui.h"

// Color palette (for use in DrawList calls outside ImGui widget colors)
namespace PL {
    static constexpr ImU32 Accent       = IM_COL32(124,  77, 255, 255);   // #7C4DFF
    static constexpr ImU32 AccentLight  = IM_COL32(183, 148, 255, 255);   // #B794FF
    static constexpr ImU32 BgPanel      = IM_COL32( 36,  36,  58, 255);   // #24243A
    static constexpr ImU32 BgHeader     = IM_COL32( 22,  22,  42, 255);   // #16162A
    static constexpr ImU32 BgCanvas     = IM_COL32( 19,  19,  32, 255);   // #131320
    static constexpr ImU32 BgDeep       = IM_COL32( 28,  28,  39, 255);   // #1C1C27
    static constexpr ImU32 Border       = IM_COL32( 58,  58,  92, 255);   // #3A3A5C
    static constexpr ImU32 TextPrimary  = IM_COL32(232, 232, 240, 255);   // #E8E8F0
    static constexpr ImU32 TextMuted    = IM_COL32(136, 136, 170, 255);   // #8888AA
    static constexpr ImU32 Success      = IM_COL32( 76, 175,  80, 255);   // #4CAF50
    static constexpr ImU32 Error        = IM_COL32(255,  82,  82, 255);   // #FF5252

    // ImVec4 variants for PushStyleColor
    static constexpr ImVec4 BgPanelV    = {0.141f, 0.141f, 0.227f, 1.0f};
    static constexpr ImVec4 BgHeaderV   = {0.086f, 0.086f, 0.165f, 1.0f};
    static constexpr ImVec4 BgCanvasV   = {0.075f, 0.075f, 0.125f, 1.0f};
    static constexpr ImVec4 BgDeepV     = {0.110f, 0.110f, 0.153f, 1.0f};
    static constexpr ImVec4 AccentV     = {0.486f, 0.302f, 1.000f, 1.0f};
    static constexpr ImVec4 AccentHovV  = {0.600f, 0.450f, 1.000f, 1.0f};
    static constexpr ImVec4 AccentActV  = {0.380f, 0.200f, 0.900f, 1.0f};
    static constexpr ImVec4 TextPrimV   = {0.910f, 0.910f, 0.941f, 1.0f};
    static constexpr ImVec4 TextMutedV  = {0.533f, 0.533f, 0.667f, 1.0f};
    static constexpr ImVec4 AccentLightV= {0.718f, 0.580f, 1.000f, 1.0f};
    static constexpr ImVec4 BorderV     = {0.227f, 0.227f, 0.361f, 1.0f};
    static constexpr ImVec4 SuccessV    = {0.298f, 0.686f, 0.314f, 1.0f};
    static constexpr ImVec4 ErrorV      = {1.000f, 0.322f, 0.322f, 1.0f};
    static constexpr ImVec4 BtnDarkV    = {0.180f, 0.180f, 0.290f, 1.0f};
    static constexpr ImVec4 BtnDarkHovV = {0.250f, 0.250f, 0.400f, 1.0f};
}

inline void ApplyPixelLabTheme(ImGuiIO& io)
{
    // --- Fonts ---
    io.Fonts->Clear();
    // Try Segoe UI (always available on Windows 7+)
    ImFont* fontDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 15.0f);
    if (!fontDefault) io.Fonts->AddFontDefault();
    // Larger font for filter names in right panel
    ImFont* fontLarge = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 18.0f);
    if (!fontLarge) io.Fonts->AddFontDefault();
    io.Fonts->Build();

    // --- Style ---
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding     = 0.0f;
    s.ChildRounding      = 6.0f;
    s.FrameRounding      = 4.0f;
    s.PopupRounding      = 6.0f;
    s.ScrollbarRounding  = 4.0f;
    s.GrabRounding       = 4.0f;
    s.TabRounding        = 4.0f;
    s.WindowBorderSize   = 0.0f;
    s.ChildBorderSize    = 1.0f;
    s.FrameBorderSize    = 0.0f;
    s.PopupBorderSize    = 1.0f;
    s.WindowPadding      = {12.0f, 10.0f};
    s.FramePadding       = {8.0f,  5.0f};
    s.ItemSpacing        = {8.0f,  6.0f};
    s.ItemInnerSpacing   = {6.0f,  4.0f};
    s.ScrollbarSize      = 10.0f;
    s.IndentSpacing      = 16.0f;
    s.GrabMinSize        = 10.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]             = PL::BgDeepV;
    c[ImGuiCol_ChildBg]              = PL::BgPanelV;
    c[ImGuiCol_PopupBg]              = {0.141f, 0.141f, 0.227f, 0.97f};
    c[ImGuiCol_Border]               = PL::BorderV;
    c[ImGuiCol_BorderShadow]         = {0,0,0,0};
    c[ImGuiCol_FrameBg]              = {0.165f, 0.165f, 0.267f, 1.0f};
    c[ImGuiCol_FrameBgHovered]       = {0.220f, 0.220f, 0.350f, 1.0f};
    c[ImGuiCol_FrameBgActive]        = {0.270f, 0.270f, 0.420f, 1.0f};
    c[ImGuiCol_TitleBg]              = PL::BgHeaderV;
    c[ImGuiCol_TitleBgActive]        = PL::BgHeaderV;
    c[ImGuiCol_TitleBgCollapsed]     = PL::BgHeaderV;
    c[ImGuiCol_MenuBarBg]            = PL::BgHeaderV;
    c[ImGuiCol_ScrollbarBg]          = {0.090f, 0.090f, 0.140f, 0.6f};
    c[ImGuiCol_ScrollbarGrab]        = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.7f};
    c[ImGuiCol_ScrollbarGrabHovered] = {PL::AccentHovV.x, PL::AccentHovV.y, PL::AccentHovV.z, 0.9f};
    c[ImGuiCol_ScrollbarGrabActive]  = PL::AccentV;
    c[ImGuiCol_CheckMark]            = PL::AccentLightV;
    c[ImGuiCol_SliderGrab]           = PL::AccentV;
    c[ImGuiCol_SliderGrabActive]     = PL::AccentHovV;
    c[ImGuiCol_Button]               = PL::BtnDarkV;
    c[ImGuiCol_ButtonHovered]        = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.85f};
    c[ImGuiCol_ButtonActive]         = PL::AccentActV;
    c[ImGuiCol_Header]               = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.35f};
    c[ImGuiCol_HeaderHovered]        = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.55f};
    c[ImGuiCol_HeaderActive]         = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.80f};
    c[ImGuiCol_Separator]            = PL::BorderV;
    c[ImGuiCol_SeparatorHovered]     = PL::AccentLightV;
    c[ImGuiCol_SeparatorActive]      = PL::AccentV;
    c[ImGuiCol_ResizeGrip]           = {0,0,0,0};
    c[ImGuiCol_ResizeGripHovered]    = PL::AccentV;
    c[ImGuiCol_ResizeGripActive]     = PL::AccentHovV;
    c[ImGuiCol_Tab]                  = {0.165f, 0.165f, 0.267f, 1.0f};
    c[ImGuiCol_TabHovered]           = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.70f};
    c[ImGuiCol_TabActive]            = PL::AccentV;
    c[ImGuiCol_TabUnfocused]         = {0.14f, 0.14f, 0.22f, 1.0f};
    c[ImGuiCol_TabUnfocusedActive]   = {0.22f, 0.22f, 0.35f, 1.0f};
    c[ImGuiCol_PlotLines]            = PL::AccentLightV;
    c[ImGuiCol_PlotLinesHovered]     = PL::AccentV;
    c[ImGuiCol_PlotHistogram]        = PL::AccentV;
    c[ImGuiCol_PlotHistogramHovered] = PL::AccentHovV;
    c[ImGuiCol_TextSelectedBg]       = {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.35f};
    c[ImGuiCol_DragDropTarget]       = PL::AccentLightV;
    c[ImGuiCol_NavHighlight]         = PL::AccentV;
    c[ImGuiCol_NavWindowingHighlight]= PL::AccentLightV;
    c[ImGuiCol_NavWindowingDimBg]    = {0.0f, 0.0f, 0.0f, 0.5f};
    c[ImGuiCol_ModalWindowDimBg]     = {0.0f, 0.0f, 0.0f, 0.5f};
    c[ImGuiCol_Text]                 = PL::TextPrimV;
    c[ImGuiCol_TextDisabled]         = PL::TextMutedV;
}
