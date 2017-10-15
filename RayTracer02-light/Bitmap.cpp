#include <atlstr.h> 
#include <iostream>

#include "Bitmap.h"


Bitmap::Bitmap()
{
	Bitmap::data = NULL;
	Bitmap::dataMatrix = NULL;
}

Bitmap::Bitmap(int height, int width, int bpp){

	Bitmap::data = NULL;
	Bitmap::dataMatrix = NULL;
	Bitmap::width = width;
	Bitmap::height = height;


	Bitmap::padWidth = bpp/8 * Bitmap::width;
	while (Bitmap::padWidth % 4 != 0) {
		Bitmap::padWidth++;
	}

	Bitmap::paddingByte = Bitmap::padWidth - bpp / 8 * Bitmap::width;

	//LPTR: Allocates fixed memory. The return value is a pointer to the memory object
	//		and also initializes memory contents to zero
	Bitmap::bmi = (BITMAPINFO*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = Bitmap::width;
	bmi->bmiHeader.biHeight = Bitmap::height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = bpp;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = padWidth * Bitmap::height;
	bmi->bmiHeader.biClrImportant = 0;

	// allocate enough memory for the bitmap image data
	Bitmap::data = (unsigned char*)LocalAlloc(LPTR, bmi->bmiHeader.biSizeImage);


	// make sure bitmap image data was allocated
	if (Bitmap::data == NULL)
	{
		throw "memory allocation failure";
		//return false;
	}

	/*
	If hSection is NULL, the system allocates memory for the DIB.
	The system closes the handle to that memory when you later delete the DIB
	by calling the DeleteObject function.
	If hSection is not NULL, you must close the hSection memory handle
	yourself after calling DeleteObject to delete the bitmap.
	*/

	Bitmap::hbitmap = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (VOID**)&data, NULL, 0);

}


Bitmap::~Bitmap()
{
	// in case hSecton is NULL delete the hbitmap will close the handle to the memory
	if (Bitmap::hbitmap)
	{
		DeleteObject(Bitmap::hbitmap);
		Bitmap::hbitmap = NULL;
	}

	if (Bitmap::data && (Bitmap::hbitmap != NULL)){
		free(Bitmap::data);
		Bitmap::data = NULL;
	}

	if (Bitmap::dataMatrix){
		for (int j = 0; j<Bitmap::height; j++)
		{
			free(Bitmap::dataMatrix[j]);
			Bitmap::dataMatrix[j] = NULL;
		}
		free(Bitmap::dataMatrix);
		Bitmap::dataMatrix = NULL;
	}
}


// LoadBitmapFile
// desc: Returns a pointer to the bitmap image of the bitmap specified
//       by filename. Also returns the bitmap header information.
//		 No support for 8-bit bitmaps.
bool Bitmap::loadBitmap24(char *filename)
{
	FILE 				*filePtr;				// the file pointer
	unsigned char		tempRGB;				// swap variable
	int					padWidth;				// widht of a padded row
	int					paddingByte;		  // number of Bytes to fill up the row to a multiple of four

	// open filename in "read binary" mode
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return false;

	// read the bitmap file header
	fread(&(Bitmap::bmfh), sizeof(BITMAPFILEHEADER), 1, filePtr);

	// verify that this is a bitmap by checking for the universal bitmap id
	if (Bitmap::bmfh.bfType != 0x4D42)
	{
		fclose(filePtr);
		return false;
	}

	// read the bitmap information header
	fread(&(Bitmap::bmih), sizeof(BITMAPINFOHEADER), 1, filePtr);
	Bitmap::width = Bitmap::bmih.biWidth;
	Bitmap::height = Bitmap::bmih.biHeight;

	padWidth = 3 * Bitmap::width;
	while (padWidth % 4 != 0) {
		padWidth++;
	}

	paddingByte = padWidth - 3 * Bitmap::width;

	// move file pointer to beginning of bitmap data
	fseek(filePtr, Bitmap::bmfh.bfOffBits, SEEK_SET);

	// allocate enough memory for the bitmap image data
	Bitmap::data = (unsigned char*)malloc(bmih.biSizeImage);

	// verify memory allocation
	if (!Bitmap::data)
	{
		free(Bitmap::data);
		fclose(filePtr);
		return false;
	}

	// read in the bitmap image data
	fread(Bitmap::data, 1, (Bitmap::bmih).biSizeImage, filePtr);

	// make sure bitmap image data was read
	if (Bitmap::data == NULL)
	{
		fclose(filePtr);
		return false;
	}

	// swap the R and B values to get RGB since the bitmap color format is in BGR
	//count backwards so you start at the front of the image
	for (int i = 0; i<(Bitmap::bmih).biSizeImage; i += 3) {
		//jump over the padding before the start of a new line
		if (((i + paddingByte) % padWidth == 0) && (i + paddingByte) != 0) {

			i += paddingByte;

		}
		//transfer the data
		tempRGB = Bitmap::data[i];
		Bitmap::data[i] = Bitmap::data[i + 2];
		Bitmap::data[i + 2] = tempRGB;
	}



	// close the file and return the bitmap image data
	fclose(filePtr);

	return true;
}



