#include "pch.h"
#include "Adjustments.h"
#include <algorithm>
#include <cmath>

static inline unsigned char clamp(int v) {
    return (unsigned char)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

void AdjustBrightness(Image& img, int value) {
    if (!img.imageData || img.width <= 0 || img.height <= 0) return;
    int n = img.width * img.height * 3;
    for (int i = 0; i < n; ++i) {
        img.imageData[i] = clamp(img.imageData[i] + value);
    }
}

void AdjustContrast(Image& img, float value) {
    if (!img.imageData || img.width <= 0 || img.height <= 0) return;
    int n = img.width * img.height * 3;
    for (int i = 0; i < n; ++i) {
        int v = img.imageData[i];
        float f = (v - 128.0f) * value + 128.0f;
        img.imageData[i] = clamp((int)f);
    }
}

void RGBtoHSV(float r, float g, float b, float& h, float& s, float& v) {
    float minC = std::min({r, g, b});
    float maxC = std::max({r, g, b});
    v = maxC;
    float delta = maxC - minC;
    if (delta < 0.0001f) {
        s = 0;
        h = 0;
        return;
    }
    if (maxC > 0.0f) {
        s = (delta / maxC);
    } else {
        s = 0;
        h = -1.0f;
        return;
    }
    if (r >= maxC)
        h = (g - b) / delta;
    else if (g >= maxC)
        h = 2.0f + (b - r) / delta;
    else
        h = 4.0f + (r - g) / delta;
    h *= 60.0f;
    if (h < 0.0f) h += 360.0f;
}

void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b) {
    if (s <= 0.0f) {
        r = v; g = v; b = v;
        return;
    }
    float hh = h;
    if (hh >= 360.0f) hh = 0.0f;
    hh /= 60.0f;
    int i = (int)hh;
    float ff = hh - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * ff));
    float t = v * (1.0f - (s * (1.0f - ff)));
    switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: default: r = v; g = p; b = q; break;
    }
}

void AdjustSaturation(Image& img, float value) {
    if (!img.imageData || img.width <= 0 || img.height <= 0) return;
    int n = img.width * img.height;
    for (int i = 0; i < n; ++i) {
        float r = img.imageData[i * 3 + 0] / 255.0f;
        float g = img.imageData[i * 3 + 1] / 255.0f;
        float b = img.imageData[i * 3 + 2] / 255.0f;
        float h, s, v;
        RGBtoHSV(r, g, b, h, s, v);
        s *= value;
        s = std::max(0.0f, std::min(1.0f, s));
        HSVtoRGB(h, s, v, r, g, b);
        img.imageData[i * 3 + 0] = clamp((int)(r * 255.0f));
        img.imageData[i * 3 + 1] = clamp((int)(g * 255.0f));
        img.imageData[i * 3 + 2] = clamp((int)(b * 255.0f));
    }
}

void AdjustHue(Image& img, float angle) {
    if (!img.imageData || img.width <= 0 || img.height <= 0) return;
    int n = img.width * img.height;
    for (int i = 0; i < n; ++i) {
        float r = img.imageData[i * 3 + 0] / 255.0f;
        float g = img.imageData[i * 3 + 1] / 255.0f;
        float b = img.imageData[i * 3 + 2] / 255.0f;
        float h, s, v;
        RGBtoHSV(r, g, b, h, s, v);
        h += angle;
        while (h >= 360.0f) h -= 360.0f;
        while (h < 0.0f) h += 360.0f;
        HSVtoRGB(h, s, v, r, g, b);
        img.imageData[i * 3 + 0] = clamp((int)(r * 255.0f));
        img.imageData[i * 3 + 1] = clamp((int)(g * 255.0f));
        img.imageData[i * 3 + 2] = clamp((int)(b * 255.0f));
    }
}
