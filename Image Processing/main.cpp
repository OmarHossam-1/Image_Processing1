/*
    CS213 - OOP Programming Assignment 1
    Part 1

    Team Members:
    20250117 - Belal Ibrahim Sayed
    20250426 - Omar Hossam Eldin Zidan
    20251551 - Ahmed
    20253056 - Shahd

    Section:

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
    Resizing = 8
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


void ApplyBlackAndWhite(Image& image)
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
                image(i, j, k) =
                    255 - image(i, j, k);
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

    cout << "Frame added successfully!\n";
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

                image(image.width - 1 - i, j, k) =
                    temp;
            }
        }
    }
}


void RotateImage(Image& image, int angle)
{
    // 90 degrees
    if (angle == 1)
    {
        Image rotated(image.height, image.width);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(image.height - 1 - j, i, 0) =
                    image(i, j, 0);

                rotated(image.height - 1 - j, i, 1) =
                    image(i, j, 1);

                rotated(image.height - 1 - j, i, 2) =
                    image(i, j, 2);
            }
        }

        image = rotated;
    }

    // 180 degrees
    else if (angle == 2)
    {
        Image rotated(image.width, image.height);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(image.width - 1 - i,
                    image.height - 1 - j, 0) =
                    image(i, j, 0);

                rotated(image.width - 1 - i,
                    image.height - 1 - j, 1) =
                    image(i, j, 1);

                rotated(image.width - 1 - i,
                    image.height - 1 - j, 2) =
                    image(i, j, 2);
            }
        }

        image = rotated;
    }

    // 270 degrees
    else if (angle == 3)
    {
        Image rotated(image.height, image.width);

        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                rotated(j, image.width - 1 - i, 0) =
                    image(i, j, 0);

                rotated(j, image.width - 1 - i, 1) =
                    image(i, j, 1);

                rotated(j, image.width - 1 - i, 2) =
                    image(i, j, 2);
            }
        }

        image = rotated;
    }
}


void DarkenAndLightenImage(Image& image)
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
                    image(i, j, k) =
                        (unsigned char)(image(i, j, k) * 0.5);
                }
                else
                {
                    image(i, j, k) =
                        (unsigned char)min(
                            255,
                            (int)(image(i, j, k) * 1.5)
                        );
                }
            }
        }
    }

    cout << "Brightness filter applied successfully!\n";
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
                resized(x, y, c) =
                    image(oldx, oldy, c);
            }
        }
    }

    image = resized;

    cout << "Image resized successfully!\n";
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
    cout << "\n=============================\n";
    cout << "         FILTERS MENU\n";
    cout << "=============================\n";

    cout << "1. Grayscale\n";
    cout << "2. Black and White\n";
    cout << "3. Invert Image\n";
    cout << "4. Frame\n";
    cout << "5. Flip\n";
    cout << "6. Rotate\n";
    cout << "7. Darken and Lighten\n";
    cout << "8. Resizing\n";
    cout << "0. Back\n";

    cout << "=============================\n";
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
        ApplyBlackAndWhite(image);
        cout << "Black and White applied successfully!\n";
        break;

    case Filters::InvertImage:
        InvertImageFilter(image);
        cout << "Invert applied successfully!\n";
        break;

    case Filters::Frame:
        FrameFilter(image);
        break;

    case Filters::Flip:
        FlipFilter(image);
        cout << "Flip applied successfully!\n";
        break;

    case Filters::Rotate:
    {
        int angle;

        cout << "\n1. Rotate 90 degrees\n";
        cout << "2. Rotate 180 degrees\n";
        cout << "3. Rotate 270 degrees\n";
        cout << "Enter your choice: ";
        cin >> angle;

        if (angle >= 1 && angle <= 3)
        {
            RotateImage(image, angle);
            cout << "Rotation applied successfully!\n";
        }
        else
        {
            cout << "Invalid rotation option\n";
        }

        break;
    }

    case Filters::DarkenAndLighten:
        DarkenAndLightenImage(image);
        break;

    case Filters::Resizing:
        ResizeFilter(image);
        break;

    default:
        cout << "Invalid filter number.\n";
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
        case 1:
            imageLoaded = LoadImage(image);
            break;

        case 2:
            if (imageLoaded)
            {
                FilterMenu(image);
            }
            else
            {
                cout << "Please load an image first.\n";
            }
            break;

        case 3:
            if (imageLoaded)
            {
                SaveImage(image);
            }
            else
            {
                cout << "Please load an image first.\n";
            }
            break;

        case 0:
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