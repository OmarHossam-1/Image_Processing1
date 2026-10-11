/*
    CS213 - OOP Programming Assignment 1
    Part 1

    Team Members:
    20250117 - Belal Ibrahim Sayed
    20250426 - Omar Hossam Eldin Zidan
    20251551 - Ahmed Mahmoud Roshdy
    20253056 - Shahd Ali Mohammed

    Section: (S7,S8)

    Filter Distribution:
    Belal  : Filters 1, 5, 9, 13
    Omar   : Filters 2, 6, 10, 14
    Ahmed  : Filters 3, 7, 11, 15
    Shahd  : Filters 4, 8, 12, 16

    Part 1:
    8 Filters + Console Menu
*/

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "Image_Class.h"

using namespace std;

enum Filters
{
    Grayscale = 1,
    BlackAndWhite = 2,
    InvertImage = 3,
    Frame = 4,
    Flip = 5,
    Rotate = 6,
    DarkenAndLighten = 7,
    Resize = 8,
    MergeImages = 9,
    EdgeDetection = 10,
    Crop = 11,
    Blur = 12,
    Sunlight = 13,
    OldTV = 14,
    PurpleNight = 15,
    Infrared = 16,
    SkewImage = 17,
    OilPainting = 18,
};

enum MainMenuChoices 
{ 
    Load_Image = 1,
    Apply_Filter = 2,
    Save_Image = 3,
    Back = 4 
};


 // FILTERS

void GrayscaleFilter(Image& image)
{
    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            unsigned char R = image(i, j, 0);
            unsigned char G = image(i, j, 1);
            unsigned char B = image(i, j, 2);

            unsigned char gray =
                (unsigned char)(0.299 * R + 0.587 * G + 0.114 * B);

            image(i, j, 0) = gray;
            image(i, j, 1) = gray;
            image(i, j, 2) = gray;
        }
    }
}

void BlackAndWhiteFilter(Image& image)
{
    for (int i = 0; i < image.width; i++)
    {
        for (int j = 0; j < image.height; j++)
        {
            int avg = 0;

            for (int k = 0; k < 3; k++)
            {
                avg = avg + image(i, j, k);
            }

            avg = avg / 3;

            if (avg < 128)
                avg = 0;

            else
                avg = 255;

            image(i, j, 0) = avg;
            image(i, j, 1) = avg;
            image(i, j, 2) = avg;
        }
    }
}

void InvertImageFilter(Image& image)
{
    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                image(i, j, k) = 255 - image(i, j, k);
            }
        }
    }
}

void FrameFilter(Image& image)
{
    int border = 15;
    int r, g, b;

    cout << "Enter RGB color for frame (0-255): ";
    cin >> r >> g >> b;

    if (r < 0 || r > 255 ||
        g < 0 || g > 255 ||
        b < 0 || b > 255)
    {
        cout << "Invalid RGB values\n";
        return;
    }

    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            if (i < border ||
                i >= image.width - border ||
                j < border ||
                j >= image.height - border)
            {
                image(i, j, 0) = r;
                image(i, j, 1) = g;
                image(i, j, 2) = b;
            }
        }
    }
}

void FlipFilter(Image& image)
{
    for (int i = 0; i < image.width / 2; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                unsigned char temp = image(i, j, k);

                image(i, j, k) =
                    image(image.width - 1 - i, j, k);

                image(image.width - 1 - i, j, k) = temp;
            }
        }
    }
}

