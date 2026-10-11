// GUI.cpp - Pixel Lab main application
// WinMain entry point, DX11 setup, ImGui frame loop, Assets panel, and behavior-based filter panels.

#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <commdlg.h>
#include <shellapi.h>
#include <oleidl.h>
#include <shlobj.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

// App
#include "Filters.h"
#include "FilterBridge.h"
#include "AppState.h"
#include "Theme.h"
#include "Renderer.h"
#include "Adjustments.h"

// ============================================================================
// DX11 globals
// ============================================================================
static ID3D11Device*           g_pd3dDevice           = nullptr;
static ID3D11DeviceContext*    g_pd3dDeviceContext     = nullptr;
static IDXGISwapChain*         g_pSwapChain           = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView  = nullptr;
static HWND                    g_hwnd                  = nullptr;
static bool                    g_swapChainOccluded     = false;
static UINT                    g_ResizeWidth  = 0;
static UINT                    g_ResizeHeight = 0;

// Application state — single global instance
static AppState g_state;

// ============================================================================
// Layout constants
// ============================================================================
static constexpr float kHeaderH = 52.0f;
static constexpr float kStatusH = 26.0f;
static constexpr float kLeftW   = 230.0f;
static constexpr float kRightW  = 310.0f;

static constexpr ImGuiWindowFlags kPanelFlags =
    ImGuiWindowFlags_NoTitleBar        |
    ImGuiWindowFlags_NoResize          |
    ImGuiWindowFlags_NoMove            |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNav             |
    ImGuiWindowFlags_NoCollapse        |
    ImGuiWindowFlags_NoSavedSettings;

// ============================================================================
// Forward declarations
// ============================================================================
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool OpenFileDialog(char* path, int maxLen);
bool SaveFileDialog(char* path, int maxLen);

void SetStatus(const char* msg, bool isError = false);
void UpdateAssetThumbnail(int assetIndex);
void SelectAsset(int newIndex);
bool ImportAsset(const char* path);
void DeleteAsset(int index);
void CommitHistory(const Image& img, bool updateBaseState = true);
void Undo();
void Redo();
void ApplyQuickFilter(FilterID id);
void ClearQuickFilterThumbnails();
void GenerateQuickFilterThumbnails();

void DrawPixelLabLogo(ImDrawList* dl, ImVec2 pos, float size);
void DrawHeader();
void DrawLeftSidebar(float x, float y, float w, float h);
void DrawCenterCanvas(float x, float y, float w, float h);
void DrawRightPanel(float x, float y, float w, float h);
void DrawStatusBar(float x, float y, float w, float h);

// ============================================================================
// DX11 Device Management
// ============================================================================
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevelArray, 2, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            featureLevelArray, 2, D3D11_SDK_VERSION,
            &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (FAILED(res)) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)         { g_pSwapChain->Release();         g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext)  { g_pd3dDeviceContext->Release();  g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice)         { g_pd3dDevice->Release();         g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// ============================================================================
// File Dialogs
// ============================================================================
static const char kImgFilter[] =
    "Images\0*.jpg;*.jpeg;*.png;*.bmp;*.tga\0"
    "JPEG\0*.jpg;*.jpeg\0"
    "PNG\0*.png\0"
    "BMP\0*.bmp\0"
    "TGA\0*.tga\0"
    "All Files\0*.*\0";

bool OpenFileDialog(char* path, int maxLen)
{
    path[0] = '\0';
    OPENFILENAMEA ofn    = {};
    ofn.lStructSize      = sizeof(ofn);
    ofn.hwndOwner        = g_hwnd;
    ofn.lpstrFilter      = kImgFilter;
    ofn.lpstrFile        = path;
    ofn.nMaxFile         = static_cast<DWORD>(maxLen);
    ofn.Flags            = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameA(&ofn) != FALSE;
}

bool SaveFileDialog(char* path, int maxLen)
{
    path[0] = '\0';
    OPENFILENAMEA ofn    = {};
    ofn.lStructSize      = sizeof(ofn);
    ofn.hwndOwner        = g_hwnd;
    ofn.lpstrFilter      = kImgFilter;
    ofn.lpstrFile        = path;
    ofn.nMaxFile         = static_cast<DWORD>(maxLen);
    ofn.lpstrDefExt      = "png";
    ofn.Flags            = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetSaveFileNameA(&ofn) != FALSE;
}

void SetStatus(const char* msg, bool isError)
{
    strncpy_s(g_state.statusMsg, sizeof(g_state.statusMsg), msg, _TRUNCATE);
    g_state.statusIsError = isError;
}

// ============================================================================
// Quick Filter Thumbnail State
// ============================================================================
struct QuickFilterThumb {
    FilterID id;
    const char* name;
    ID3D11ShaderResourceView* srv = nullptr;
};
static std::vector<QuickFilterThumb> g_quickFilterThumbs;

void ClearQuickFilterThumbnails()
{
    for (auto& t : g_quickFilterThumbs) {
        if (t.srv) t.srv->Release();
    }
    g_quickFilterThumbs.clear();
}

void GenerateQuickFilterThumbnails()
{
    ClearQuickFilterThumbnails();
    if (!g_state.imageLoaded || g_state.activeAssetIndex < 0 ||
        g_state.activeAssetIndex >= static_cast<int>(g_state.assets.size()))
        return;

    const auto& asset = g_state.assets[g_state.activeAssetIndex];
    if (asset.original.width <= 0 || asset.original.height <= 0 || !asset.original.imageData)
        return;

    // Stable source image downscaled once from asset.original to max 140x140 for preview generation
    Image baseImg = asset.original;
    if (baseImg.width > 140 || baseImg.height > 140) {
        int tw = 140, th = 140;
        if (baseImg.width > baseImg.height) {
            th = std::max(1, static_cast<int>(140.0f * baseImg.height / baseImg.width));
        } else {
            tw = std::max(1, static_cast<int>(140.0f * baseImg.width / baseImg.height));
        }
        Bridge_ResizeFilter(baseImg, tw, th);
    }

    // 1. Original (unmodified source image preview)
    {
        ID3D11ShaderResourceView* srv = CreateTextureFromImage(baseImg, g_pd3dDevice);
        if (srv) g_quickFilterThumbs.push_back({ FilterID::None, "Original", srv });
    }

    // 2. Eligible zero-parameter Quick Filters
    struct QFEntry {
        FilterID id;
        const char* name;
    };
    static const QFEntry kQuickList[] = {
        { FilterID::Grayscale,     "Grayscale" },
        { FilterID::BlackAndWhite, "Black & White" },
        { FilterID::Invert,        "Invert" },
        { FilterID::Blur,          "Blur" },
        { FilterID::EdgeDetection, "Edge Detection" },
        { FilterID::OldTV,         "Old Film" },
        { FilterID::PurpleNight,   "Purple Night" },
        { FilterID::Infrared,      "Infrared" },
    };

    for (const auto& item : kQuickList) {
        // ALWAYS copy fresh from baseImg — strictly no chaining or progressive darkening!
        Image thumb = baseImg;
        try {
            switch (item.id) {
                case FilterID::Grayscale:     GrayscaleFilter(thumb); break;
                case FilterID::BlackAndWhite: BlackAndWhiteFilter(thumb); break;
                case FilterID::Invert:        InvertImageFilter(thumb); break;
                case FilterID::Blur:          BlurFilter(thumb); break;
                case FilterID::EdgeDetection: EdgeDetectionFilter(thumb); break;
                case FilterID::OldTV:         OldTVFilter(thumb); break;
                case FilterID::PurpleNight:   PurpleNightFilter(thumb); break;
                case FilterID::Infrared:      InfraredFilter(thumb); break;
                default: break;
            }
        } catch (...) {}

        ID3D11ShaderResourceView* srv = CreateTextureFromImage(thumb, g_pd3dDevice);
        if (srv) {
            g_quickFilterThumbs.push_back({ item.id, item.name, srv });
        }
    }
}

// ============================================================================
// Asset Management (Multiple Images, Safe Editing Separation)
// ============================================================================
void UpdateAssetThumbnail(int assetIndex)
{
    if (assetIndex < 0 || assetIndex >= static_cast<int>(g_state.assets.size())) return;
    auto& asset = g_state.assets[assetIndex];

    if (asset.thumbSRV) {
        asset.thumbSRV->Release();
        asset.thumbSRV = nullptr;
    }

    if (asset.working.width <= 0 || asset.working.height <= 0 || !asset.working.imageData) return;

    Image thumb = asset.working;
    int tw = 160, th = 110;
    if (thumb.width > thumb.height) {
        th = std::max(1, static_cast<int>(160.0f * thumb.height / thumb.width));
    } else {
        tw = std::max(1, static_cast<int>(110.0f * thumb.width / thumb.height));
    }
    Bridge_ResizeFilter(thumb, tw, th);

    asset.thumbSRV = CreateTextureFromImage(thumb, g_pd3dDevice);
    asset.thumbW = tw;
    asset.thumbH = th;
}

void SelectAsset(int newIndex)
{
    if (newIndex < 0 || newIndex >= static_cast<int>(g_state.assets.size())) return;

    // Cancel uncommitted skew or adjustment previews on current asset
    if (g_state.skewPreviewing) {
        g_state.working = g_state.skewBaseline;
        g_state.skewPreviewing = false;
        g_state.params.skewAngle = 0.0f;
    }
    if (g_state.adjPreviewing) {
        g_state.working = g_state.adjBaseline;
        g_state.adjPreviewing = false;
    }

    // 1. Sync current active state back to previous asset before switching
    if (g_state.activeAssetIndex >= 0 && g_state.activeAssetIndex < static_cast<int>(g_state.assets.size())) {
        auto& prev = g_state.assets[g_state.activeAssetIndex];
        prev.working = g_state.working;
        prev.activeQuickFilter = g_state.activeQuickFilter;
        prev.history = g_state.history;
        prev.historyIndex = g_state.historyIndex;
    }

    // 2. Switch active index
    g_state.activeAssetIndex = newIndex;
    auto& curr = g_state.assets[newIndex];

    g_state.original = curr.original;
    g_state.working = curr.working;
    g_state.activeQuickFilter = curr.activeQuickFilter;
    g_state.history = curr.history;
    g_state.historyIndex = curr.historyIndex;
    strncpy_s(g_state.loadedFilename, sizeof(g_state.loadedFilename), curr.filename.c_str(), _TRUNCATE);
    strncpy_s(g_state.loadedPath, sizeof(g_state.loadedPath), curr.path.c_str(), _TRUNCATE);

    // Reset crop/resize/skew parameters to current asset dimensions
    g_state.params.cropX = 0;
    g_state.params.cropY = 0;
    g_state.params.cropW = std::min(curr.working.width, 200);
    g_state.params.cropH = std::min(curr.working.height, 200);
    g_state.params.resizeW = curr.working.width;
    g_state.params.resizeH = curr.working.height;
    g_state.params.skewAngle = 0.0f;

    g_state.imageLoaded = true;
    g_state.previewDirty = true;
    g_state.zoomLevel = -1.0f; // Fit to window
    g_state.isCropping = false;
    g_state.adjPreviewing = false;
    g_state.skewPreviewing = false;

    // Reset adjustment sliders
    g_state.adjBrightness = 0;
    g_state.adjContrast = 1.0f;
    g_state.adjSaturation = 1.0f;
    g_state.adjHue = 0.0f;

    GenerateQuickFilterThumbnails();

    char buf[256];
    snprintf(buf, sizeof(buf), "Active Asset: %s (%d x %d px)",
             curr.filename.c_str(), curr.working.width, curr.working.height);
    SetStatus(buf);
}

bool ImportAsset(const char* rawPath)
{
    if (!rawPath || !*rawPath) return false;

    std::string path = rawPath;
    // Strip leading/trailing quotes and whitespace
    while (!path.empty() && (path.front() == ' ' || path.front() == '\t' || path.front() == '\"' || path.front() == '\''))
        path.erase(path.begin());
    while (!path.empty() && (path.back() == ' ' || path.back() == '\t' || path.back() == '\"' || path.back() == '\''))
        path.pop_back();

    if (path.empty()) return false;

    // Normalize file extension to lowercase for Image_Class.h getExtensionType compatibility
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        for (size_t k = dotPos; k < path.size(); ++k) {
            path[k] = static_cast<char>(tolower(static_cast<unsigned char>(path[k])));
        }
    }

    // Check if already in assets list (case-insensitive path comparison)
    for (size_t i = 0; i < g_state.assets.size(); ++i) {
        if (_stricmp(g_state.assets[i].path.c_str(), path.c_str()) == 0) {
            SelectAsset(static_cast<int>(i));
            SetStatus("Asset already in project — selected.");
            return true;
        }
    }

    try {
        AssetItem item;
        item.path = path;

        // Extract filename
        const char* fn = path.c_str();
        for (const char* p = path.c_str(); *p; ++p)
            if (*p == '\\' || *p == '/') fn = p + 1;
        item.filename = fn;

        item.original.loadNewImage(path);
        item.baseState = item.original;
        item.working = item.original;
        item.activeQuickFilter = FilterID::None;
        item.history.push_back(item.working);
        item.historyIndex = 0;
        item.isModified = false;

        g_state.assets.push_back(std::move(item));
        int newIdx = static_cast<int>(g_state.assets.size()) - 1;

        UpdateAssetThumbnail(newIdx);
        SelectAsset(newIdx);

        SetStatus("Asset imported successfully.");
        return true;
    } catch (const std::exception& e) {
        SetStatus(e.what(), true);
        return false;
    } catch (...) {
        SetStatus("Failed to load asset image.", true);
        return false;
    }
}

