// Filters.h - Forward declarations for all 18 filter functions in main.cpp
// main.cpp is compiled into the GUI project (as main.obj under /SUBSYSTEM:WINDOWS).
// The linker resolves these declarations to the definitions in main.obj.
// DO NOT implement these functions here - they must remain in main.cpp unchanged.
#pragma once

// The Image type comes from the PCH (Image_Class.h compiled via pch.h).
// No include needed here.

// 1. Grayscale
void GrayscaleFilter(Image& image);
// 2. Black and White
void BlackAndWhiteFilter(Image& image);
// 3. Invert Image
void InvertImageFilter(Image& image);
// 4. Frame (reads R G B from std::cin)
void FrameFilter(Image& image);
// 5. Flip (horizontal)
void FlipFilter(Image& image);
// 6. Rotate (reads 1/2/3 from std::cin for 90/180/270 degrees)
void RotateFilter(Image& image);
// 7. Darken and Lighten (reads 1/2 from std::cin)
void DarkenAndLightenFilter(Image& image);
// 8. Resize (reads W H from std::cin)
void ResizeFilter(Image& image);
// 9. Merge Images (reads filename and mode from std::cin)
void MergeFilter(Image& image);
// 10. Edge Detection
void EdgeDetectionFilter(Image& image);
// 11. Crop (reads startX startY cropW cropH from std::cin)
void CropFilter(Image& image);
// 12. Blur (box blur, radius hardcoded to 3)
void BlurFilter(Image& img);
// 13. Sunlight (reads intensity 1-100 from std::cin)
void SunlightFilter(Image& image);
// 14. Old TV
void OldTVFilter(Image& image);
// 15. Purple Night
void PurpleNightFilter(Image& image);
// 16. Infrared
void InfraredFilter(Image& img);
// 17. Skew Image (reads angle from std::cin)
void SkewFilter(Image& image);
// 18. Oil Painting
void OilPaintingFilter(Image& img);
