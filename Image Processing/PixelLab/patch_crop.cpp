        ImGui::Image(reinterpret_cast<ImTextureID>(g_state.previewSRV), {dispW, dispH});
        
        if (g_state.isCropping) {
            float zoom = (g_state.zoomLevel < 0.0f) ? (fitW / imgW) : g_state.zoomLevel;
            
            // Interactive logic
            ImVec2 mpos = ImGui::GetMousePos();
            bool hovered = ImGui::IsItemHovered();
            static int dragMode = 0; // 0=none, 1=move, 2=BR corner
            static ImVec2 dragStartPos;
            static int startCX, startCY, startCW, startCH;
            
            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                float sx = curPos.x + g_state.params.cropX * zoom;
                float sy = curPos.y + g_state.params.cropY * zoom;
                float sw = g_state.params.cropW * zoom;
                float sh = g_state.params.cropH * zoom;
                
                if (mpos.x >= sx + sw - 15 && mpos.x <= sx + sw + 5 && mpos.y >= sy + sh - 15 && mpos.y <= sy + sh + 5) {
                    dragMode = 2; // Resize
                } else if (mpos.x >= sx && mpos.x <= sx + sw && mpos.y >= sy && mpos.y <= sy + sh) {
                    dragMode = 1; // Move
                } else {
                    dragMode = 3; // New rect
                    g_state.params.cropX = (mpos.x - curPos.x) / zoom;
                    g_state.params.cropY = (mpos.y - curPos.y) / zoom;
                    g_state.params.cropW = 1;
                    g_state.params.cropH = 1;
                }
                dragStartPos = mpos;
                startCX = g_state.params.cropX;
                startCY = g_state.params.cropY;
                startCW = g_state.params.cropW;
                startCH = g_state.params.cropH;
            }
            
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && dragMode != 0) {
                float dx = (mpos.x - dragStartPos.x) / zoom;
                float dy = (mpos.y - dragStartPos.y) / zoom;
                
                if (dragMode == 1) { // Move
                    g_state.params.cropX = startCX + dx;
                    g_state.params.cropY = startCY + dy;
                } else if (dragMode == 2) { // Resize
                    g_state.params.cropW = startCW + dx;
                    g_state.params.cropH = startCH + dy;
                } else if (dragMode == 3) { // New rect
                    g_state.params.cropW = std::max(1, (int)dx);
                    g_state.params.cropH = std::max(1, (int)dy);
                }
                
                // Clamp
                g_state.params.cropW = std::max(1, std::min(g_state.params.cropW, (int)imgW - g_state.params.cropX));
                g_state.params.cropH = std::max(1, std::min(g_state.params.cropH, (int)imgH - g_state.params.cropY));
                g_state.params.cropX = std::max(0, std::min(g_state.params.cropX, (int)imgW - g_state.params.cropW));
                g_state.params.cropY = std::max(0, std::min(g_state.params.cropY, (int)imgH - g_state.params.cropH));
            }
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) dragMode = 0;
            
            // Draw
            float sx = curPos.x + g_state.params.cropX * zoom;
            float sy = curPos.y + g_state.params.cropY * zoom;
            float sw = g_state.params.cropW * zoom;
            float sh = g_state.params.cropH * zoom;
            
            cdl->AddRectFilled(curPos, {curPos.x + dispW, sy}, IM_COL32(0,0,0, 150)); 
            cdl->AddRectFilled({curPos.x, sy}, {sx, sy + sh}, IM_COL32(0,0,0, 150)); 
            cdl->AddRectFilled({sx + sw, sy}, {curPos.x + dispW, sy + sh}, IM_COL32(0,0,0, 150)); 
            cdl->AddRectFilled({curPos.x, sy + sh}, {curPos.x + dispW, curPos.y + dispH}, IM_COL32(0,0,0, 150)); 
            
            cdl->AddRect({sx, sy}, {sx + sw, sy + sh}, IM_COL32(124, 77, 255, 255), 0.0f, 0, 2.0f);
            cdl->AddRectFilled({sx + sw - 6, sy + sh - 6}, {sx + sw + 6, sy + sh + 6}, IM_COL32(200, 200, 255, 255));
        }