void DeleteAsset(int index)
{
    if (index < 0 || index >= static_cast<int>(g_state.assets.size())) return;

    if (g_state.assets[index].thumbSRV) {
        g_state.assets[index].thumbSRV->Release();
        g_state.assets[index].thumbSRV = nullptr;
    }

    g_state.assets.erase(g_state.assets.begin() + index);

    if (g_state.assets.empty()) {
        g_state.activeAssetIndex = -1;
        g_state.imageLoaded = false;
        ReleasePreviewTexture(g_state);
        ClearQuickFilterThumbnails();
        g_state.loadedFilename[0] = '\0';
        g_state.loadedPath[0] = '\0';
        g_state.working = Image();
        g_state.original = Image();
        g_state.history.clear();
        g_state.historyIndex = -1;
        g_state.texWidth = 0;
        g_state.texHeight = 0;
        g_state.isCropping = false;
        g_state.adjPreviewing = false;
        g_state.skewPreviewing = false;
        g_state.params.skewAngle = 0.0f;
        g_state.activeQuickFilter = FilterID::None;
        SetStatus("Asset removed. No assets remaining.");
    } else {
        int nextIndex = index;
        if (nextIndex >= static_cast<int>(g_state.assets.size())) {
            nextIndex = static_cast<int>(g_state.assets.size()) - 1;
        }
        g_state.activeAssetIndex = -1; // force clean reload without syncing back deleted item
        SelectAsset(nextIndex);
        SetStatus("Asset removed from project.");
    }
}

void LoadImageFile(const char* path)
{
    ImportAsset(path);
}

void SaveImageFile(const char* path)
{
    if (!g_state.imageLoaded) return;
    try {
        g_state.working.saveImage(std::string(path));
        char buf[512];
        snprintf(buf, sizeof(buf), "Saved: %s", path);
        SetStatus(buf);
        if (g_state.activeAssetIndex >= 0 && g_state.activeAssetIndex < static_cast<int>(g_state.assets.size())) {
            g_state.assets[g_state.activeAssetIndex].isModified = false;
        }
    } catch (const std::exception& e) {
        SetStatus(e.what(), true);
    } catch (...) {
        SetStatus("Failed to save image.", true);
    }
}

// ============================================================================
// Undo / Redo History Engine
// ============================================================================
void CommitHistory(const Image& img, bool updateBaseState)
{
    g_state.skewPreviewing = false;
    g_state.params.skewAngle = 0.0f;
    g_state.adjPreviewing = false;
    g_state.isCropping = false;

    // Truncate any redone history beyond the current index
    if (g_state.historyIndex >= 0 && g_state.historyIndex < static_cast<int>(g_state.history.size()) - 1) {
        g_state.history.erase(g_state.history.begin() + g_state.historyIndex + 1, g_state.history.end());
    }
    g_state.history.push_back(img);
    g_state.historyIndex = static_cast<int>(g_state.history.size()) - 1;

    // Synchronize working state, tool baselines, and geometry parameters
    g_state.working = img;
    g_state.adjBaseline = img;
    g_state.skewBaseline = img;
    g_state.params.resizeW = img.width;
    g_state.params.resizeH = img.height;
    g_state.params.cropW = std::min(img.width, 200);
    g_state.params.cropH = std::min(img.height, 200);
    g_state.params.cropX = 0;
    g_state.params.cropY = 0;

    // Sync to active asset
    if (g_state.activeAssetIndex >= 0 && g_state.activeAssetIndex < static_cast<int>(g_state.assets.size())) {
        auto& asset = g_state.assets[g_state.activeAssetIndex];
        asset.working = img;
        if (updateBaseState) {
            asset.baseState = img;
            asset.activeQuickFilter = FilterID::None;
            g_state.activeQuickFilter = FilterID::None;
        }
        asset.history = g_state.history;
        asset.historyIndex = g_state.historyIndex;
        asset.isModified = true;

        UpdateAssetThumbnail(g_state.activeAssetIndex);
    }

    // Synchronously update preview texture so all viewports and dimension indicators refresh immediately
    UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
    g_state.previewDirty = false;
}

void Undo()
{
    if (!g_state.imageLoaded) return;

    // If an uncommitted live preview is active, discard it first
    if (g_state.skewPreviewing) {
        g_state.working = g_state.skewBaseline;
        g_state.skewPreviewing = false;
        g_state.params.skewAngle = 0.0f;
        UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
        g_state.previewDirty = false;
        SetStatus("Skew preview discarded.");
        return;
    }
    if (g_state.adjPreviewing) {
        g_state.working = g_state.adjBaseline;
        g_state.adjPreviewing = false;
        g_state.adjBrightness = 0;
        g_state.adjContrast = 1.0f;
        g_state.adjSaturation = 1.0f;
        g_state.adjHue = 0.0f;
        UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
        g_state.previewDirty = false;
        SetStatus("Adjustments preview discarded.");
        return;
    }

    if (g_state.historyIndex > 0 && g_state.historyIndex < static_cast<int>(g_state.history.size())) {
        g_state.historyIndex--;
        g_state.working = g_state.history[g_state.historyIndex];

        // Synchronize all application state and tool baselines
        g_state.isCropping = false;
        g_state.adjBaseline = g_state.working;
        g_state.skewBaseline = g_state.working;
        g_state.params.skewAngle = 0.0f;
        g_state.params.resizeW = g_state.working.width;
        g_state.params.resizeH = g_state.working.height;
        g_state.params.cropW = std::min(g_state.working.width, 200);
        g_state.params.cropH = std::min(g_state.working.height, 200);
        g_state.params.cropX = 0;
        g_state.params.cropY = 0;
        g_state.activeQuickFilter = FilterID::None;

        if (g_state.activeAssetIndex >= 0 && g_state.activeAssetIndex < static_cast<int>(g_state.assets.size())) {
            auto& asset = g_state.assets[g_state.activeAssetIndex];
            asset.working = g_state.working;
            asset.baseState = g_state.working;
            asset.historyIndex = g_state.historyIndex;
            asset.activeQuickFilter = FilterID::None;
            asset.isModified = (g_state.historyIndex > 0);
            UpdateAssetThumbnail(g_state.activeAssetIndex);
        }

        UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
        g_state.previewDirty = false;

        char buf[128];
        snprintf(buf, sizeof(buf), "Undo applied (step %d of %d).",
                 g_state.historyIndex, static_cast<int>(g_state.history.size()) - 1);
        SetStatus(buf);
    }
}

void Redo()
{
    if (!g_state.imageLoaded) return;

    // Discard any active uncommitted previews before redoing
    if (g_state.skewPreviewing) {
        g_state.working = g_state.skewBaseline;
        g_state.skewPreviewing = false;
        g_state.params.skewAngle = 0.0f;
    }
    if (g_state.adjPreviewing) {
        g_state.working = g_state.adjBaseline;
        g_state.adjPreviewing = false;
        g_state.adjBrightness = 0;
        g_state.adjContrast = 1.0f;
        g_state.adjSaturation = 1.0f;
        g_state.adjHue = 0.0f;
    }

    if (g_state.historyIndex >= 0 && g_state.historyIndex < static_cast<int>(g_state.history.size()) - 1) {
        g_state.historyIndex++;
        g_state.working = g_state.history[g_state.historyIndex];

        // Synchronize all application state and tool baselines
        g_state.isCropping = false;
        g_state.adjBaseline = g_state.working;
        g_state.skewBaseline = g_state.working;
        g_state.params.skewAngle = 0.0f;
        g_state.params.resizeW = g_state.working.width;
        g_state.params.resizeH = g_state.working.height;
        g_state.params.cropW = std::min(g_state.working.width, 200);
        g_state.params.cropH = std::min(g_state.working.height, 200);
        g_state.params.cropX = 0;
        g_state.params.cropY = 0;
        g_state.activeQuickFilter = FilterID::None;

        if (g_state.activeAssetIndex >= 0 && g_state.activeAssetIndex < static_cast<int>(g_state.assets.size())) {
            auto& asset = g_state.assets[g_state.activeAssetIndex];
            asset.working = g_state.working;
            asset.baseState = g_state.working;
            asset.historyIndex = g_state.historyIndex;
            asset.activeQuickFilter = FilterID::None;
            asset.isModified = (g_state.historyIndex > 0);
            UpdateAssetThumbnail(g_state.activeAssetIndex);
        }

        UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
        g_state.previewDirty = false;

        char buf[128];
        snprintf(buf, sizeof(buf), "Redo applied (step %d of %d).",
                 g_state.historyIndex, static_cast<int>(g_state.history.size()) - 1);
        SetStatus(buf);
    }
}