void RotateFilter(Image& image)
{
    int rotation;

    cout << "\n1. Rotate 90 degrees\n";
    cout << "2. Rotate 180 degrees\n";
    cout << "3. Rotate 270 degrees\n";
    cout << "Enter your choice: ";
    cin >> rotation;

    if (rotation == 1)
    {
        Image rotated(image.height, image.width);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(image.height - 1 - j, i, 0) = image(i, j, 0);
                rotated(image.height - 1 - j, i, 1) = image(i, j, 1);
                rotated(image.height - 1 - j, i, 2) = image(i, j, 2);
            }
        }

        image = rotated;
    }
    else if (rotation == 2)
    {
        Image rotated(image.width, image.height);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(image.width - 1 - i, image.height - 1 - j, 0) = image(i, j, 0);
                rotated(image.width - 1 - i, image.height - 1 - j, 1) = image(i, j, 1);
                rotated(image.width - 1 - i, image.height - 1 - j, 2) = image(i, j, 2);
            }
        }

        image = rotated;
    }

    else if (rotation == 3)
    {
        Image rotated(image.height, image.width);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(j, image.width - 1 - i, 0) = image(i, j, 0);
                rotated(j, image.width - 1 - i, 1) = image(i, j, 1);
                rotated(j, image.width - 1 - i, 2) = image(i, j, 2);
            }
        }

        image = rotated;
    }

    else
    {
        cout << "Invalid rotation option.\n";
    }
}

void DarkenAndLightenFilter(Image& image)
{
    int choice;

    cout << "\n1. Darken Image\n";
    cout << "2. Lighten Image\n";
    cout << "Enter choice: ";
    cin >> choice;

    if (choice != 1 && choice != 2)
    {
        cout << "Invalid choice\n";
        return;
    }

    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                if (choice == 1)
                {
                    image(i, j, k) = (unsigned char)(image(i, j, k) * 0.5);
                }
                else
                {
                    image(i, j, k) = (unsigned char)min(255,(int)(image(i, j, k) * 1.5));
                }
            }
        }
    }
}

void ResizeFilter(Image& image)
{
    int W, H;

    cout << "Enter new width and height: ";
    cin >> W >> H;

    if (W <= 0 || H <= 0)
    {
        cout << "Invalid width or height\n";
        return;
    }

    Image resized(W, H);

    float scalex = (float)image.width / W;
    float scaley = (float)image.height / H;

    for (int x = 0; x < W; x++)
    {
        for (int y = 0; y < H; y++)
        {
            int oldx = (int)(x * scalex);
            int oldy = (int)(y * scaley);

            for (int c = 0; c < image.channels; c++)
            {
                resized(x, y, c) = image(oldx, oldy, c);
            }
        }
    }

    image = resized;
}

void MergeFilter(Image& image)
{
    string filename;
    int choice;

    cout << "Enter second image name with extension: ";
    cin >> filename;

    Image image2;

    try
    {
        image2.loadNewImage(filename);
    }
    catch (...)
    {
        cout << "Failed to load second image.\n";
        return;
    }

    cout << "\nChoose merge method:\n";
    cout << "1. Resize both images to the largest dimensions\n";
    cout << "2. Merge the common area only\n";
    cout << "Enter your choice: ";
    cin >> choice;

    int width, height;

    if (choice == 1)
    {
        width = max(image.width, image2.width);
        height = max(image.height, image2.height);

        Image result(width, height);

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                int x1 = i * image.width / width;
                int y1 = j * image.height / height;

                int x2 = i * image2.width / width;
                int y2 = j * image2.height / height;

                for (int k = 0; k < 3; k++)
                {
                    result(i, j, k) =
                        (image(x1, y1, k) + image2(x2, y2, k)) / 2;
                }
            }
        }

        image = result;
    }
    else if (choice == 2)
    {
        width = min(image.width, image2.width);
        height = min(image.height, image2.height);

        Image result(width, height);

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    result(i, j, k) =
                        (image(i, j, k) + image2(i, j, k)) / 2;
                }
            }
        }

        image = result;
    }
    else
    {
        cout << "Invalid choice.\n";
        return;
    }

}

void SunlightFilter(Image& image)
{
    int intensity;

    cout << "Enter sunlight intensity (1-100): ";
    cin >> intensity;

    while (intensity < 1 || intensity > 100)
    {
        cout << "Invalid value. Enter intensity (1-100): ";
        cin >> intensity;
    }

    for (int i = 0; i < image.width; i++)
    {
        for (int j = 0; j < image.height; j++)
        {
            int r = image(i, j, 0) + intensity;

            if (r > 255)
                r = 255;

            image(i, j, 0) = r;
        }
    }
}