bool Bitmap::readMonochrome(char *filename)
{
	FILE *filePtr = fopen(filename, "rb");

	// BMP header is 54 bytes
	fread(&(Bitmap::bmfh), sizeof(BITMAPFILEHEADER), 1, filePtr);

	// read the bitmap information header
	fread(&(Bitmap::bmih), sizeof(BITMAPINFOHEADER), 1, filePtr);

	Bitmap::width = bmih.biWidth;
	Bitmap::height = bmih.biHeight;

	int w = Bitmap::bmih.biWidth;
	int h = Bitmap::bmih.biHeight;



	// lines are aligned on 4-byte boundary
	int lineSize = (w / 32) * 4;
	if (w % 32)lineSize += 4;
	int fileSize = lineSize * h;



	Bitmap::data = (unsigned char *)malloc(w * h);
	unsigned char *tempData = (unsigned char *)malloc(fileSize);

	// skip the header
	fseek(filePtr, 54, SEEK_SET);

	// skip palette - two rgb quads, 8 bytes
	fseek(filePtr, 8, SEEK_CUR);

	// read data
	fread(tempData, 1, fileSize, filePtr);

	// decode bits
	int i, j, k, rev_j;
	for (j = 0, rev_j = h - 1; j < h; j++, rev_j--) {
		for (i = 0; i < w / 8; i++) {
			int fpos = j * lineSize + i, pos = rev_j * w + i * 8;
			for (k = 0; k < 8; k++)
				Bitmap::data[pos + (7 - k)] = (tempData[fpos] >> k) & 1;

		}
	}

	free(tempData);

	return true;
}


bool Bitmap::loadBitmap24B(char *filename)
{
	FILE *filePtr;								// the file pointer
	int					padWidth;				// widht of a padded row
	int					paddingByte;			// number of Bytes to fill up the row to a multiple of four

	// open filename in "read binary" mode
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return false;

	// read the bitmap file header
	fread(&(Bitmap::bmfh), sizeof(BITMAPFILEHEADER), 1, filePtr);

	// verify that this is a bitmap by checking for the universal bitmap id
	if (Bitmap::bmfh.bfType != 0x4D42)
	{
		fclose(filePtr);
		return false;
	}

	// read the bitmap information header
	fread(&(Bitmap::bmih), sizeof(BITMAPINFOHEADER), 1, filePtr);
	Bitmap::width = Bitmap::bmih.biWidth;
	Bitmap::height = Bitmap::bmih.biHeight;


	padWidth = 3 * Bitmap::width;
	while (padWidth % 4 != 0) {
		padWidth++;
	}

	paddingByte = padWidth - 3 * Bitmap::width;

	// move file pointer to beginning of bitmap data
	fseek(filePtr, Bitmap::bmfh.bfOffBits, SEEK_SET);

	// allocate enough memory for the bitmap image data
	Bitmap::data = (unsigned char*)malloc(bmih.biSizeImage);

	// verify memory allocation
	if (!Bitmap::data)
	{
		free(Bitmap::data);
		fclose(filePtr);
		return false;
	}

	// read in the bitmap image data
	fread(Bitmap::data, 1, (Bitmap::bmih).biSizeImage, filePtr);

	// make sure bitmap image data was read
	if (Bitmap::data == NULL)
	{
		fclose(filePtr);
		return false;
	}


	// allocate enough memory for the B-color data
	Bitmap::dataMatrix = (unsigned char**)malloc(Bitmap::height* sizeof(unsigned char*));

	for (int j = 0; j<Bitmap::height; j++)
	{
		dataMatrix[j] = (unsigned char *)malloc(Bitmap::width* sizeof(unsigned char));
	}


	// storing the B-color data inside the dataMatrix
	for (int j = 0; j<Bitmap::height; j++) {

		for (int i = 0; i<Bitmap::width; i++) {

			dataMatrix[j][i] = data[j*padWidth + 3 * i];
		}
	}

	// close the file and return the bitmap image data
	fclose(filePtr);

	return true;
}


bool Bitmap::writeBitmap24(COLORREF *(*RGBMatrix), int height, int width){

	for (int i = 0, h = 0, m = 0; i < bmi->bmiHeader.biSizeImage - paddingByte; i += 3, m += 3) {
		//jump over the padding before the start of a new line
		if ((m + paddingByte) == padWidth) {
			i += paddingByte;
			h++;
			m = 0;
		}

		//B-value
		data[i] = GetBValue(RGBMatrix[h][m / 3]);
		//G-value
		data[i + 1] = GetGValue(RGBMatrix[h][m / 3]);
		//R-value
		data[i + 2] = GetRValue(RGBMatrix[h][m / 3]);
	}

	return true;
}

void Bitmap::setPixel24(int x, int y,const Color &rgbValue){
	data[y*padWidth + 3 * x] = rgbValue.b;
	data[y*padWidth + 3 * x + 1] = rgbValue.g;
	data[y*padWidth + 3 * x + 2] = rgbValue.r;
}