// ============================================================================
// Independent Quick Filter Application
// Every Quick Filter applies to the untouched asset.original, never inheriting edits
// ============================================================================
void ApplyQuickFilter(FilterID id)
{
    if (!g_state.imageLoaded || g_state.activeAssetIndex < 0 ||
        g_state.activeAssetIndex >= static_cast<int>(g_state.assets.size()))
        return;

    auto& asset = g_state.assets[g_state.activeAssetIndex];

    // Cancel uncommitted skew or adjustment previews
    if (g_state.skewPreviewing) {
        g_state.skewPreviewing = false;
        g_state.params.skewAngle = 0.0f;
    }
    if (g_state.adjPreviewing) {
        g_state.adjPreviewing = false;
    }

    if (id == FilterID::None) {
        // Restore clean original image
        g_state.working = asset.original;
        asset.working = asset.original;
        asset.activeQuickFilter = FilterID::None;
        g_state.activeQuickFilter = FilterID::None;
        CommitHistory(g_state.working, false);
        SetStatus("Restored Original source image.");
    } else {
        // ALWAYS create a fresh copy from asset.original — independent, does NOT inherit skew!
        Image filtered = asset.original;
        try {
            switch (id) {
                case FilterID::Grayscale:     GrayscaleFilter(filtered); break;
                case FilterID::BlackAndWhite: BlackAndWhiteFilter(filtered); break;
                case FilterID::Invert:        InvertImageFilter(filtered); break;
                case FilterID::Blur:          BlurFilter(filtered); break;
                case FilterID::EdgeDetection: EdgeDetectionFilter(filtered); break;
                case FilterID::OldTV:         OldTVFilter(filtered); break;
                case FilterID::PurpleNight:   PurpleNightFilter(filtered); break;
                case FilterID::Infrared:      InfraredFilter(filtered); break;
                default: break;
            }
            g_state.working = filtered;
            asset.working = filtered;
            asset.activeQuickFilter = id;
            g_state.activeQuickFilter = id;
            CommitHistory(g_state.working, false);

            const char* name = "Filter";
            for (const auto& qf : g_quickFilterThumbs) {
                if (qf.id == id) { name = qf.name; break; }
            }
            char msg[128];
            snprintf(msg, sizeof(msg), "Applied %s (independent).", name);
            SetStatus(msg);
        } catch (...) {
            SetStatus("Failed to apply filter.", true);
        }
    }
}

// ============================================================================
// Logo (drawn procedurally with ImGui DrawList)
// ============================================================================
void DrawPixelLabLogo(ImDrawList* dl, ImVec2 pos, float size)
{
    const float gap  = size * 0.14f;
    const float cell = (size - gap) * 0.5f;
    const float r    = size * 0.16f;

    dl->AddRectFilled({ pos.x,              pos.y              },
                      { pos.x + cell,       pos.y + cell       }, PL::AccentLight, r);
    dl->AddRectFilled({ pos.x + cell + gap, pos.y              },
                      { pos.x + size,       pos.y + cell       }, PL::Accent,      r);
    dl->AddRectFilled({ pos.x,              pos.y + cell + gap },
                      { pos.x + cell,       pos.y + size       }, PL::Accent,      r);
    dl->AddRectFilled({ pos.x + cell + gap, pos.y + cell + gap },
                      { pos.x + size,       pos.y + size       }, PL::Accent,      r);
}

