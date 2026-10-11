import re

with open(r'c:\Users\EL MOSTAWRED\source\repos\Image Processing\Image Processing\PixelLab\GUI.cpp', 'r') as f:
    gui = f.read()

# Add includes
if '#include "Adjustments.h"' not in gui:
    gui = gui.replace('#include "Renderer.h"', '#include "Renderer.h"\n#include "Adjustments.h"')

# 1. Add Thumbnail struct and history helpers
helpers = '''
struct Thumbnail {
    FilterID id;
    ID3D11ShaderResourceView* srv = nullptr;
};
std::vector<Thumbnail> g_thumbnails;

void CommitHistory(const Image& img) {
    if (g_state.historyIndex < (int)g_state.history.size() - 1) {
        g_state.history.erase(g_state.history.begin() + g_state.historyIndex + 1, g_state.history.end());
    }
    g_state.history.push_back(img);
    g_state.historyIndex = (int)g_state.history.size() - 1;
    g_state.previewDirty = true;
}

void Undo() {
    if (g_state.historyIndex > 0) {
        g_state.historyIndex--;
        g_state.working = g_state.history[g_state.historyIndex];
        g_state.previewDirty = true;
    }
}

void Redo() {
    if (g_state.historyIndex < (int)g_state.history.size() - 1) {
        g_state.historyIndex++;
        g_state.working = g_state.history[g_state.historyIndex];
        g_state.previewDirty = true;
    }
}

void ClearThumbnails() {
    for (auto& t : g_thumbnails) {
        if (t.srv) t.srv->Release();
    }
    g_thumbnails.clear();
}

void GenerateThumbnails() {
    ClearThumbnails();
    if (!g_state.imageLoaded) return;
    
    // Create a small base image for thumbnails
    Image baseImg = g_state.original;
    if (baseImg.width > 200 || baseImg.height > 200) {
        int tw = 150;
        int th = 150;
        if (baseImg.width > baseImg.height) { th = (int)(150.0f * baseImg.height / baseImg.width); }
        else { tw = (int)(150.0f * baseImg.width / baseImg.height); }
        Bridge_ResizeFilter(baseImg, tw, th);
    }
    
    for (int i = 0; i < kFilterCount; ++i) {
        FilterID id = kFilters[i].id;
        Image thumb = baseImg;
        
        try {
            switch (id) {
                case FilterID::Grayscale: GrayscaleFilter(thumb); break;
                case FilterID::BlackAndWhite: BlackAndWhiteFilter(thumb); break;
                case FilterID::Invert: InvertImageFilter(thumb); break;
                case FilterID::Frame: Bridge_FrameFilter(thumb, 124, 77, 255); break;
                case FilterID::Flip: FlipImage(thumb); break;
                case FilterID::Rotate: Bridge_RotateFilter(thumb, 1); break;
                case FilterID::DarkenLighten: Bridge_DarkenAndLightenFilter(thumb, true); break;
                case FilterID::EdgeDetection: EdgeDetectionFilter(thumb); break;
                case FilterID::Blur: BlurFilter(thumb); break;
                case FilterID::Sunlight: Bridge_SunlightFilter(thumb, 50); break;
                case FilterID::OldTV: OldTVFilter(thumb); break;
                case FilterID::PurpleNight: PurpleNightFilter(thumb); break;
                case FilterID::Infrared: InfraredFilter(thumb); break;
                case FilterID::SkewImage: Bridge_SkewFilter(thumb, 15.0); break;
                case FilterID::OilPainting: OilPaintingFilter(thumb); break;
                default: break; // Skip Crop, Resize, Merge for thumbnails
            }
        } catch(...) {}
        
        ID3D11ShaderResourceView* srv = CreateTextureFromImage(thumb, g_pd3dDevice);
        if (srv) {
            g_thumbnails.push_back({id, srv});
        }
    }
}
'''
if 'struct Thumbnail {' not in gui:
    gui = gui.replace('void LoadImageFile(const char* path)', helpers + '\nvoid LoadImageFile(const char* path)')

# 2. Update LoadImageFile
load_img = '''void LoadImageFile(const char* path)
{
    try {
        g_state.original.loadNewImage(std::string(path));
        g_state.working   = g_state.original;
        g_state.imageLoaded = true;
        
        g_state.history.clear();
        g_state.historyIndex = -1;
        CommitHistory(g_state.working);
        
        GenerateThumbnails();
        
        const char* fn = path;
        for (const char* p = path; *p; ++p)
            if (*p == '\\\\' || *p == '/') fn = p + 1;
        strncpy_s(g_state.loadedFilename, sizeof(g_state.loadedFilename), fn, _TRUNCATE);
        strncpy_s(g_state.loadedPath,     sizeof(g_state.loadedPath),    path, _TRUNCATE);
        
        g_state.params.cropW = std::min(g_state.original.width,  200);
        g_state.params.cropH = std::min(g_state.original.height, 200);
        g_state.params.resizeW = g_state.original.width;
        g_state.params.resizeH = g_state.original.height;
        g_state.previewDirty = true;
        g_state.zoomLevel = -1.0f;
        SetStatus("Image loaded successfully.");
    } catch (const std::exception& e) {
        SetStatus(e.what(), true);
    } catch (...) {
        SetStatus("Failed to load image.", true);
    }
}'''
gui = re.sub(r'void LoadImageFile\(const char\* path\)[\s\S]*?(?=\nvoid SaveImageFile)', load_img, gui)

# 3. Add Undo/Redo buttons to DrawHeader
header_replacement = '''    ImGui::SameLine(w - 240.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, PL::BgPanelV);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::BtnDarkHovV);
    
    ImGui::BeginDisabled(g_state.historyIndex <= 0);
    if (ImGui::Button("Undo", {50.0f, 26.0f})) Undo();
    ImGui::EndDisabled();
    
    ImGui::SameLine();
    ImGui::BeginDisabled(g_state.historyIndex >= (int)g_state.history.size() - 1);
    if (ImGui::Button("Redo", {50.0f, 26.0f})) Redo();
    ImGui::EndDisabled();
    
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();'''
if 'Undo();' not in gui:
    gui = gui.replace('ImGui::SameLine(w - 130.0f);', header_replacement + '\n    ImGui::SameLine(w - 130.0f);')

# 4. WinMain WM_DROPFILES support
dropfiles_code = '''    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        char filepath[MAX_PATH];
        if (DragQueryFileA(hDrop, 0, filepath, MAX_PATH)) {
            LoadImageFile(filepath);
        }
        DragFinish(hDrop);
        return 0;
    }
    case WM_SIZE:'''
if 'WM_DROPFILES' not in gui:
    gui = gui.replace('case WM_SIZE:', dropfiles_code)
    gui = gui.replace('ShowWindow(g_hwnd, SW_SHOWDEFAULT);', 'ShowWindow(g_hwnd, SW_SHOWDEFAULT);\n    DragAcceptFiles(g_hwnd, TRUE);')

with open(r'c:\Users\EL MOSTAWRED\source\repos\Image Processing\Image Processing\PixelLab\GUI.cpp', 'w') as f:
    f.write(gui)