void BlurFilter(Image& img) { int rad = 3; Image temp(img.width, img.height);
for (int i = 0; i < img.width; i++) {
    for (int j = 0; j < img.height; j++) {
        for (int k = 0; k < img.channels; k++) {
            int sum = 0;
            int count = 0;

            for (int dx = -rad; dx <= rad; dx++) {
                for (int dy = -rad; dy <= rad; dy++) {
                    int ni = i + dx;
                    int nj = j + dy;

                    if (ni >= 0 && ni < img.width && nj >= 0 && nj < img.height) {
                        sum += img(ni, nj, k);
                        count++;
                    }
                }
            }
            temp(i, j, k) = sum / count;
        }
    }
}
img = temp;
}


void InfraredFilter(Image& img) {
    for (int i = 0; i < img.width; i++) {
        for (int j = 0; j < img.height; j++) {

            int r = img(i, j, 0);
            int g = img(i, j, 1);
            int b = img(i, j, 2);

            int intensity = (r + g + b) / 3;

            int newR, newG, newB;

            if (intensity < 85) {
                newR = 0;
                newG = 0;
                newB = intensity * 3;
            }
            else if (intensity < 170) {
                newR = (intensity - 85) * 3;
                newG = 0;
                newB = 255 - (intensity - 85) * 3;
            }
            else {
                newR = 255;
                newG = (intensity - 170) * 3;
                newB = 0;
            }

            img(i, j, 0) = newR;
            img(i, j, 1) = newG;
            img(i, j, 2) = newB;
        }
    }

}


void EdgeDetectionFilter(Image& image)
{
    Image result(image.width, image.height);

    for (int i = 0; i < image.width; i++)
    {
        for (int j = 0; j < image.height; j++)
        {
            int current = (image(i, j, 0) +
                image(i, j, 1) +
                image(i, j, 2)) / 3;

            bool edge = false;

            // compare with the pixel on the right
            if (i < image.width - 1)
            {
                int right = (image(i + 1, j, 0) +
                    image(i + 1, j, 1) +
                    image(i + 1, j, 2)) / 3;

                if (abs(current - right) > 30)
                    edge = true;
            }

            // compare with the pixel below
            if (j < image.height - 1)
            {
                int below = (image(i, j + 1, 0) +
                    image(i, j + 1, 1) +
                    image(i, j + 1, 2)) / 3;

                if (abs(current - below) > 30)
                    edge = true;
            }

            if (edge)
            {
                result(i, j, 0) = 0;
                result(i, j, 1) = 0;
                result(i, j, 2) = 0;
            }
            else
            {
                result(i, j, 0) = 255;
                result(i, j, 1) = 255;
                result(i, j, 2) = 255;
            }
        }
    }

    image = result;
}

void OldTVFilter(Image& image)
{
    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            int r = image(x, y, 0);
            int g = image(x, y, 1);
            int b = image(x, y, 2);

            if (y % 2 == 1)
            {
                r = r / 2;
                g = g / 2;
                b = b / 2;
            }

            image(x, y, 0) = r;
            image(x, y, 1) = g;
            image(x, y, 2) = b;
        }
    }
}

void OilPaintingFilter(Image& img) { int radius = 3; int levels = 20;
Image temp(img.width, img.height);

for (int i = 0; i < img.width; i++) {
    for (int j = 0; j < img.height; j++) {

        int count[20] = {0};
        int sumR[20] = {0};
        int sumG[20] = {0};
        int sumB[20] = {0};

        for (int dx = -radius; dx <= radius; dx++) {
            for (int dy = -radius; dy <= radius; dy++) {
                int ni = i + dx;
                int nj = j + dy;

                if (ni >= 0 && ni < img.width && nj >= 0 && nj < img.height) {
                    int r = img(ni, nj, 0);
                    int g = img(ni, nj, 1);
                    int b = img(ni, nj, 2);

                    int lvl = ((r + g + b) / 3) * levels / 256;

                    count[lvl]++;
                    sumR[lvl] += r;
                    sumG[lvl] += g;
                    sumB[lvl] += b;
                }
            }
        }

        int maxIndex = 0;
        for (int k = 1; k < levels; k++) {
            if (count[k] > count[maxIndex]) {
                maxIndex = k;
            }
        }

        temp(i, j, 0) = sumR[maxIndex] / count[maxIndex];
        temp(i, j, 1) = sumG[maxIndex] / count[maxIndex];
        temp(i, j, 2) = sumB[maxIndex] / count[maxIndex];
    }
}

img = temp;
}

