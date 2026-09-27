#include <iostream>
#include <cstdlib>
#include "Image_Class.h"

using namespace std;

enum Filters {
    Grayscale = 1,
    BlackAndWhite = 2,
    Frame = 4,
    Flip = 5,
    Rotate = 6,
    Resizing = 8
};

void grayscalefilter(Image& image)
{
    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            unsigned char R = image(i, j, 0);
            unsigned char G = image(i, j, 1);
            unsigned char B = image(i, j, 2);

            unsigned char gray = 0.299 * R + 0.587 * G + 0.114 * B;

            image(i, j, 0) = gray;
            image(i, j, 1) = gray;
            image(i, j, 2) = gray;
        }
    }
}

void flipfilter(Image& image)
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

void RotateImage(Image& image, int angle)
{
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

void framefilter(Image& image)
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

    cout << "Frame added\n";
}

void resizefilter(Image& image)
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
            int oldx = x * scalex;
            int oldy = y * scaley;

            for (int c = 0; c < image.channels; c++)
            {
                resized(x, y, c) =
                    image(oldx, oldy, c);
            }
        }
    }

    image = resized;

    cout << "Image resized\n";
}

int main()
{
    string filename;

    cout << "Please enter colored image name with extension: ";
    cin >> filename;

    Image image(filename);

    int FilterNum;

    cout << "\nPlease choose a filter from this menu:\n";
    cout << "1. Grayscale\n";
    cout << "2. Black and White\n";
    cout << "4. Frame\n";
    cout << "5. Flip\n";
    cout << "6. Rotate\n";
    cout << "8. Resizing\n";

    cout << "Enter your filter number: ";
    cin >> FilterNum;

    switch (FilterNum)
    {
        case Filters::BlackAndWhite:

            ApplyBlackAndWhite(image);

            break;

        case Filters::Rotate:
        {
            int angle;

            cout << "Please choose a rotation option:\n";
            cout << "1. Rotate 90 degrees\n";
            cout << "2. Rotate 180 degrees\n";
            cout << "3. Rotate 270 degrees\n";

            cin >> angle;

            if (angle < 1 || angle > 3)
            {
                cout << "Invalid rotation option\n";
                return 1;
            }

            RotateImage(image, angle);

            break;
        }

        case Filters::Grayscale:

            grayscalefilter(image);

            break;

        case Filters::Flip:

            flipfilter(image);

            break;

        case Filters::Frame:

            framefilter(image);

            break;

        case Filters::Resizing:

            resizefilter(image);

            break;

        default:

            cout << "Invalid filter number\n";

            return 1;
    }

    cout << "\nPlease enter image name to store new image\n";
    cout << "and specify extension .jpg, .bmp, .png, .tga: ";

    cin >> filename;

    image.saveImage(filename);

    cout << "Image saved successfully!\n";

    return 0;
}


