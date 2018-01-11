#ifndef _BITMAP_H
#define _BITMAP_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Color.h"


class Bitmap {
public:
	//variables
	HBITMAP hbitmap;
	unsigned char *data;
	unsigned char *(*dataMatrix);
	int width, height;
	//methods
	Bitmap();
	Bitmap(int height, int width, int bpp);
	~Bitmap();

	bool loadBitmap24(char *filename);
	bool loadBitmap24B(char *filename);
	bool readMonochrome(char *filename);
	bool writeBitmap24(COLORREF *(*RGBMatrix), int height, int width);
	void setPixel24(int x, int y, Color &color);
	void setPixel24(int x, int y, int r, int g, int b);

private:
	//variables
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	BITMAPINFO* bmi;

	int	padWidth;											// widht of a padded row
	int	paddingByte;										// number of Bytes to fill up the row to a multiple of four

};

#endif