void CropFilter(Image& image)
{
    int startX, startY, cropWidth, cropHeight;

    cout << "Enter starting X and Y: ";
    cin >> startX >> startY;

    cout << "Enter crop width and height: ";
    cin >> cropWidth >> cropHeight;

    if (startX < 0 || startY < 0 ||
        cropWidth <= 0 || cropHeight <= 0 ||
        startX + cropWidth > image.width ||
        startY + cropHeight > image.height)
    {
        cout << "Invalid crop dimensions or position!\n";
        return;
    }

    Image cropped(cropWidth, cropHeight);

    for (int x = 0; x < cropWidth; x++)
    {
        for (int y = 0; y < cropHeight; y++)
        {
            for (int c = 0; c < image.channels; c++)
            {
                cropped(x, y, c) =
                    image(startX + x, startY + y, c);
            }
        }
    }

    image = cropped;

    cout << "Image cropped successfully!\n";
}

void PurpleNightFilter(Image& image)
{
    for (int i = 0; i < image.width; i++)
    {
        for (int j = 0; j < image.height; j++)
        {
            unsigned char R = image(i, j, 0);
            unsigned char G = image(i, j, 1);
            unsigned char B = image(i, j, 2);

            image(i, j, 0) = (R + B) / 2;
            image(i, j, 1) = G * 0.3;
            image(i, j, 2) = min(255, (int)(B * 1.5));
        }
    }
}

void SkewFilter(Image& image)
{
    double angle;
    cout << "Enter angle to skew: ";
    cin >> angle;

    // Convert angle to radians and calculate shift
    double rad = angle * 3.14159 / 180.0;
    int max_shift = image.height * tan(rad);

    Image new_image(image.width + max_shift, image.height);

    for (int j = 0; j < image.height; ++j) {
        int shift = (image.height - j) * tan(rad);
        for (int i = 0; i < image.width; ++i) {
            for (int k = 0; k < 3; ++k) {
                new_image(i + shift, j, k) = image(i, j, k);
            }
        }
    }
    image = new_image;
}


// LOAD / SAVE

bool LoadImage(Image& image)
{
    string filename;

    cout << "\nEnter colored image name with extension: ";
    cin >> filename;

    try
    {
        image.loadNewImage(filename);

        cout << "Image loaded successfully!\n";
        return true;
    }
    catch (...)
    {
        cout << "Failed to load image.\n";
        return false;
    }
}

void SaveImage(Image& image)
{
    string filename;

    cout << "\nEnter image name to store new image\n";
    cout << "and specify extension .jpg, .bmp, .png, .tga: ";

    cin >> filename;

    image.saveImage(filename);

    cout << "Image saved successfully!\n";

    system(filename.c_str());
}


// FILTER MENU

void ShowFilterMenu()
{
    cout << "\n==============================\n";
    cout << "          FILTERS MENU\n";
    cout << "==============================\n";

    cout << " 1. Grayscale\n";
    cout << " 2. Black and White\n";
    cout << " 3. Invert Image\n";
    cout << " 4. Frame\n";
    cout << " 5. Flip\n";
    cout << " 6. Rotate\n";
    cout << " 7. Darken and Lighten\n";
    cout << " 8. Resize Image\n";
    cout << " 9. Merge Images\n";
    cout << "10. Edge Detection\n";
    cout << "11. Crop\n";
    cout << "12. Blur\n";
    cout << "13. Sunlight\n";
    cout << "14. Old TV\n";
    cout << "15. Purple Night\n";
    cout << "16. Infrared\n";
    cout << "17. Skew Image\n";
    cout << "18. Oil Painting\n";

    cout << " 0. Back\n";

    cout << "==============================\n";
}

