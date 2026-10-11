// AppState.h - Application state for Pixel Lab
// Contains FilterID enum, FilterParam struct, and AppState aggregate.
// Uses a forward declaration for ID3D11ShaderResourceView to avoid pulling
// in d3d11.h from every file that uses AppState.
#pragma once
#include <string>
#include <vector>

struct ID3D11ShaderResourceView;  // Forward decl — d3d11.h not needed here

// =============================================================================
// Filter identifier enum
// Ordered to match the filter metadata array in GUI.cpp.
// =============================================================================
enum class FilterID {
    None = -1,
    Grayscale = 0,
    BlackAndWhite,
    Invert,
    Frame,
    Flip,
    Rotate,
    DarkenLighten,
    Resize,
    Merge,
    EdgeDetection,
    Crop,
    Blur,
    Sunlight,
    OldTV,
    PurpleNight,
    Infrared,
    SkewImage,
    OilPainting,
    Count
};

// =============================================================================
// Filter parameter storage
// All parameters for all 18 filters stored here with sensible defaults.
// The GUI writes into this struct; FilterBridge reads from it.
// =============================================================================
struct FilterParam {
    // Frame
    int   frameR = 255, frameG = 0, frameB = 0;

    // Rotate (1=90, 2=180, 3=270)
    int   rotateChoice = 1;

    // Darken/Lighten
    bool  darken = true;

    // Resize
    int   resizeW = 640, resizeH = 480;

    // Merge
    char  mergePath[512] = "";
    int   mergeMode = 1;        // 1=resize to largest, 2=common area

    // Sunlight
    int   sunIntensity = 50;    // [1, 100]

    // Crop
    int   cropX = 0, cropY = 0, cropW = 200, cropH = 200;

    // Skew
    float skewAngle = 0.0f;   // degrees (0 = unskewed)
};

// =============================================================================
// Asset Item - Represents an imported image asset with its own editing state
// =============================================================================
struct AssetItem {
    std::string path;
    std::string filename;
    Image       original;
    Image       baseState;
    Image       working;
    FilterID    activeQuickFilter = FilterID::None;
    std::vector<Image> history;
    int         historyIndex = -1;
    ID3D11ShaderResourceView* thumbSRV = nullptr;
    int         thumbW = 0;
    int         thumbH = 0;
    bool        isModified = false;
};

// =============================================================================
// Application state — single global instance in GUI.cpp
// =============================================================================
struct AppState {
    // Assets list and active asset index
    std::vector<AssetItem> assets;
    int  activeAssetIndex = -1;
    bool showDeleteConfirm = false;

    // Active image references (synced with active asset)
    Image  original;
    Image  working;
    bool   imageLoaded = false;

    char   loadedPath[512]      = "";
    char   loadedFilename[256]  = "";
    char   statusMsg[512]       = "Ready - Import an image to begin.";
    bool   statusIsError        = false;

    FilterID    selectedFilter = FilterID::None;
    FilterID    activeQuickFilter = FilterID::None;
    FilterParam params;

    // Search buffer for any text filtering
    char   searchBuf[128] = "";

    // DX11 preview texture — managed by Renderer.cpp
    ID3D11ShaderResourceView* previewSRV = nullptr;
    int    texWidth  = 0;
    int    texHeight = 0;
    bool   previewDirty = false;  // Set to true whenever working changes

    // Zoom control
    float  zoomLevel = -1.0f; // -1.0f means Fit to Window
    float  scrollX = 0.0f;
    float  scrollY = 0.0f;

    // Undo / Redo history for active asset
    std::vector<Image> history;
    int historyIndex = -1;

    // Interactive Crop State
    bool isCropping = false;

    // Drag and Drop hover state
    bool isDragHovering = false;

    // Adjust Panel Live State (preview only, resets on commit)
    Image adjBaseline;
    int   adjBrightness = 0;
    float adjContrast   = 1.0f;
    float adjSaturation = 1.0f;
    float adjHue        = 0.0f;
    bool  adjDarkenMode = true;
    bool  adjPreviewing = false;

    // Skew Tool Live State (preview only, resets on commit)
    Image skewBaseline;
    bool  skewPreviewing = false;
};
