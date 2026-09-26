#include<iostream>
#include<cstdlib>
#include"Image_Class.h"

using namespace std;

enum Filters {BlackAndWhite = 1 , Rotate = 2};

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
	if (angle == 1) // 90 degrees
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

	else if (angle == 2) // 180 degrees
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

	else if (angle == 3) // 270 degrees
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
}

int main()
{
	string filename;
	cout << "Please enter colored image name : ";
	cin >> filename;

	Image image(filename);

	int FilterNum;
	cout << "Please choose a filter : \n";
	cout << "1. Black and White \n";
	cout << "2. Rotate \n";
	cin >> FilterNum;

	switch (FilterNum)
	{
	case Filters::BlackAndWhite:

		ApplyBlackAndWhite(image);
		break;
		
	case Filters::Rotate:

		int angle;
		cout << "Please choose the angle of Rotation : \n";
		cout << "1. Rotate 90° \n";
		cout << "2. Rotate 180° \n";
		cout << "3. Rotate 270° \n";
		cin >> angle;

		RotateImage(image, angle);
		break;

	}

	cout << "Pls enter image name to store new image\n";
	cout << "and specify extension .jpg, .bmp, .png, .tga: ";

	cin >> filename;
	image.saveImage(filename);

	system(filename.c_str());

	return 0;
}