void ApplyFilter(Image& image, int choice)
{
    switch (choice)
    {
    case Filters::Grayscale:

        GrayscaleFilter(image);
        cout << "Grayscale applied successfully!\n";
        break;

    case Filters::BlackAndWhite:

        BlackAndWhiteFilter(image);
        cout << "Black and White applied successfully!\n";
        break;

    case Filters::InvertImage:

        InvertImageFilter(image);
        cout << "Invert Image applied successfully!\n";
        break;

    case Filters::Frame:

        FrameFilter(image);
        cout << "Frame applied successfully!\n";
        break;

    case Filters::Flip:

        FlipFilter(image);
        cout << "Flip applied successfully!\n";
        break;

    case Filters::Rotate:

        RotateFilter(image);
        cout << "Rotate applied successfully!\n";
        break;

    case Filters::DarkenAndLighten:

        DarkenAndLightenFilter(image);
        cout << "Darken and Lighten applied successfully!\n";
        break;

    case Filters::Resize:

        ResizeFilter(image);
        cout << "Resizing applied successfully!\n";
        break;

    case Filters::MergeImages:

        MergeFilter(image);
        cout << "Merge Images applied successfully!\n";
        break;

    case Filters::EdgeDetection:

        EdgeDetectionFilter(image);
        cout << "Edge Detection applied successfully!\n";
        break;

    case Filters::Blur:

        BlurFilter(image);
        cout << "Blur applied successfully!\n";
        break;

    case Filters::Sunlight:

        SunlightFilter(image);
        cout << "Sunlight applied successfully!\n";
        break;

    case Filters::OldTV:

        OldTVFilter(image);
        cout << "Old TV applied successfully!\n";
        break;

    case Filters::Infrared:

        InfraredFilter(image);
        cout << "Infrared applied successfully!\n";
        break;

        case Filters::OilPainting:
            
        OilPaintingFilter(image);
        cout << "Oil Painting applied successfully!\n";
        break;

        case Filters::Crop:

            CropFilter(image);
            cout << "Crop Filter applied successfully!\n";
            break;

        case Filters::PurpleNight:

            PurpleNightFilter(image);
            cout << "Purple Night Filter applied successfully!\n";
            break;

        case Filters::SkewImage:

            SkewFilter(image);
            cout << "Skew Image applied successfully!\n";
            break;

       default:
           cout << "Invalid filter number.\n";
           break;
    }  
}
    
void FilterMenu(Image& image)
{
    int choice;

    do
    {
        ShowFilterMenu();

        cout << "Enter your filter choice: ";
        cin >> choice;

        if (choice != 0)
        {
            ApplyFilter(image, choice);
        }

    } while (choice != 0);
}


// MAIN MENU

void ShowMainMenu()
{
    cout << "\n=============================\n";
    cout << "       IMAGE PROCESSING\n";
    cout << "=============================\n";

    cout << "1. Load Image\n";
    cout << "2. Apply Filter\n";
    cout << "3. Save Image\n";
    cout << "0. Exit\n";

    cout << "=============================\n";
}

void MainMenu()
{
    Image image;
    bool imageLoaded = false;

    int choice;

    do
    {
        ShowMainMenu();

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case MainMenuChoices::Load_Image :

            imageLoaded = LoadImage(image);
            break;

        case MainMenuChoices::Apply_Filter :

            if (imageLoaded)
            {
                FilterMenu(image);
            }
            else
            {
                cout << "Please load an image first.\n";
            }
            break;

        case MainMenuChoices::Save_Image :

            if (imageLoaded)
            {
                SaveImage(image);
            }
            else
            {
                cout << "Please load an image first.\n";
            }
            break;

        case MainMenuChoices::Back :

            cout << "Exiting program...\n";
            break;

        default:
            cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);
}



int main()
{
    MainMenu();

    return 0;
}
