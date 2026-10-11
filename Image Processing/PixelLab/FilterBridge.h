// FilterBridge.h
// RAII-safe wrappers for the 8 filter functions that read from std::cin.
// The other 10 filters need no wrapping and are called directly.
//
// Strategy: Before calling each filter, construct the exact string the user
// would have typed, redirect std::cin to a std::istringstream containing that
// string, call the original filter function unchanged, then unconditionally
// restore std::cin via the RAII guard (even on exception).
#pragma once
#include <sstream>
#include <string>

#include <locale>

// =============================================================================
// CinRedirectGuard - RAII std::cin stream buffer and locale redirector
// Swaps std::cin's underlying streambuf and locale on construction, restores on destruction.
// Destructor fires unconditionally: normal return, early return, or exception.
// =============================================================================
class CinRedirectGuard {
    std::streambuf* savedBuf_;
    std::locale savedLoc_;
public:
    explicit CinRedirectGuard(std::streambuf* replacement)
        : savedBuf_(std::cin.rdbuf(replacement)),
          savedLoc_(std::cin.getloc()) {}

    ~CinRedirectGuard() {
        std::cin.rdbuf(savedBuf_);   // Always restored
        std::cin.imbue(savedLoc_);   // Always restored
    }

    void Imbue(const std::locale& loc) {
        std::cin.imbue(loc);
    }

    CinRedirectGuard(const CinRedirectGuard&)            = delete;
    CinRedirectGuard& operator=(const CinRedirectGuard&) = delete;
};

// =============================================================================
// Wrappers for the 8 cin-reading filter functions
// The original functions in main.cpp are called here, completely unchanged.
// Parameters are supplied by the GUI and formatted as the filter expects.
// =============================================================================

// FrameFilter reads: "R G B\n"  (three ints, space-separated)
void Bridge_FrameFilter(Image& img, int r, int g, int b);

// RotateFilter reads: "choice\n"  (1=90deg, 2=180deg, 3=270deg)
void Bridge_RotateFilter(Image& img, int choice);

// DarkenAndLightenFilter reads: "choice\n"  (1=darken, 2=lighten)
void Bridge_DarkenAndLightenFilter(Image& img, bool darken);

// ResizeFilter reads: "W H\n"  (two positive ints)
void Bridge_ResizeFilter(Image& img, int w, int h);

// MergeFilter reads: "filename\nchoice\n"  (path with NO spaces, then 1 or 2)
void Bridge_MergeFilter(Image& img, const std::string& path, int mode);

// SunlightFilter reads: "intensity\n"  (int in [1,100])
// The GUI clamps intensity before calling, so the while-retry loop exits immediately.
void Bridge_SunlightFilter(Image& img, int intensity);

// CropFilter reads: "startX startY\ncropW cropH\n"
void Bridge_CropFilter(Image& img, int startX, int startY, int cropW, int cropH);

// SkewFilter reads: "angle\n"  (double, in degrees)
void Bridge_SkewFilter(Image& img, double angle);
