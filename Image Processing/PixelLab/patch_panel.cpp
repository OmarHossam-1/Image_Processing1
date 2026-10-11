void DrawRightPanel(float x, float y, float w, float h)
{
    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PL::BgPanelV);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {12.0f, 12.0f});

    ImGui::Begin("##right", nullptr, kPanelFlags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wpos = ImGui::GetWindowPos();
    dl->AddLine({wpos.x, wpos.y}, {wpos.x, wpos.y + h}, PL::Border);

    if (ImGui::BeginTabBar("RightPanelTabs")) {
        // --- TAB 1: QUICK FILTERS ---
        if (ImGui::BeginTabItem("Quick Filters")) {
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Open an image to view Quick Filters.");
                ImGui::PopStyleColor();
            } else {
                // Draw 2-column grid of thumbnails
                const float thumbW = (w - 36.0f) * 0.5f;
                const float thumbH = thumbW;
                
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f, 16.0f});
                int cols = 0;
                
                for (int i = 0; i < kFilterCount; ++i) {
                    FilterID id = kFilters[i].id;
                    ID3D11ShaderResourceView* srv = nullptr;
                    for (auto& t : g_thumbnails) {
                        if (t.id == id) { srv = t.srv; break; }
                    }
                    
                    ImGui::BeginGroup();
                    
                    // Thumbnail Button
                    ImGui::PushID(i);
                    ImVec2 cp = ImGui::GetCursorScreenPos();
                    bool clicked = false;
                    
                    if (srv) {
                        if (ImGui::ImageButton((ImTextureID)srv, {thumbW, thumbH}, {0,0}, {1,1}, 0)) clicked = true;
                    } else {
                        if (ImGui::Button("Generating...", {thumbW, thumbH})) clicked = true;
                    }
                    
                    // Highlight selected or hovering
                    if (ImGui::IsItemHovered()) {
                        dl->AddRect({cp.x, cp.y}, {cp.x+thumbW, cp.y+thumbH}, IM_COL32(124, 77, 255, 255), 0.0f, 0, 2.0f);
                    } else if (g_state.selectedFilter == id) {
                        dl->AddRect({cp.x, cp.y}, {cp.x+thumbW, cp.y+thumbH}, IM_COL32(124, 77, 255, 180), 0.0f, 0, 2.0f);
                    } else {
                        dl->AddRect({cp.x, cp.y}, {cp.x+thumbW, cp.y+thumbH}, IM_COL32(255, 255, 255, 30), 0.0f, 0, 1.0f);
                    }
                    
                    // Name label
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (thumbW - ImGui::CalcTextSize(kFilters[i].name).x) * 0.5f);
                    ImGui::PushStyleColor(ImGuiCol_Text, (g_state.selectedFilter == id) ? PL::AccentLightV : PL::TextPrimV);
                    ImGui::TextUnformatted(kFilters[i].name);
                    ImGui::PopStyleColor();
                    
                    ImGui::PopID();
                    ImGui::EndGroup();
                    
                    // Instant application on click
                    if (clicked) {
                        g_state.selectedFilter = id;
                        
                        if (id == FilterID::Crop) {
                            g_state.isCropping = !g_state.isCropping; // Toggle crop mode natively!
                        } else {
                            // Apply instantly
                            g_state.working = g_state.history[g_state.historyIndex]; // start from clean state
                            
                            try {
                                switch(id) {
                                    case FilterID::Grayscale: GrayscaleFilter(g_state.working); break;
                                    case FilterID::BlackAndWhite: BlackAndWhiteFilter(g_state.working); break;
                                    case FilterID::Invert: InvertImageFilter(g_state.working); break;
                                    case FilterID::Frame: Bridge_FrameFilter(g_state.working, g_state.params.frameR, g_state.params.frameG, g_state.params.frameB); break;
                                    case FilterID::Flip: FlipImage(g_state.working); break;
                                    case FilterID::Rotate: Bridge_RotateFilter(g_state.working, g_state.params.rotateChoice); break;
                                    case FilterID::DarkenLighten: Bridge_DarkenAndLightenFilter(g_state.working, g_state.params.darken); break;
                                    case FilterID::EdgeDetection: EdgeDetectionFilter(g_state.working); break;
                                    case FilterID::Blur: BlurFilter(g_state.working); break;
                                    case FilterID::Sunlight: Bridge_SunlightFilter(g_state.working, g_state.params.sunIntensity); break;
                                    case FilterID::OldTV: OldTVFilter(g_state.working); break;
                                    case FilterID::PurpleNight: PurpleNightFilter(g_state.working); break;
                                    case FilterID::Infrared: InfraredFilter(g_state.working); break;
                                    case FilterID::SkewImage: Bridge_SkewFilter(g_state.working, (double)g_state.params.skewAngle); break;
                                    case FilterID::OilPainting: OilPaintingFilter(g_state.working); break;
                                    case FilterID::Resize: Bridge_ResizeFilter(g_state.working, g_state.params.resizeW, g_state.params.resizeH); break;
                                    case FilterID::Merge: if (g_state.params.mergePath[0]) Bridge_MergeFilter(g_state.working, g_state.params.mergePath, g_state.params.mergeMode); break;
                                    default: break;
                                }
                                CommitHistory(g_state.working);
                                g_state.previewDirty = true;
                                SetStatus("Filter applied.");
                            } catch (...) {
                                SetStatus("Failed to apply filter.", true);
                            }
                        }
                    }
                    
                    cols++;
                    if (cols < 2) {
                        ImGui::SameLine();
                    } else {
                        cols = 0;
                    }
                }
                ImGui::PopStyleVar();
            }
            ImGui::EndTabItem();
        }
        
        // --- TAB 2: ADJUST ---
        if (ImGui::BeginTabItem("Adjust")) {
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Open an image to make adjustments.");
                ImGui::PopStyleColor();
            } else {
                const float fw = w - 24.0f;
                bool changed = false;
                
                ImGui::TextUnformatted("Brightness");
                ImGui::SetNextItemWidth(fw);
                if (ImGui::SliderInt("##adjB", &g_state.adjBrightness, -255, 255)) changed = true;
                
                ImGui::Spacing();
                ImGui::TextUnformatted("Contrast");
                ImGui::SetNextItemWidth(fw);
                if (ImGui::SliderFloat("##adjC", &g_state.adjContrast, 0.0f, 3.0f, "%.2f")) changed = true;
                
                ImGui::Spacing();
                ImGui::TextUnformatted("Saturation");
                ImGui::SetNextItemWidth(fw);
                if (ImGui::SliderFloat("##adjS", &g_state.adjSaturation, 0.0f, 3.0f, "%.2f")) changed = true;
                
                ImGui::Spacing();
                ImGui::TextUnformatted("Hue");
                ImGui::SetNextItemWidth(fw);
                if (ImGui::SliderFloat("##adjH", &g_state.adjHue, -180.0f, 180.0f, "%.1f deg")) changed = true;
                
                if (changed) {
                    g_state.adjPreviewing = true;
                    // Live preview from history state
                    g_state.working = g_state.history[g_state.historyIndex];
                    if (g_state.adjBrightness != 0) AdjustBrightness(g_state.working, g_state.adjBrightness);
                    if (g_state.adjContrast != 1.0f) AdjustContrast(g_state.working, g_state.adjContrast);
                    if (g_state.adjSaturation != 1.0f) AdjustSaturation(g_state.working, g_state.adjSaturation);
                    if (g_state.adjHue != 0.0f) AdjustHue(g_state.working, g_state.adjHue);
                    g_state.previewDirty = true;
                }
                
                ImGui::Spacing();
                ImGui::Spacing();
                
                ImGui::BeginDisabled(!g_state.adjPreviewing);
                ImGui::PushStyleColor(ImGuiCol_Button, PL::AccentV);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, PL::AccentActV);
                if (ImGui::Button("Commit Adjustments", {fw, 36.0f})) {
                    CommitHistory(g_state.working);
                    g_state.adjPreviewing = false;
                    g_state.adjBrightness = 0;
                    g_state.adjContrast = 1.0f;
                    g_state.adjSaturation = 1.0f;
                    g_state.adjHue = 0.0f;
                }
                ImGui::PopStyleColor(3);
                ImGui::EndDisabled();
                
                ImGui::Spacing();
                
                ImGui::BeginDisabled(!g_state.adjPreviewing);
                if (ImGui::Button("Cancel", {fw, 28.0f})) {
                    g_state.working = g_state.history[g_state.historyIndex];
                    g_state.previewDirty = true;
                    g_state.adjPreviewing = false;
                    g_state.adjBrightness = 0;
                    g_state.adjContrast = 1.0f;
                    g_state.adjSaturation = 1.0f;
                    g_state.adjHue = 0.0f;
                }
                ImGui::EndDisabled();
                
                // Add existing Free Rotate/Skew control
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextUnformatted("Free Rotate (Skew)");
                ImGui::SetNextItemWidth(fw);
                ImGui::SliderFloat("##skewadj", &g_state.params.skewAngle, -80.0f, 80.0f, "%.1f deg");
                if (ImGui::Button("Apply Free Rotate", {fw, 28.0f})) {
                    g_state.working = g_state.history[g_state.historyIndex];
                    Bridge_SkewFilter(g_state.working, (double)g_state.params.skewAngle);
                    CommitHistory(g_state.working);
                    g_state.previewDirty = true;
                }
            }
            ImGui::EndTabItem();
        }
        
        // --- TAB 3: PROPERTIES ---
        if (ImGui::BeginTabItem("Properties")) {
            ImGui::Spacing();
            if (g_state.selectedFilter == FilterID::None) {
                ImGui::TextDisabled("Select a Quick Filter first.");
            } else {
                FilterParam& p = g_state.params;
                const float fw = w - 24.0f;
                
                // Draw name
                const FilterInfo* fi = nullptr;
                for (int i = 0; i < kFilterCount; ++i)
                    if (kFilters[i].id == g_state.selectedFilter) { fi = &kFilters[i]; break; }
                
                if (fi) {
                    ImGui::PushStyleColor(ImGuiCol_Text, PL::TextPrimV);
                    ImGui::TextUnformatted(fi->name);
                    ImGui::PopStyleColor();
                    ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                    ImGui::TextWrapped("%s", fi->description);
                    ImGui::PopStyleColor();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
                
                bool needsApply = false;
                
                switch (g_state.selectedFilter) {
                    case FilterID::Frame:
                        ImGui::Text("Border Color (RGB)");
                        if (ImGui::ColorEdit3("##frcol", (float*)&p.frameR, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)) needsApply = true;
                        break;
                    case FilterID::Rotate:
                        ImGui::RadioButton("90 deg clockwise",  &p.rotateChoice, 1);
                        ImGui::RadioButton("180 deg",           &p.rotateChoice, 2);
                        ImGui::RadioButton("270 deg clockwise", &p.rotateChoice, 3);
                        needsApply = true;
                        break;
                    case FilterID::DarkenLighten:
                        { int mode = p.darken ? 0 : 1;
                        ImGui::RadioButton("Darken (x0.5)", &mode, 0);
                        ImGui::RadioButton("Lighten (x1.5)", &mode, 1);
                        p.darken = (mode == 0);
                        needsApply = true; }
                        break;
                    case FilterID::Sunlight:
                        ImGui::SliderInt("Intensity", &p.sunIntensity, 1, 100);
                        needsApply = true;
                        break;
                    case FilterID::Resize:
                        ImGui::InputInt("Width", &p.resizeW);
                        ImGui::InputInt("Height", &p.resizeH);
                        needsApply = true;
                        break;
                    case FilterID::Merge:
                        ImGui::InputText("Second Image", p.mergePath, sizeof(p.mergePath));
                        if (ImGui::Button("Browse...")) {
                            char path[512] = {};
                            if (OpenFileDialog(path, sizeof(path))) strncpy_s(p.mergePath, sizeof(p.mergePath), path, _TRUNCATE);
                        }
                        ImGui::RadioButton("Resize to largest", &p.mergeMode, 1);
                        ImGui::RadioButton("Common area only", &p.mergeMode, 2);
                        needsApply = true;
                        break;
                    case FilterID::Crop:
                        ImGui::TextWrapped("Click the canvas to drag a crop selection, then click Apply Crop.");
                        if (!g_state.isCropping) {
                            if (ImGui::Button("Activate Crop Tool")) g_state.isCropping = true;
                        } else {
                            if (ImGui::Button("Cancel Crop")) g_state.isCropping = false;
                        }
                        needsApply = true;
                        break;
                    default:
                        ImGui::TextDisabled("No parameters required.");
                        break;
                }
                
                if (needsApply) {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Button, PL::AccentV);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, PL::AccentActV);
                    if (ImGui::Button("Apply Params", {fw, 32.0f})) {
                        g_state.working = g_state.history[g_state.historyIndex];
                        switch(g_state.selectedFilter) {
                            case FilterID::Frame: Bridge_FrameFilter(g_state.working, (int)p.frameR, (int)p.frameG, (int)p.frameB); break;
                            case FilterID::Rotate: Bridge_RotateFilter(g_state.working, p.rotateChoice); break;
                            case FilterID::DarkenLighten: Bridge_DarkenAndLightenFilter(g_state.working, p.darken); break;
                            case FilterID::Sunlight: Bridge_SunlightFilter(g_state.working, p.sunIntensity); break;
                            case FilterID::Resize: Bridge_ResizeFilter(g_state.working, p.resizeW, p.resizeH); break;
                            case FilterID::Merge: if(p.mergePath[0]) Bridge_MergeFilter(g_state.working, p.mergePath, p.mergeMode); break;
                            case FilterID::Crop: Bridge_CropFilter(g_state.working, p.cropX, p.cropY, p.cropW, p.cropH); g_state.isCropping = false; break;
                            default: break;
                        }
                        CommitHistory(g_state.working);
                        g_state.previewDirty = true;
                    }
                    ImGui::PopStyleColor(3);
                }
            }
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
