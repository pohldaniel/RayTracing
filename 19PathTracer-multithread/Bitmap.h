#ifndef _BITMAP_H
#define _BITMAP_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

#include "Color.h"


class Bitmap {
public:
	//variables
	HBITMAP hbitmap;
	unsigned char *data;
	COLORREF** RGBMatrix;
	int width, height, padWidth; // widht of a padded row
												
	//methods
	Bitmap();
	Bitmap(int height, int width, int bpp);
	~Bitmap();

	bool loadBitmap24(const char *filename);
	
	void flipVertical();
	void flipHorizontal();
	bool writeBitmap24(COLORREF *(*RGBMatrix), int height, int width);
	void setPixel24(int x, int y, Color &color);
	void setPixel24(int x, int y, int r, int g, int b);
	void createNullBitmap(int color);

private:
	//variables
	
	BITMAPINFO* bmi;


	int	paddingByte;										// number of Bytes to fill up the row to a multiple of four
	bool nullBitmap;
};


class GaussianBlur{

public:
	GaussianBlur(double sigma, int filterheight, int filterwidth);
	~GaussianBlur();

	void createKernel();
	void applyFilter(Bitmap *bitmap);

private:

	double m_sigma;
	int m_filterheight;
	int m_filterwidth;
	double **gaussianKernel;
};

#endif