#pragma once

class Image;

void AdjustBrightness(Image& img, int value);
void AdjustContrast(Image& img, float value);
void AdjustSaturation(Image& img, float value);
void AdjustHue(Image& img, float angle);