// ============================================================================
// Header Panel (Branding, Shortcuts: Ctrl+Z, Ctrl+Shift+Z, Save, Import)
// ============================================================================
void DrawHeader()
{
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({W, kHeaderH});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PL::BgHeaderV);
    ImGui::PushStyleColor(ImGuiCol_Border,   {0,0,0,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

    ImGui::Begin("##hdr", nullptr, kPanelFlags | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2      wpos = ImGui::GetWindowPos();

    dl->AddLine({wpos.x, wpos.y + kHeaderH - 1},
                {wpos.x + W, wpos.y + kHeaderH - 1}, PL::Border, 1.0f);

    // Logo icon
    const float logoSz = 28.0f;
    const float logoX  = wpos.x + 16.0f;
    const float logoY  = wpos.y + (kHeaderH - logoSz) * 0.5f;
    DrawPixelLabLogo(dl, {logoX, logoY}, logoSz);

    // App name & subtitle
    ImGui::SetCursorPos({16.0f + logoSz + 10.0f, (kHeaderH - ImGui::GetTextLineHeightWithSpacing() * 2.0f) * 0.5f});
    ImGui::PushStyleColor(ImGuiCol_Text, PL::TextPrimV);
    ImGui::Text("Pixel Lab");
    ImGui::PopStyleColor();
    ImGui::SetCursorPos({16.0f + logoSz + 10.0f,
                          (kHeaderH - ImGui::GetTextLineHeightWithSpacing() * 2.0f) * 0.5f + ImGui::GetTextLineHeight() + 2.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
    ImGui::TextUnformatted("Image Processing Studio");
    ImGui::PopStyleColor();

    // Right-side controls (laid out right-to-left)
    const float btnY   = (kHeaderH - 26.0f) * 0.5f;
    float       rightX = W - 14.0f;

    // Save As
    ImGui::PushStyleColor(ImGuiCol_Button,        PL::BtnDarkV);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {PL::AccentV.x, PL::AccentV.y, PL::AccentV.z, 0.70f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
    rightX -= 72.0f;
    ImGui::SetCursorPos({rightX, btnY});
    if (ImGui::Button("Save As##h", {70.0f, 26.0f}) && g_state.imageLoaded) {
        char p[512] = {};
        if (SaveFileDialog(p, sizeof(p))) SaveImageFile(p);
    }

    // Save
    rightX -= 6.0f + 58.0f;
    ImGui::SetCursorPos({rightX, btnY});
    if (ImGui::Button("Save##h", {56.0f, 26.0f}) && g_state.imageLoaded) {
        char p[512] = {};
        if (SaveFileDialog(p, sizeof(p))) SaveImageFile(p);
    }

    // Redo (button label: "Redo", shortcut Ctrl+Shift+Z runs in background)
    const bool canRedo = g_state.imageLoaded && (g_state.historyIndex >= 0) &&
                         (g_state.historyIndex < static_cast<int>(g_state.history.size()) - 1);
    const float redoBtnW = 56.0f;
    rightX -= 8.0f + redoBtnW;
    ImGui::SetCursorPos({rightX, btnY});
    if (!canRedo) ImGui::BeginDisabled();
    if (ImGui::Button("Redo##h", {redoBtnW, 26.0f})) {
        Redo();
    }
    if (!canRedo) ImGui::EndDisabled();

    // Undo (button label: "Undo", shortcut Ctrl+Z runs in background)
    const bool canUndo = g_state.imageLoaded && (g_state.historyIndex > 0);
    const float undoBtnW = 56.0f;
    rightX -= 6.0f + undoBtnW;
    ImGui::SetCursorPos({rightX, btnY});
    if (!canUndo) ImGui::BeginDisabled();
    if (ImGui::Button("Undo##h", {undoBtnW, 26.0f})) {
        Undo();
    }
    if (!canUndo) ImGui::EndDisabled();

    ImGui::PopStyleColor(3);

    // Active image info badge
    if (g_state.imageLoaded) {
        char info[200];
        snprintf(info, sizeof(info), "%s   %d x %d px",
                 g_state.loadedFilename,
                 g_state.working.width, g_state.working.height);
        ImVec2 tsz = ImGui::CalcTextSize(info);
        rightX -= 16.0f + tsz.x;
        ImGui::SetCursorPos({rightX, (kHeaderH - ImGui::GetTextLineHeight()) * 0.5f});
        ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
        ImGui::TextUnformatted(info);
        ImGui::PopStyleColor();
    }

    // Import Image button in header
    rightX -= 14.0f + 110.0f;
    ImGui::SetCursorPos({rightX, btnY});
    ImGui::PushStyleColor(ImGuiCol_Button,        PL::AccentV);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
    if (ImGui::Button("  Import Image  ##h", {110.0f, 26.0f})) {
        char p[512] = {};
        if (OpenFileDialog(p, sizeof(p))) ImportAsset(p);
    }
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// ============================================================================
// Left Sidebar — Assets Panel
// ============================================================================
void DrawLeftSidebar(float x, float y, float w, float h)
{
    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PL::BgPanelV);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,  PL::BgPanelV);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});

    ImGui::Begin("##assets_panel", nullptr, kPanelFlags);

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2      wpos = ImGui::GetWindowPos();
    dl->AddLine({wpos.x + w - 1, wpos.y}, {wpos.x + w - 1, wpos.y + h}, PL::Border);

    // Assets header title & count
    ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
    ImGui::TextUnformatted("ASSETS");
    ImGui::PopStyleColor();

    if (!g_state.assets.empty()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
        ImGui::Text("(%d)", static_cast<int>(g_state.assets.size()));
        ImGui::PopStyleColor();
    }
    ImGui::Spacing();

    // Import Image Button
    const float fullW = w - 20.0f;
    ImGui::PushStyleColor(ImGuiCol_Button,        PL::AccentV);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
    if (ImGui::Button("+ Import Image##left", {fullW, 32.0f})) {
        char p[512] = {};
        if (OpenFileDialog(p, sizeof(p))) ImportAsset(p);
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Separator, PL::BorderV);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Scrollable Asset Cards List
    const float listH = h - 110.0f;
    ImGui::BeginChild("##assets_scroll", {fullW, listH}, false, ImGuiWindowFlags_NoScrollbar);

    if (g_state.assets.empty()) {
        ImGui::Spacing(); ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
        ImGui::TextWrapped("No assets imported.\n\nClick '+ Import Image' or drag and drop image files here.");
        ImGui::PopStyleColor();
    } else {
        const float cardW = fullW - 4.0f;
        const float cardH = 118.0f;

        for (int i = 0; i < static_cast<int>(g_state.assets.size()); ++i) {
            const auto& asset = g_state.assets[i];
            const bool isSelected = (i == g_state.activeAssetIndex);

            ImGui::PushID(i);
            ImVec2 curPos = ImGui::GetCursorScreenPos();

            // Background & Border styling
            ImDrawList* cdl = ImGui::GetWindowDrawList();
            ImU32 bgCol = isSelected ? IM_COL32(38, 38, 62, 255) : IM_COL32(24, 24, 38, 255);
            ImU32 borderCol = isSelected ? IM_COL32(124, 77, 255, 255) : IM_COL32(50, 50, 75, 180);
            float borderThick = isSelected ? 2.0f : 1.0f;

            cdl->AddRectFilled(curPos, {curPos.x + cardW, curPos.y + cardH}, bgCol, 6.0f);
            cdl->AddRect(curPos, {curPos.x + cardW, curPos.y + cardH}, borderCol, 6.0f, 0, borderThick);

            // Centered Thumbnail Image inside card
            const float maxImgW = cardW - 16.0f;
            const float maxImgH = 70.0f;
            float dispW = maxImgW;
            float dispH = maxImgH;

            if (asset.thumbW > 0 && asset.thumbH > 0) {
                float aspect = static_cast<float>(asset.thumbW) / static_cast<float>(asset.thumbH);
                if (dispW / dispH > aspect) {
                    dispW = dispH * aspect;
                } else {
                    dispH = dispW / aspect;
                }
            }

            float imgPosX = curPos.x + (cardW - dispW) * 0.5f;
            float imgPosY = curPos.y + 8.0f + (maxImgH - dispH) * 0.5f;

            if (asset.thumbSRV) {
                cdl->AddImage(reinterpret_cast<ImTextureID>(asset.thumbSRV),
                              {imgPosX, imgPosY}, {imgPosX + dispW, imgPosY + dispH});
                cdl->AddRect({imgPosX - 1, imgPosY - 1}, {imgPosX + dispW + 1, imgPosY + dispH + 1},
                             IM_COL32(0, 0, 0, 80), 2.0f);
            }

            // Filename label centered beneath thumbnail
            const char* fn = asset.filename.c_str();
            ImVec2 textSz = ImGui::CalcTextSize(fn);
            float textPosX = curPos.x + std::max(6.0f, (cardW - textSz.x) * 0.5f);
            float textPosY = curPos.y + 82.0f;

            ImU32 textCol = isSelected ? IM_COL32(220, 200, 255, 255) : IM_COL32(190, 190, 215, 230);
            cdl->AddText({textPosX, textPosY}, textCol, fn);

            // Subtitle: Dimensions / status
            char dims[64];
            snprintf(dims, sizeof(dims), "%d x %d%s",
                     asset.working.width, asset.working.height,
                     asset.isModified ? " *" : "");
            ImVec2 dimsSz = ImGui::CalcTextSize(dims);
            float dimsPosX = curPos.x + (cardW - dimsSz.x) * 0.5f;
            float dimsPosY = curPos.y + 98.0f;
            cdl->AddText({dimsPosX, dimsPosY}, IM_COL32(130, 130, 160, 200), dims);

            // Invisible selectable button over card
            ImGui::SetCursorScreenPos(curPos);
            if (ImGui::InvisibleButton("##asset_btn", {cardW, cardH})) {
                SelectAsset(i);
            }

            // Note: Large obstructive tooltip removed to prevent covering the canvas preview!

            ImGui::PopID();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
        }
    }
    ImGui::EndChild();

    // Delete Asset Button at bottom (immediately removes asset from project)
    ImGui::SetCursorPosY(h - 38.0f);
    const bool canDelete = (!g_state.assets.empty() && g_state.activeAssetIndex >= 0 &&
                            g_state.activeAssetIndex < static_cast<int>(g_state.assets.size()));

    if (!canDelete) ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(50, 28, 38, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 40, 50, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(220, 50, 60, 255));
    ImGui::PushStyleColor(ImGuiCol_Text,          canDelete ? ImVec4(1.0f, 0.55f, 0.58f, 1.0f) : PL::TextMutedV);

    if (ImGui::Button("Delete Asset##btn", {fullW, 28.0f})) {
        DeleteAsset(g_state.activeAssetIndex);
    }
    ImGui::PopStyleColor(4);
    if (!canDelete) ImGui::EndDisabled();

    if (g_state.isDragHovering) {
        ImDrawList* fgDl = ImGui::GetForegroundDrawList();
        fgDl->AddRect({ x + 2.0f, y + 2.0f }, { x + w - 2.0f, y + h - 2.0f },
                      IM_COL32(147, 95, 255, 220), 4.0f, 0, 2.0f);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// ============================================================================
// Center Canvas — Image Preview, Zoom, Pan & Interactive Crop Tool
// ============================================================================
void DrawCenterCanvas(float x, float y, float w, float h)
{
    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PL::BgCanvasV);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

    ImGui::Begin("##canvas", nullptr, kPanelFlags | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2      wpos = ImGui::GetWindowPos();

    if (!g_state.imageLoaded || !g_state.previewSRV) {
        // --- Empty state checkerboard ---
        const float cs = 24.0f;
        const ImU32 c1 = IM_COL32(25, 25, 40, 255);
        const ImU32 c2 = IM_COL32(30, 30, 48, 255);
        for (float gy = 0; gy < h; gy += cs)
            for (float gx = 0; gx < w; gx += cs) {
                int ci = (static_cast<int>(gy / cs) + static_cast<int>(gx / cs)) % 2;
                dl->AddRectFilled({wpos.x + gx, wpos.y + gy},
                                  {wpos.x + gx + cs, wpos.y + gy + cs},
                                  ci ? c1 : c2);
            }

        const char* line1 = "No Asset Loaded";
        const char* line2 = "Import an image to begin editing";
        ImVec2 s1 = ImGui::CalcTextSize(line1);
        ImVec2 s2 = ImGui::CalcTextSize(line2);
        float cy = wpos.y + (h - s1.y - 8.0f - s2.y - 36.0f) * 0.5f;

        const float iconSz = 48.0f;
        DrawPixelLabLogo(dl, {wpos.x + (w - iconSz) * 0.5f, cy}, iconSz);
        cy += iconSz + 16.0f;

        dl->AddText({wpos.x + (w - s1.x) * 0.5f, cy}, IM_COL32(220, 220, 240, 200), line1);
        cy += s1.y + 8.0f;
        dl->AddText({wpos.x + (w - s2.x) * 0.5f, cy}, IM_COL32(136, 136, 170, 180), line2);
        cy += s2.y + 16.0f;

        ImGui::SetCursorPos({(w - 140.0f) * 0.5f, cy - wpos.y});
        ImGui::PushStyleColor(ImGuiCol_Button,        PL::AccentV);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
        if (ImGui::Button("+ Import Image##center", {140.0f, 32.0f})) {
            char p[512] = {};
            if (OpenFileDialog(p, sizeof(p))) ImportAsset(p);
        }
        ImGui::PopStyleColor(3);

    } else {
        // --- Image Preview Area ---
        const float imgW = static_cast<float>(g_state.texWidth);
        const float imgH = static_cast<float>(g_state.texHeight);

        ImGuiIO& io = ImGui::GetIO();
        const float pad = 16.0f;
        const float aW  = w - pad * 2.0f;
        const float aH  = h - pad * 2.0f - 44.0f;

        float fitW, fitH;
        if (imgW / imgH > aW / aH) {
            fitW = aW;
            fitH = aW * imgH / imgW;
        } else {
            fitH = aH;
            fitW = aH * imgW / imgH;
        }

        float dispW, dispH;
        if (g_state.zoomLevel < 0.0f) {
            dispW = fitW;
            dispH = fitH;
        } else {
            dispW = imgW * g_state.zoomLevel;
            dispH = imgH * g_state.zoomLevel;
        }

        // Scrollable region
        ImGui::SetCursorPos({0, 0});
        ImGui::BeginChild("##image_scroll_area", {w, h - 44.0f}, false, ImGuiWindowFlags_HorizontalScrollbar);

        // Mouse wheel zoom
        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
            float currentZoom = (g_state.zoomLevel < 0.0f) ? (fitW / imgW) : g_state.zoomLevel;
            float newZoom = currentZoom * (io.MouseWheel > 0 ? 1.15f : 0.85f);
            g_state.zoomLevel = std::max(0.1f, std::min(newZoom, 20.0f));

            dispW = imgW * g_state.zoomLevel;
            dispH = imgH * g_state.zoomLevel;
        }

        float scrollAreaW = ImGui::GetContentRegionAvail().x;
        float scrollAreaH = ImGui::GetContentRegionAvail().y;

        float imgX = std::max(0.0f, (scrollAreaW - dispW) * 0.5f);
        float imgY = std::max(0.0f, (scrollAreaH - dispH) * 0.5f);

        ImGui::SetCursorPos({imgX, imgY});
        ImVec2 curPos = ImGui::GetCursorScreenPos();

        ImDrawList* cdl = ImGui::GetWindowDrawList();
        cdl->AddRectFilled({curPos.x - 2, curPos.y - 2}, {curPos.x + dispW + 2, curPos.y + dispH + 2},
                          IM_COL32(0, 0, 0, 100), 2.0f);

        ImGui::Image(reinterpret_cast<ImTextureID>(g_state.previewSRV), {dispW, dispH});

        // Middle-mouse drag to pan
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            ImGui::SetScrollX(ImGui::GetScrollX() - io.MouseDelta.x);
            ImGui::SetScrollY(ImGui::GetScrollY() - io.MouseDelta.y);
        }

        cdl->AddRect({curPos.x - 1, curPos.y - 1}, {curPos.x + dispW + 1, curPos.y + dispH + 1},
                    IM_COL32(124, 77, 255, 80), 0.0f, 0, 1.0f);

        // --- Interactive Crop Tool Overlay ---
        if (g_state.isCropping) {
            float zoom = (g_state.zoomLevel < 0.0f) ? (fitW / imgW) : g_state.zoomLevel;
            ImVec2 mpos = ImGui::GetMousePos();
            bool hovered = ImGui::IsItemHovered();

            static int dragMode = 0; // 0=none, 1=move, 2=resize-BR, 3=create-new
            static ImVec2 dragStartPos;
            static int startCX, startCY, startCW, startCH;

            float sx = curPos.x + g_state.params.cropX * zoom;
            float sy = curPos.y + g_state.params.cropY * zoom;
            float sw = g_state.params.cropW * zoom;
            float sh = g_state.params.cropH * zoom;

            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (mpos.x >= sx + sw - 14 && mpos.x <= sx + sw + 6 &&
                    mpos.y >= sy + sh - 14 && mpos.y <= sy + sh + 6) {
                    dragMode = 2; // Resize BR
                } else if (mpos.x >= sx && mpos.x <= sx + sw && mpos.y >= sy && mpos.y <= sy + sh) {
                    dragMode = 1; // Move
                } else {
                    dragMode = 3; // New rect
                    g_state.params.cropX = static_cast<int>((mpos.x - curPos.x) / zoom);
                    g_state.params.cropY = static_cast<int>((mpos.y - curPos.y) / zoom);
                    g_state.params.cropW = 10;
                    g_state.params.cropH = 10;
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
                    g_state.params.cropX = startCX + static_cast<int>(dx);
                    g_state.params.cropY = startCY + static_cast<int>(dy);
                } else if (dragMode == 2) { // Resize
                    g_state.params.cropW = std::max(1, startCW + static_cast<int>(dx));
                    g_state.params.cropH = std::max(1, startCH + static_cast<int>(dy));
                } else if (dragMode == 3) { // Create
                    g_state.params.cropW = std::max(1, static_cast<int>(dx));
                    g_state.params.cropH = std::max(1, static_cast<int>(dy));
                }

                // Bounds validation & clamping
                g_state.params.cropW = std::max(1, std::min(g_state.params.cropW, static_cast<int>(imgW) - g_state.params.cropX));
                g_state.params.cropH = std::max(1, std::min(g_state.params.cropH, static_cast<int>(imgH) - g_state.params.cropY));
                g_state.params.cropX = std::max(0, std::min(g_state.params.cropX, static_cast<int>(imgW) - g_state.params.cropW));
                g_state.params.cropY = std::max(0, std::min(g_state.params.cropY, static_cast<int>(imgH) - g_state.params.cropH));
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                dragMode = 0;
            }

            sx = curPos.x + g_state.params.cropX * zoom;
            sy = curPos.y + g_state.params.cropY * zoom;
            sw = g_state.params.cropW * zoom;
            sh = g_state.params.cropH * zoom;

            // Dim outer region
            cdl->AddRectFilled(curPos, {curPos.x + dispW, sy}, IM_COL32(0, 0, 0, 160));
            cdl->AddRectFilled({curPos.x, sy}, {sx, sy + sh}, IM_COL32(0, 0, 0, 160));
            cdl->AddRectFilled({sx + sw, sy}, {curPos.x + dispW, sy + sh}, IM_COL32(0, 0, 0, 160));
            cdl->AddRectFilled({curPos.x, sy + sh}, {curPos.x + dispW, curPos.y + dispH}, IM_COL32(0, 0, 0, 160));

            // Crop boundary box
            cdl->AddRect({sx, sy}, {sx + sw, sy + sh}, IM_COL32(140, 95, 255, 255), 0.0f, 0, 2.0f);
            cdl->AddRectFilled({sx + sw - 6, sy + sh - 6}, {sx + sw + 6, sy + sh + 6}, IM_COL32(230, 230, 255, 255), 2.0f);

            // Floating canvas action controls for Crop
            ImGui::SetCursorPos({imgX + 10.0f, imgY + 10.0f});
            ImGui::PushStyleColor(ImGuiCol_Button, PL::AccentV);
            if (ImGui::Button(" Apply Crop ##canvas_crp", {110.0f, 26.0f})) {
                Bridge_CropFilter(g_state.working, g_state.params.cropX, g_state.params.cropY, g_state.params.cropW, g_state.params.cropH);
                CommitHistory(g_state.working, true);
                g_state.isCropping = false;
                SetStatus("Crop applied.");
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::Button(" Cancel ##canvas_crp", {80.0f, 26.0f})) {
                g_state.isCropping = false;
            }
        }

        ImGui::EndChild();

        // --- Bottom Toolbar (Zoom Controls & File Badge) ---
        float toolbarY = h - 44.0f;
        ImGui::SetCursorPos({0, toolbarY});
        ImGui::BeginChild("##zoom_toolbar", {w, 44.0f}, false, ImGuiWindowFlags_NoScrollbar);

        float currentZoomPct = (g_state.zoomLevel < 0.0f) ? ((fitW / imgW) * 100.0f) : (g_state.zoomLevel * 100.0f);

        // Center badge
        char badge[256];
        snprintf(badge, sizeof(badge), " %s   %d x %d px ",
                 g_state.loadedFilename,
                 g_state.working.width,
                 g_state.working.height);
        ImVec2 bsz = ImGui::CalcTextSize(badge);
        const float bx = (w - bsz.x) * 0.5f;

        ImDrawList* tdl = ImGui::GetWindowDrawList();
        ImVec2 twpos = ImGui::GetWindowPos();
        tdl->AddRectFilled({twpos.x + bx - 4, twpos.y + 10}, {twpos.x + bx + bsz.x + 4, twpos.y + 10 + bsz.y + 6},
                          IM_COL32(22, 22, 42, 210), 4.0f);
        tdl->AddText({twpos.x + bx, twpos.y + 13}, IM_COL32(136, 136, 170, 230), badge);

        // Right side zoom controls
        float zx = w - 220.0f;
        ImGui::SetCursorPos({zx, 9.0f});

        ImGui::PushStyleColor(ImGuiCol_Button,        PL::BtnDarkV);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::BtnDarkHovV);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.35f, 0.35f, 0.55f, 1.0f});

        if (ImGui::Button("-##zout", {28, 26})) {
            if (g_state.zoomLevel < 0) g_state.zoomLevel = fitW / imgW;
            g_state.zoomLevel = std::max(0.1f, g_state.zoomLevel / 1.25f);
        }

        ImGui::SameLine();
        char ztext[32];
        snprintf(ztext, sizeof(ztext), "%.0f%%", currentZoomPct);
        ImVec2 zsz = ImGui::CalcTextSize(ztext);
        ImGui::SetCursorPosY(13.0f);
        ImGui::SetCursorPosX(zx + 28.0f + (38.0f - zsz.x) * 0.5f);
        ImGui::TextUnformatted(ztext);

        ImGui::SameLine();
        ImGui::SetCursorPosX(zx + 28.0f + 38.0f + 8.0f);
        ImGui::SetCursorPosY(9.0f);
        if (ImGui::Button("+##zin", {28, 26})) {
            if (g_state.zoomLevel < 0) g_state.zoomLevel = fitW / imgW;
            g_state.zoomLevel = std::min(20.0f, g_state.zoomLevel * 1.25f);
        }

        ImGui::SameLine(0, 10.0f);
        if (ImGui::Button("100%", {46, 26})) {
            g_state.zoomLevel = 1.0f;
        }

        ImGui::SameLine(0, 6.0f);
        if (ImGui::Button("Fit", {36, 26})) {
            g_state.zoomLevel = -1.0f;
        }

        ImGui::PopStyleColor(3);
        ImGui::EndChild();
    }

    // Drag-and-drop visual drop indicator overlay
    if (g_state.isDragHovering) {
        ImDrawList* fgDl = ImGui::GetForegroundDrawList();
        ImVec2 cMin = { x + 8.0f, y + 8.0f };
        ImVec2 cMax = { x + w - 8.0f, y + h - 8.0f };

        // Translucent background scrim
        fgDl->AddRectFilled(cMin, cMax, IM_COL32(18, 16, 32, 215), 10.0f);
        // Vibrant glowing border
        fgDl->AddRect(cMin, cMax, IM_COL32(147, 95, 255, 240), 10.0f, 0, 2.5f);

        // Center card banner
        const char* dropHeader = "Drop Image to Open";
        const char* dropSub    = "Supports JPG, PNG, BMP, and TGA";
        ImVec2 hSz = ImGui::CalcTextSize(dropHeader);
        ImVec2 sSz = ImGui::CalcTextSize(dropSub);

        float cardW = std::max(hSz.x, sSz.x) + 64.0f;
        float cardH = hSz.y + sSz.y + 40.0f;
        ImVec2 cardMin = { x + (w - cardW) * 0.5f, y + (h - cardH) * 0.5f };
        ImVec2 cardMax = { cardMin.x + cardW, cardMin.y + cardH };

        fgDl->AddRectFilled(cardMin, cardMax, IM_COL32(34, 30, 58, 250), 8.0f);
        fgDl->AddRect(cardMin, cardMax, IM_COL32(124, 77, 255, 220), 8.0f, 0, 1.5f);

        fgDl->AddText({ cardMin.x + (cardW - hSz.x) * 0.5f, cardMin.y + 14.0f },
                      IM_COL32(255, 255, 255, 255), dropHeader);
        fgDl->AddText({ cardMin.x + (cardW - sSz.x) * 0.5f, cardMin.y + 14.0f + hSz.y + 6.0f },
                      IM_COL32(180, 170, 220, 220), dropSub);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ============================================================================
// Right Panel — Order: 1. Adjust (Default), 2. Quick Filters, 3. Geometry, 4. Effects
// ============================================================================
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

    const float fw = w - 24.0f;

    // Track initial selection so Adjust is selected by default on startup
    static bool s_firstFrameTab = true;
    ImGuiTabItemFlags adjustTabFlags = s_firstFrameTab ? ImGuiTabItemFlags_SetSelected : 0;

    if (ImGui::BeginTabBar("RightPanelTabs")) {

        // ====================================================================
        // 1. ADJUST (Default first tab: Brightness, Contrast, Saturation, Hue, Darken/Lighten)
        // ====================================================================
        if (ImGui::BeginTabItem("Adjust", nullptr, adjustTabFlags)) {
            s_firstFrameTab = false;
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Import an image to adjust colors and lighting.");
                ImGui::PopStyleColor();
            } else {
                const float availW = ImGui::GetContentRegionAvail().x;
                bool slidersChanged = false;

                ImGui::TextUnformatted("Brightness");
                ImGui::SetNextItemWidth(availW);
                if (ImGui::SliderInt("##adjB", &g_state.adjBrightness, -255, 255)) slidersChanged = true;

                ImGui::Spacing();
                ImGui::TextUnformatted("Contrast");
                ImGui::SetNextItemWidth(availW);
                if (ImGui::SliderFloat("##adjC", &g_state.adjContrast, 0.10f, 3.0f, "%.2f")) slidersChanged = true;

                ImGui::Spacing();
                ImGui::TextUnformatted("Saturation");
                ImGui::SetNextItemWidth(availW);
                if (ImGui::SliderFloat("##adjS", &g_state.adjSaturation, 0.0f, 3.0f, "%.2f")) slidersChanged = true;

                ImGui::Spacing();
                ImGui::TextUnformatted("Hue");
                ImGui::SetNextItemWidth(availW);
                if (ImGui::SliderFloat("##adjH", &g_state.adjHue, -180.0f, 180.0f, "%.1f deg")) slidersChanged = true;

                // Live Preview from stable baseline (never multiplies repeatedly or compounds)
                if (slidersChanged) {
                    if (!g_state.adjPreviewing) {
                        g_state.adjBaseline = g_state.working;
                        g_state.adjPreviewing = true;
                    }
                    g_state.working = g_state.adjBaseline;
                    if (g_state.adjBrightness != 0) AdjustBrightness(g_state.working, g_state.adjBrightness);
                    if (g_state.adjContrast != 1.0f) AdjustContrast(g_state.working, g_state.adjContrast);
                    if (g_state.adjSaturation != 1.0f) AdjustSaturation(g_state.working, g_state.adjSaturation);
                    if (g_state.adjHue != 0.0f) AdjustHue(g_state.working, g_state.adjHue);
                    g_state.previewDirty = true;
                }

                ImGui::Spacing();
                ImGui::BeginDisabled(!g_state.adjPreviewing);
                ImGui::PushStyleColor(ImGuiCol_Button,        PL::AccentV);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
                if (ImGui::Button("Commit Adjustments##btn", {availW, 32.0f})) {
                    CommitHistory(g_state.working, true); // updates baseState and commits to history
                    g_state.adjPreviewing = false;
                    g_state.adjBrightness = 0;
                    g_state.adjContrast = 1.0f;
                    g_state.adjSaturation = 1.0f;
                    g_state.adjHue = 0.0f;
                    SetStatus("Adjustments committed.");
                }
                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                if (ImGui::Button("Discard##btn", {availW, 26.0f})) {
                    g_state.working = g_state.adjBaseline;
                    g_state.previewDirty = true;
                    g_state.adjPreviewing = false;
                    g_state.adjBrightness = 0;
                    g_state.adjContrast = 1.0f;
                    g_state.adjSaturation = 1.0f;
                    g_state.adjHue = 0.0f;
                    SetStatus("Adjustments discarded.");
                }
                ImGui::EndDisabled();

                // Darken / Lighten (Filter 7 from original implementations)
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Separator, PL::BorderV);
                ImGui::Separator();
                ImGui::PopStyleColor();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("DARKEN / LIGHTEN (Filter 7)");
                ImGui::PopStyleColor();
                ImGui::Spacing();

                int dlChoice = g_state.adjDarkenMode ? 0 : 1;
                ImGui::RadioButton("Darken (x0.5)##dl_opt", &dlChoice, 0);
                ImGui::SameLine(0, 16.0f);
                ImGui::RadioButton("Lighten (x1.5)##dl_opt", &dlChoice, 1);
                g_state.adjDarkenMode = (dlChoice == 0);

                ImGui::Spacing();
                if (ImGui::Button("Apply Darken / Lighten##dl_apply", {availW, 30.0f})) {
                    Bridge_DarkenAndLightenFilter(g_state.working, g_state.adjDarkenMode);
                    CommitHistory(g_state.working, true);
                    SetStatus(g_state.adjDarkenMode ? "Applied Darken (x0.5)." : "Applied Lighten (x1.5).");
                }
            }
            ImGui::EndTabItem();
        }

        // ====================================================================
        // 2. QUICK FILTERS (Independent zero-parameter filters, responsive 2-column layout)
        // ====================================================================
        if (ImGui::BeginTabItem("Quick Filters")) {
            s_firstFrameTab = false;
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Import an image to view real-time Quick Filter previews.");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Click any thumbnail to apply independently:");
                ImGui::PopStyleColor();
                ImGui::Spacing();

                // Dynamically calculate responsive content width to eliminate right-edge clipping
                float availW = ImGui::GetContentRegionAvail().x;
                const float spacingX = 8.0f;
                int cols = (availW >= 220.0f) ? 2 : 1;
                float cardW = std::floor((availW - spacingX * (cols - 1)) / cols);
                float imgW  = std::max(40.0f, cardW - 4.0f);
                float imgH  = std::floor(imgW * 0.80f);

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2.0f, 2.0f});
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {spacingX, 10.0f});

                for (size_t i = 0; i < g_quickFilterThumbs.size(); ++i) {
                    const auto& qf = g_quickFilterThumbs[i];

                    ImGui::BeginGroup();
                    ImGui::PushID(static_cast<int>(i));

                    ImVec2 cp = ImGui::GetCursorScreenPos();
                    bool clicked = false;

                    if (qf.srv) {
                        if (ImGui::ImageButton("##qfthumb", reinterpret_cast<ImTextureID>(qf.srv), {imgW, imgH})) {
                            clicked = true;
                        }
                    } else {
                        if (ImGui::Button("Preview...", {cardW, imgH})) {
                            clicked = true;
                        }
                    }

                    // Border highlight
                    bool isSelected = (g_state.activeQuickFilter == qf.id);
                    if (ImGui::IsItemHovered()) {
                        dl->AddRect({cp.x, cp.y}, {cp.x + cardW, cp.y + imgH + 4.0f}, IM_COL32(140, 95, 255, 255), 4.0f, 0, 2.0f);
                    } else if (isSelected) {
                        dl->AddRect({cp.x, cp.y}, {cp.x + cardW, cp.y + imgH + 4.0f}, IM_COL32(124, 77, 255, 255), 4.0f, 0, 2.0f);
                    } else {
                        dl->AddRect({cp.x, cp.y}, {cp.x + cardW, cp.y + imgH + 4.0f}, IM_COL32(50, 50, 75, 180), 4.0f, 0, 1.0f);
                    }

                    // Label centered beneath thumbnail
                    ImVec2 nameSz = ImGui::CalcTextSize(qf.name);
                    float textX = cp.x + std::max(0.0f, (cardW - nameSz.x) * 0.5f);
                    ImGui::SetCursorScreenPos({textX, cp.y + imgH + 8.0f});
                    ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? PL::AccentLightV : PL::TextPrimV);
                    ImGui::TextUnformatted(qf.name);
                    ImGui::PopStyleColor();

                    ImGui::PopID();
                    ImGui::EndGroup();

                    if (clicked) {
                        ApplyQuickFilter(qf.id);
                    }

                    if (cols > 1 && (i % cols) < (cols - 1)) {
                        ImGui::SameLine(0.0f, spacingX);
                    } else {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
                    }
                }
                ImGui::PopStyleVar(2);
            }
            ImGui::EndTabItem();
        }

        // ====================================================================
        // 3. GEOMETRY (Crop, Resize, Rotate, Flip, Skew)
        // ====================================================================
        if (ImGui::BeginTabItem("Geometry")) {
            s_firstFrameTab = false;
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Import an image to use geometry tools.");
                ImGui::PopStyleColor();
            } else {
                FilterParam& p = g_state.params;
                const int curW = g_state.working.width;
                const int curH = g_state.working.height;

                // Responsive content width: dynamically respects window padding, custom size, and vertical scrollbar
                const float availW = ImGui::GetContentRegionAvail().x;

                // --- CROP ---
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("CROP TOOL");
                ImGui::PopStyleColor();
                ImGui::TextColored(PL::TextMutedV, "Dimensions: %d x %d px", curW, curH);
                ImGui::Spacing();

                if (!g_state.isCropping) {
                    if (ImGui::Button("Activate Crop Mode##geom", {availW, 28.0f})) {
                        g_state.isCropping = true;
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, PL::AccentV);
                    if (ImGui::Button("Exit Crop Mode##geom", {availW, 28.0f})) {
                        g_state.isCropping = false;
                    }
                    ImGui::PopStyleColor();
                }

                ImGui::Spacing();
                const float cropLabelW = 24.0f;
                const float cropSliderW = std::max(40.0f, availW - cropLabelW);

                ImGui::SetNextItemWidth(cropSliderW);
                ImGui::SliderInt("X##crp", &p.cropX, 0, std::max(0, curW - 1), "%d px");
                ImGui::SetNextItemWidth(cropSliderW);
                ImGui::SliderInt("Y##crp", &p.cropY, 0, std::max(0, curH - 1), "%d px");
                ImGui::SetNextItemWidth(cropSliderW);
                ImGui::SliderInt("W##crp", &p.cropW, 1, std::max(1, curW - p.cropX), "%d px");
                ImGui::SetNextItemWidth(cropSliderW);
                ImGui::SliderInt("H##crp", &p.cropH, 1, std::max(1, curH - p.cropY), "%d px");

                ImGui::Spacing();
                const bool validCrop = (p.cropX >= 0 && p.cropY >= 0 && p.cropW > 0 && p.cropH > 0 &&
                                        p.cropX + p.cropW <= curW && p.cropY + p.cropH <= curH);

                const float cropBtnSpacing = 8.0f;
                const float applyCropW = std::floor((availW - cropBtnSpacing) * 0.60f);
                const float clearCropW = availW - cropBtnSpacing - applyCropW;

                if (!validCrop) ImGui::BeginDisabled();
                if (ImGui::Button("Apply Crop##geom_crp", {applyCropW, 28.0f})) {
                    Bridge_CropFilter(g_state.working, p.cropX, p.cropY, p.cropW, p.cropH);
                    p.cropX = 0; p.cropY = 0;
                    p.cropW = std::min(g_state.working.width, 200);
                    p.cropH = std::min(g_state.working.height, 200);
                    CommitHistory(g_state.working, true);
                    g_state.isCropping = false;
                    SetStatus("Crop applied.");
                }
                if (!validCrop) ImGui::EndDisabled();

                ImGui::SameLine(0.0f, cropBtnSpacing);
                if (ImGui::Button("Clear##geom_crp_clr", {clearCropW, 28.0f})) {
                    p.cropX = 0; p.cropY = 0;
                    p.cropW = curW; p.cropH = curH;
                    g_state.isCropping = false;
                }

                // --- RESIZE ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("RESIZE");
                ImGui::PopStyleColor();

                // Responsive layout:
                // When width is generous (>= 360 px), display side-by-side.
                // For standard/narrow panels, rearrange into two dedicated rows:
                // Height on the first row, Width on the second row.
                if (availW >= 360.0f) {
                    const float spacingX = 8.0f;
                    const float colW = std::floor((availW - spacingX) * 0.5f);
                    const float labelH = ImGui::CalcTextSize("Height").x + 8.0f;
                    const float labelW = ImGui::CalcTextSize("Width").x + 8.0f;

                    ImGui::SetNextItemWidth(std::max(50.0f, colW - labelH));
                    ImGui::InputInt("Height##rsz_h", &p.resizeH);
                    ImGui::SameLine(0.0f, spacingX);
                    ImGui::SetNextItemWidth(std::max(50.0f, colW - labelW));
                    ImGui::InputInt("Width##rsz_w", &p.resizeW);
                } else {
                    const float maxLabelW = std::max(ImGui::CalcTextSize("Height").x, ImGui::CalcTextSize("Width").x) + 10.0f;
                    const float inputW = std::max(60.0f, availW - maxLabelW);

                    ImGui::SetNextItemWidth(inputW);
                    ImGui::InputInt("Height##rsz_h", &p.resizeH);

                    ImGui::Spacing();

                    ImGui::SetNextItemWidth(inputW);
                    ImGui::InputInt("Width##rsz_w", &p.resizeW);
                }

                if (p.resizeH < 1) p.resizeH = 1;
                if (p.resizeW < 1) p.resizeW = 1;

                ImGui::Spacing();
                const bool validResize = (p.resizeW > 0 && p.resizeH > 0);
                if (!validResize) ImGui::BeginDisabled();
                if (ImGui::Button("Apply Resize##geom_rsz", {availW, 28.0f})) {
                    Bridge_ResizeFilter(g_state.working, p.resizeW, p.resizeH);
                    CommitHistory(g_state.working, true);
                    SetStatus("Resize applied.");
                }
                if (!validResize) ImGui::EndDisabled();

                // --- ROTATE ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("ROTATE");
                ImGui::PopStyleColor();

                const float rotSpacing = 6.0f;
                const float rotBtnW = std::floor((availW - rotSpacing * 2.0f) / 3.0f);
                const float rotBtnW3 = availW - (rotBtnW * 2.0f + rotSpacing * 2.0f);

                if (ImGui::Button("90 deg CW##r90", {rotBtnW, 28.0f})) {
                    Bridge_RotateFilter(g_state.working, 1);
                    CommitHistory(g_state.working, true);
                    SetStatus("Rotated 90 deg clockwise.");
                }
                ImGui::SameLine(0.0f, rotSpacing);
                if (ImGui::Button("180 deg##r180", {rotBtnW, 28.0f})) {
                    Bridge_RotateFilter(g_state.working, 2);
                    CommitHistory(g_state.working, true);
                    SetStatus("Rotated 180 deg.");
                }
                ImGui::SameLine(0.0f, rotSpacing);
                if (ImGui::Button("270 deg CW##r270", {rotBtnW3, 28.0f})) {
                    Bridge_RotateFilter(g_state.working, 3);
                    CommitHistory(g_state.working, true);
                    SetStatus("Rotated 270 deg clockwise.");
                }

                // --- FLIP ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("FLIP");
                ImGui::PopStyleColor();

                if (ImGui::Button("Flip Horizontal##geom_flip", {availW, 28.0f})) {
                    FlipFilter(g_state.working);
                    CommitHistory(g_state.working, true);
                    SetStatus("Flipped image horizontally.");
                }

                // --- SKEW ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("SKEW");
                ImGui::PopStyleColor();

                bool skewSliderMoved = false;
                ImGui::SetNextItemWidth(availW);
                if (ImGui::SliderFloat("##geom_skew", &p.skewAngle, 0.0f, 75.0f, "%.1f deg")) {
                    skewSliderMoved = true;
                }

                if (skewSliderMoved) {
                    if (!g_state.skewPreviewing) {
                        g_state.skewBaseline = g_state.working;
                        g_state.skewPreviewing = true;
                    }
                    if (p.skewAngle <= 0.001f) {
                        g_state.working = g_state.skewBaseline;
                        g_state.previewDirty = true;
                        SetStatus("Skew preview: 0.0 deg (unskewed).");
                    } else {
                        Image previewImg = g_state.skewBaseline;
                        Bridge_SkewFilter(previewImg, static_cast<double>(p.skewAngle));
                        g_state.working = previewImg;
                        g_state.previewDirty = true;
                        char msg[128];
                        snprintf(msg, sizeof(msg), "Skew preview: %.1f deg (live preview).", p.skewAngle);
                        SetStatus(msg);
                    }
                }

                ImGui::TextColored(PL::TextMutedV, "Drag slider for real-time live preview.");

                // Action buttons: Apply Skew and Reset side-by-side
                const float skewBtnSpacing = 8.0f;
                const float applySkewW = std::floor((availW - skewBtnSpacing) * 0.65f);
                const float resetSkewW = availW - skewBtnSpacing - applySkewW;

                const bool canApplySkew = (g_state.skewPreviewing && p.skewAngle > 0.001f);
                if (!canApplySkew) ImGui::BeginDisabled();
                ImGui::PushStyleColor(ImGuiCol_Button,        PL::AccentV);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::AccentHovV);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  PL::AccentActV);
                if (ImGui::Button("Apply Skew##geom_skew_btn", {applySkewW, 28.0f})) {
                    // Commit current preview to history
                    CommitHistory(g_state.working, false);
                    g_state.skewPreviewing = false;
                    p.skewAngle = 0.0f;
                    SetStatus("Skew applied and committed to history.");
                }
                ImGui::PopStyleColor(3);
                if (!canApplySkew) ImGui::EndDisabled();

                ImGui::SameLine(0.0f, skewBtnSpacing);

                // Reset button: sets angle to 0 degrees and restores unskewed preview
                const bool canReset = (g_state.skewPreviewing || p.skewAngle > 0.001f);
                if (!canReset) ImGui::BeginDisabled();
                if (ImGui::Button("Reset##geom_skew_reset", {resetSkewW, 28.0f})) {
                    if (g_state.skewPreviewing) {
                        g_state.working = g_state.skewBaseline;
                        g_state.previewDirty = true;
                        g_state.skewPreviewing = false;
                    }
                    p.skewAngle = 0.0f;
                    SetStatus("Skew reset to 0 degrees.");
                }
                if (!canReset) ImGui::EndDisabled();
            }
            ImGui::EndTabItem();
        }

        // ====================================================================
        // 4. EFFECTS (Frame, Sunlight, Oil Painting, Merge Images)
        // ====================================================================
        if (ImGui::BeginTabItem("Effects")) {
            s_firstFrameTab = false;
            ImGui::Spacing();
            if (!g_state.imageLoaded) {
                ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
                ImGui::TextWrapped("Import an image to apply special effects.");
                ImGui::PopStyleColor();
            } else {
                FilterParam& p = g_state.params;
                const float availW = ImGui::GetContentRegionAvail().x;

                // --- FRAME ---
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("FRAME FILTER (Filter 4)");
                ImGui::PopStyleColor();
                ImGui::TextColored(PL::TextMutedV, "Border Color (15 px fixed width):");

                float col[3] = { p.frameR / 255.0f, p.frameG / 255.0f, p.frameB / 255.0f };
                ImGui::SetNextItemWidth(availW);
                if (ImGui::ColorEdit3("##frcol_fx", col)) {
                    p.frameR = static_cast<int>(col[0] * 255.0f);
                    p.frameG = static_cast<int>(col[1] * 255.0f);
                    p.frameB = static_cast<int>(col[2] * 255.0f);
                }
                ImGui::Spacing();
                if (ImGui::Button("Apply Frame##fx_fr", {availW, 28.0f})) {
                    Bridge_FrameFilter(g_state.working, p.frameR, p.frameG, p.frameB);
                    CommitHistory(g_state.working, true);
                    SetStatus("Frame applied.");
                }

                // --- SUNLIGHT ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("SUNLIGHT FILTER (Filter 13)");
                ImGui::PopStyleColor();
                ImGui::TextColored(PL::TextMutedV, "Intensity (Red channel boost):");

                ImGui::SetNextItemWidth(availW);
                ImGui::SliderInt("##sun_fx", &p.sunIntensity, 1, 100);
                ImGui::Spacing();
                if (ImGui::Button("Apply Sunlight##fx_sun", {availW, 28.0f})) {
                    Bridge_SunlightFilter(g_state.working, p.sunIntensity);
                    CommitHistory(g_state.working, true);
                    SetStatus("Sunlight effect applied.");
                }

                // --- OIL PAINTING ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("OIL PAINTING (Filter 18)");
                ImGui::PopStyleColor();
                ImGui::TextColored(PL::TextMutedV, "Mode quantization in 3-pixel radius.");

                ImGui::Spacing();
                if (ImGui::Button("Apply Oil Painting##fx_oil", {availW, 28.0f})) {
                    OilPaintingFilter(g_state.working);
                    CommitHistory(g_state.working, true);
                    SetStatus("Oil Painting effect applied.");
                }

                // --- MERGE IMAGES ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, PL::AccentLightV);
                ImGui::TextUnformatted("MERGE IMAGES (Filter 9)");
                ImGui::PopStyleColor();

                const float browseW = 75.0f;
                const float browseSpacing = 6.0f;
                ImGui::SetNextItemWidth(std::max(50.0f, availW - browseW - browseSpacing));
                ImGui::InputText("##mpath_fx", p.mergePath, sizeof(p.mergePath));
                ImGui::SameLine(0.0f, browseSpacing);
                if (ImGui::Button("Browse##fx_mrg", {browseW, 0})) {
                    char path[512] = {};
                    if (OpenFileDialog(path, sizeof(path)))
                        strncpy_s(p.mergePath, sizeof(p.mergePath), path, _TRUNCATE);
                }

                ImGui::Spacing();
                ImGui::RadioButton("Resize to largest##mrg_m1", &p.mergeMode, 1);
                ImGui::SameLine(0, 10.0f);
                ImGui::RadioButton("Common area only##mrg_m2", &p.mergeMode, 2);

                const bool canMerge = (p.mergePath[0] != '\0');
                if (!canMerge) ImGui::BeginDisabled();
                ImGui::Spacing();
                if (ImGui::Button("Apply Merge##fx_mrg_btn", {availW, 28.0f})) {
                    Bridge_MergeFilter(g_state.working, std::string(p.mergePath), p.mergeMode);
                    CommitHistory(g_state.working, true);
                    SetStatus("Merged with second image.");
                }
                if (!canMerge) ImGui::EndDisabled();

                // --- RESET TO ORIGINAL ---
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,        PL::BtnDarkV);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PL::BtnDarkHovV);
                if (ImGui::Button("Reset to Original Image##fx_rst", {availW, 30.0f})) {
                    g_state.working = g_state.original;
                    CommitHistory(g_state.working, true);
                    SetStatus("Reset to original image.");
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ============================================================================
// Status Bar
// ============================================================================
void DrawStatusBar(float x, float y, float w, float h)
{
    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.075f, 0.075f, 0.118f, 1.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {12.0f, 4.0f});

    ImGui::Begin("##status", nullptr,
                 kPanelFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList* dl   = ImGui::GetWindowDrawList();
    ImVec2      wpos = ImGui::GetWindowPos();
    dl->AddLine({wpos.x, wpos.y}, {wpos.x + w, wpos.y}, PL::Border, 1.0f);

    // Status dot
    ImU32 dotCol = g_state.statusIsError ? PL::Error : PL::Success;
    dl->AddCircleFilled({wpos.x + 8.0f, wpos.y + h * 0.5f}, 4.0f, dotCol);

    ImGui::SetCursorPos({18.0f, (h - ImGui::GetTextLineHeight()) * 0.5f});
    ImGui::PushStyleColor(ImGuiCol_Text, g_state.statusIsError ? PL::ErrorV : PL::TextMutedV);
    ImGui::TextUnformatted(g_state.statusMsg);
    ImGui::PopStyleColor();

    if (g_state.imageLoaded) {
        char right[200];
        snprintf(right, sizeof(right), "Active: %s   %d x %d px",
                 g_state.loadedFilename, g_state.working.width, g_state.working.height);
        ImVec2 rsz = ImGui::CalcTextSize(right);
        ImGui::SetCursorPos({w - rsz.x - 14.0f, (h - ImGui::GetTextLineHeight()) * 0.5f});
        ImGui::PushStyleColor(ImGuiCol_Text, PL::TextMutedV);
        ImGui::TextUnformatted(right);
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// ============================================================================
// OLE Drag & Drop Implementation
// ============================================================================
static bool IsSupportedImageExtensionW(const std::wstring& path)
{
    size_t dot = path.find_last_of(L'.');
    if (dot == std::wstring::npos) return false;
    std::wstring ext = path.substr(dot);
    for (auto& c : ext) c = static_cast<wchar_t>(::towlower(c));
    return (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".bmp" || ext == L".tga");
}

static std::string WideToAnsiOrUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (count > 1) {
        std::string str(count - 1, '\0');
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, nullptr, nullptr);
        return str;
    }
    count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (count > 1) {
        std::string str(count - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, nullptr, nullptr);
        return str;
    }
    return {};
}

static bool HasSupportedDropFiles(IDataObject* pDataObj)
{
    if (!pDataObj) return false;
    FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stg;
    if (FAILED(pDataObj->GetData(&fmt, &stg))) {
        return false;
    }

    HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
    if (!hDrop) {
        ReleaseStgMedium(&stg);
        return false;
    }

    bool hasSupported = false;
    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
    for (UINT i = 0; i < count; ++i) {
        UINT len = DragQueryFileW(hDrop, i, nullptr, 0);
        if (len == 0) continue;
        std::wstring wpath(len + 1, L'\0');
        DragQueryFileW(hDrop, i, &wpath[0], len + 1);
        wpath.resize(len);

        if (IsSupportedImageExtensionW(wpath)) {
            hasSupported = true;
            break;
        }
    }

    GlobalUnlock(stg.hGlobal);
    ReleaseStgMedium(&stg);
    return hasSupported;
}

class PixelLabDropTarget : public IDropTarget {
private:
    LONG m_refCount;
    bool m_hasSupportedFiles;

public:
    PixelLabDropTarget() : m_refCount(1), m_hasSupportedFiles(false) {}
    virtual ~PixelLabDropTarget() {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IDropTarget) {
            *ppvObject = static_cast<IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) {
            delete this;
            return 0;
        }
        return count;
    }

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* pdwEffect) override {
        if (!pdwEffect) return E_INVALIDARG;
        m_hasSupportedFiles = HasSupportedDropFiles(pDataObj);
        if (m_hasSupportedFiles) {
            *pdwEffect = DROPEFFECT_COPY;
            g_state.isDragHovering = true;
        } else {
            *pdwEffect = DROPEFFECT_NONE;
            g_state.isDragHovering = false;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragOver(DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* pdwEffect) override {
        if (!pdwEffect) return E_INVALIDARG;
        if (m_hasSupportedFiles) {
            *pdwEffect = DROPEFFECT_COPY;
            g_state.isDragHovering = true;
        } else {
            *pdwEffect = DROPEFFECT_NONE;
            g_state.isDragHovering = false;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave() override {
        m_hasSupportedFiles = false;
        g_state.isDragHovering = false;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD /*grfKeyState*/, POINTL /*pt*/, DWORD* pdwEffect) override {
        g_state.isDragHovering = false;
        m_hasSupportedFiles = false;

        if (!pdwEffect) return E_INVALIDARG;
        *pdwEffect = DROPEFFECT_NONE;

        if (!pDataObj) return E_FAIL;

        FORMATETC fmt = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stg;
        if (FAILED(pDataObj->GetData(&fmt, &stg))) {
            return E_FAIL;
        }

        HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
        if (!hDrop) {
            ReleaseStgMedium(&stg);
            return E_FAIL;
        }

        UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
        int importedCount = 0;
        int rejectedCount = 0;

        for (UINT i = 0; i < fileCount; ++i) {
            UINT len = DragQueryFileW(hDrop, i, nullptr, 0);
            if (len == 0) continue;
            std::wstring wpath(len + 1, L'\0');
            DragQueryFileW(hDrop, i, &wpath[0], len + 1);
            wpath.resize(len);

            if (!IsSupportedImageExtensionW(wpath)) {
                rejectedCount++;
                continue;
            }

            std::string path = WideToAnsiOrUtf8(wpath);
            if (!path.empty()) {
                if (ImportAsset(path.c_str())) {
                    importedCount++;
                }
            }
        }

        GlobalUnlock(stg.hGlobal);
        ReleaseStgMedium(&stg);

        if (importedCount > 0) {
            *pdwEffect = DROPEFFECT_COPY;
            if (rejectedCount > 0) {
                char msg[128];
                snprintf(msg, sizeof(msg), "Imported %d image(s). Skipped %d unsupported file(s).", importedCount, rejectedCount);
                SetStatus(msg, false);
            }
            return S_OK;
        } else if (rejectedCount > 0) {
            SetStatus("Unsupported file format. Please drop JPG, PNG, BMP, or TGA files.", true);
            return S_OK;
        }

        return S_OK;
    }
};

static PixelLabDropTarget* g_pDropTarget = nullptr;

// ============================================================================
// Windows Message Procedure
// ============================================================================
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_DROPFILES: {
        g_state.isDragHovering = false;
        HDROP hDrop = reinterpret_cast<HDROP>(wParam);
        UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
        int importedCount = 0;
        int rejectedCount = 0;
        for (UINT i = 0; i < fileCount; ++i) {
            UINT len = DragQueryFileW(hDrop, i, nullptr, 0);
            if (len == 0) continue;
            std::wstring wpath(len + 1, L'\0');
            DragQueryFileW(hDrop, i, &wpath[0], len + 1);
            wpath.resize(len);

            if (!IsSupportedImageExtensionW(wpath)) {
                rejectedCount++;
                continue;
            }

            std::string path = WideToAnsiOrUtf8(wpath);
            if (!path.empty()) {
                if (ImportAsset(path.c_str())) {
                    importedCount++;
                }
            }
        }
        DragFinish(hDrop);
        if (importedCount == 0 && rejectedCount > 0) {
            SetStatus("Unsupported file format. Please drop JPG, PNG, BMP, or TGA files.", true);
        }
        return 0;
    }
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        g_ResizeWidth  = LOWORD(lParam);
        g_ResizeHeight = HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// WinMain — Application Entry Point
// ============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    HRESULT hrOle = OleInitialize(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"PixelLabWindow";
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_ACCEPTFILES,
        wc.lpszClassName,
        L"Pixel Lab - Image Processing Studio",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1440, 900,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        if (SUCCEEDED(hrOle)) OleUninitialize();
        return 1;
    }

    // Allow drag-drop messages through Windows UIPI if running with elevated privileges
    typedef BOOL(WINAPI* pfnChangeWindowMessageFilterEx)(HWND, UINT, DWORD, PCHANGEFILTERSTRUCT);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        auto pChangeFilter = reinterpret_cast<pfnChangeWindowMessageFilterEx>(
            GetProcAddress(hUser32, "ChangeWindowMessageFilterEx"));
        if (pChangeFilter) {
            pChangeFilter(g_hwnd, WM_DROPFILES, 1 /* MSGFLT_ALLOW */, nullptr);
            pChangeFilter(g_hwnd, WM_COPYDATA,  1 /* MSGFLT_ALLOW */, nullptr);
            pChangeFilter(g_hwnd, 0x0049 /* WM_COPYGLOBALDATA */, 1 /* MSGFLT_ALLOW */, nullptr);
        }
    }

    g_pDropTarget = new PixelLabDropTarget();
    RegisterDragDrop(g_hwnd, g_pDropTarget);
    DragAcceptFiles(g_hwnd, TRUE);

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ApplyPixelLabTheme(io);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main render loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (g_swapChainOccluded &&
            g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            Sleep(10);
            continue;
        }
        g_swapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                        DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        if (g_state.previewDirty) {
            UpdatePreviewTexture(g_state, g_pd3dDevice, g_pd3dDeviceContext);
            g_state.previewDirty = false;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ------------------------------------------------------------
        // Global Keyboard Shortcuts (Undo: Ctrl+Z, Redo: Ctrl+Shift+Z / Ctrl+Y)
        // Active anywhere in the application, respecting active text entry fields
        // ------------------------------------------------------------
        HWND fgWnd = GetForegroundWindow();
        bool isAppFocused = (fgWnd == g_hwnd || GetActiveWindow() == g_hwnd || fgWnd == nullptr);

        if (isAppFocused && !io.WantTextInput) {
            const bool ctrlDown  = io.KeyCtrl  || ((GetKeyState(VK_CONTROL) & 0x8000) != 0);
            const bool shiftDown = io.KeyShift || ((GetKeyState(VK_SHIFT)   & 0x8000) != 0);

            // ImGui 1.91 global shortcut routing
            bool undoShortcut = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteAlways);
            bool redoShortcut = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, ImGuiInputFlags_RouteAlways)
                             || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_RouteAlways);

            // Robust fallback key-press checks
            bool undoFallback = ctrlDown && !shiftDown && ImGui::IsKeyPressed(ImGuiKey_Z, false);
            bool redoFallback = (ctrlDown && shiftDown && ImGui::IsKeyPressed(ImGuiKey_Z, false))
                             || (ctrlDown && !shiftDown && ImGui::IsKeyPressed(ImGuiKey_Y, false));

            if (undoShortcut || undoFallback) {
                if (g_state.imageLoaded && g_state.historyIndex > 0) {
                    Undo();
                }
            } else if (redoShortcut || redoFallback) {
                if (g_state.imageLoaded && g_state.historyIndex >= 0 &&
                    g_state.historyIndex < static_cast<int>(g_state.history.size()) - 1) {
                    Redo();
                }
            }
        }

        const float W = io.DisplaySize.x;
        const float H = io.DisplaySize.y;
        const float contentY = kHeaderH;
        const float contentH = H - kHeaderH - kStatusH;
        const float centerW  = W - kLeftW - kRightW;

        DrawHeader();
        DrawLeftSidebar  (0.0f,             contentY, kLeftW,   contentH);
        DrawCenterCanvas (kLeftW,           contentY, centerW,  contentH);
        DrawRightPanel   (kLeftW + centerW, contentY, kRightW, contentH);
        DrawStatusBar    (0.0f,             H - kStatusH, W,    kStatusH);

        ImGui::Render();
        const float clearColor[4] = { 0.110f, 0.110f, 0.153f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup resources
    if (g_pDropTarget) {
        RevokeDragDrop(g_hwnd);
        g_pDropTarget->Release();
        g_pDropTarget = nullptr;
    }

    ReleasePreviewTexture(g_state);
    ClearQuickFilterThumbnails();
    for (auto& asset : g_state.assets) {
        if (asset.thumbSRV) {
            asset.thumbSRV->Release();
            asset.thumbSRV = nullptr;
        }
    }
    g_state.assets.clear();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    if (SUCCEEDED(hrOle)) {
        OleUninitialize();
    }

    return 0;
}
