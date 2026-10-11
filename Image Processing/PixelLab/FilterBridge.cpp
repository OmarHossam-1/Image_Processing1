// FilterBridge.cpp - RAII-safe cin wrappers for the 8 cin-reading filters
// These wrappers call the ORIGINAL filter functions in main.cpp exactly as-is.
// Nothing here duplicates filter logic. The cin redirect is purely mechanical.
#include "pch.h"
#include <vector>
#include "FilterBridge.h"
#include "Filters.h"

// Helper: build input string, redirect cin, call filter, restore cin on any exit
// All bridge functions follow this identical pattern.

void Bridge_FrameFilter(Image& img, int r, int g, int b)
{
    // FrameFilter reads:  r <SP> g <SP> b <NL>
    std::ostringstream oss;
    oss << r << " " << g << " " << b << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());   // cin redirected
    FrameFilter(img);                        // original function, unmodified
}                                            // guard destructor restores cin

void Bridge_RotateFilter(Image& img, int choice)
{
    // RotateFilter reads:  choice <NL>  (1=90, 2=180, 3=270)
    std::ostringstream oss;
    oss << choice << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    RotateFilter(img);
}

void Bridge_DarkenAndLightenFilter(Image& img, bool darken)
{
    // DarkenAndLightenFilter reads:  choice <NL>  (1=darken, 2=lighten)
    std::ostringstream oss;
    oss << (darken ? 1 : 2) << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    DarkenAndLightenFilter(img);
}

void Bridge_ResizeFilter(Image& img, int w, int h)
{
    // ResizeFilter reads:  W <SP> H <NL>
    std::ostringstream oss;
    oss << w << " " << h << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    ResizeFilter(img);
}

// Custom locale ctype that treats spaces as non-whitespace so std::cin >> std::string reads past spaces
struct NoSpaceCtype : std::ctype<char> {
    NoSpaceCtype() : std::ctype<char>(get_table()) {}
    static mask const* get_table() {
        static std::vector<mask> rc(table_size, std::ctype_base::mask());
        static bool initialized = false;
        if (!initialized) {
            for (size_t i = 0; i < table_size; ++i)
                rc[i] = std::ctype<char>::classic_table()[i];
            rc[' '] &= ~std::ctype_base::space;
            rc['\t'] &= ~std::ctype_base::space;
            rc['\n'] |= std::ctype_base::space;
            initialized = true;
        }
        return rc.data();
    }
};

void Bridge_MergeFilter(Image& img, const std::string& path, int mode)
{
    // MergeFilter reads:  filename <NL> choice <NL>
    // Using NoSpaceCtype allows original cin >> to read paths with spaces.
    std::ostringstream oss;
    oss << path << "\n" << mode << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    guard.Imbue(std::locale(std::locale::classic(), new NoSpaceCtype));
    MergeFilter(img);
}

void Bridge_SunlightFilter(Image& img, int intensity)
{
    // SunlightFilter reads:  intensity <NL>
    // It has a while-retry loop for invalid values (< 1 or > 100).
    // The GUI clamps intensity to [1,100] before calling, so the loop exits
    // immediately after the first successful read.
    intensity = std::max(1, std::min(100, intensity));
    std::ostringstream oss;
    oss << intensity << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    SunlightFilter(img);
}

void Bridge_CropFilter(Image& img, int startX, int startY, int cropW, int cropH)
{
    // CropFilter reads:  startX <SP> startY <NL> cropW <SP> cropH <NL>
    std::ostringstream oss;
    oss << startX << " " << startY << "\n"
        << cropW  << " " << cropH  << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    CropFilter(img);
}

void Bridge_SkewFilter(Image& img, double angle)
{
    // SkewFilter reads:  angle <NL>  (double, in degrees)
    std::ostringstream oss;
    oss << angle << "\n";
    std::istringstream input(oss.str());
    CinRedirectGuard guard(input.rdbuf());
    SkewFilter(img);
}
