#define _USE_MATH_DEFINES
#include <cmath>
#include <atlstr.h> 
#include <iostream>

#include "Bitmap.h"


Bitmap::Bitmap(){

	Bitmap::bmi = NULL;
	Bitmap::data = NULL;
	Bitmap::RGBMatrix = NULL;
}

Bitmap::Bitmap(int height, int width, int bpp){

	Bitmap::bmi = NULL;
	Bitmap::data = NULL;
	Bitmap::RGBMatrix = NULL;


	Bitmap::width = width;
	Bitmap::height = height;


	Bitmap::padWidth = bpp / 8 * Bitmap::width;
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

	

	if (nullBitmap){
		free(Bitmap::data);
		Bitmap::data = NULL;
	}
}


void Bitmap::flipHorizontal(){
	BYTE *pFront = 0;
	BYTE *pBack = 0;
	BYTE pixel[3] = { 0 };

	for (int i = 0; i < height; ++i){

		pFront = &data[i * padWidth];
		pBack = &pFront[padWidth - 3];

		while (pFront < pBack){

			// Save current pixel at position pFront.
			pixel[0] = pFront[0];
			pixel[1] = pFront[1];
			pixel[2] = pFront[2];


			// Copy new pixel from position pBack into pFront.
			pFront[0] = pBack[0];
			pFront[1] = pBack[1];
			pFront[2] = pBack[2];


			// Copy old pixel at position pFront into pBack.
			pBack[0] = pixel[0];
			pBack[1] = pixel[1];
			pBack[2] = pixel[2];


			pFront += 3;
			pBack -= 3;
		}
	}
}

void Bitmap::flipVertical(){

	std::vector<BYTE> srcPixels(padWidth * height);

	memcpy(&srcPixels[0], data, padWidth * height);

	BYTE *pSrcRow = 0;
	BYTE *pDestRow = 0;

	for (int i = 0; i < height; ++i)
	{
		pSrcRow = &srcPixels[(height - 1 - i) * padWidth];
		pDestRow = &data[i * padWidth];
		memcpy(pDestRow, pSrcRow, padWidth);
	}
}

// LoadBitmapFile
// desc: Returns a pointer to the bitmap image of the bitmap specified
//       by filename. Also returns the bitmap header information.
//		 No support for 8-bit bitmaps.
bool Bitmap::loadBitmap24(const char *filename){

	FILE 				*filePtr;				// the file pointer
	unsigned char		tempRGB;				// swap variable
	int					paddingByte;		  // number of Bytes to fill up the row to a multiple of four
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmih;

	// open filename in "read binary" mode
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return false;



	// read the bitmap file header
	fread(&(bmfh), sizeof(BITMAPFILEHEADER), 1, filePtr);

	// verify that this is a bitmap by checking for the universal bitmap id
	if (bmfh.bfType != 0x4D42){

		fclose(filePtr);
		return false;
	}

	// read the bitmap information header
	fread(&(bmih), sizeof(BITMAPINFOHEADER), 1, filePtr);

	Bitmap::bmi = (BITMAPINFO*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	Bitmap::width = bmih.biWidth;
	Bitmap::height = bmih.biHeight;

	padWidth = 3 * Bitmap::width;



	while (padWidth % 4 != 0) {
		padWidth++;
	}

	paddingByte = padWidth - 3 * Bitmap::width;

	// allocate enough memory for the bitmap image data
	Bitmap::data = (unsigned char*)malloc(bmih.biSizeImage);

	// verify memory allocation
	if (!Bitmap::data){

		free(Bitmap::data);
		fclose(filePtr);
		return false;
	}
	// create DIB Section
	bmi->bmiHeader = bmih;
	Bitmap::hbitmap = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (VOID**)&data, NULL, 0);

	// move file pointer to beginning of bitmap data and read in the bitmap image data
	fseek(filePtr, bmfh.bfOffBits, SEEK_SET);
	fread(Bitmap::data, 1, (bmih).biSizeImage, filePtr);
	// make sure bitmap image data was read
	if (Bitmap::data == NULL){
		fclose(filePtr);
		return false;
	}

	// swap the R and B values to get RGB since the bitmap color format is in BGR
	for (unsigned int i = 0; i<(bmih).biSizeImage; i += 3) {
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



	// fill the rgb matrix
	RGBMatrix = new COLORREF*[height];
	for (int y = 0; y < height; y++) {
		RGBMatrix[y] = new COLORREF[width];
		for (int x = 0; x < width; x++) {
			RGBMatrix[y][x] = 0;

		}
	}

	for (int i = 0, h = 0, m = 0; i < (bmih).biSizeImage - paddingByte; i += 3, m += 3) {
		//jump over the padding before the start of a new line
		if ((m + paddingByte) == padWidth) {
			i += paddingByte;
			h++;
			m = 0;
		}

		RGBMatrix[h][m / 3] = RGB(data[i + 2], data[i + 1], data[i]);

	}

	return true;
}


void Bitmap::createNullBitmap(int color){

	Bitmap::data = (unsigned char*)malloc(120000);

	for (unsigned int i = 0; i < 120000; i++) {

		Bitmap::data[i] = color;
	}

	Bitmap::width = 200;
	Bitmap::height = 200;
	Bitmap::padWidth = 600;
	nullBitmap = true;
}


bool Bitmap::writeBitmap24(COLORREF *(*RGBMatrix), int height, int width){

	Bitmap::width = width;
	Bitmap::height = height;



	Bitmap::padWidth = 3 * width;
	while (padWidth % 4 != 0) {
		padWidth++;
	}
	Bitmap::paddingByte = padWidth - 3 * width;


	if (bmi){

		free(bmi);
		bmi = NULL;
	}

	if (hbitmap){

		DeleteObject(hbitmap);
		hbitmap = NULL;
	}

	if (data && (hbitmap != NULL)){
		free(data);
		data = NULL;
	}


	//LPTR: Allocates fixed memory. The return value is a pointer to the memory object
	//		and also initializes memory contents to zero
	bmi = (BITMAPINFO*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 24;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = padWidth * height;
	bmi->bmiHeader.biClrImportant = 0;

	// allocate enough memory for the bitmap image data
	Bitmap::data = (unsigned char*)LocalAlloc(LPTR, bmi->bmiHeader.biSizeImage);


	// make sure bitmap image data was read
	if (Bitmap::data == NULL){

		return false;
	}

	Bitmap::hbitmap = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (VOID**)&data, NULL, 0);


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


void Bitmap::setPixel24(int x, int y, Color &color){
	data[y*padWidth + 3 * x] = color.b;
	data[y*padWidth + 3 * x + 1] = color.g;
	data[y*padWidth + 3 * x + 2] = color.r;
}

void Bitmap::setPixel24(int x, int y, int r, int g, int b){
	data[y*padWidth + 3 * x] = b;
	data[y*padWidth + 3 * x + 1] = g;
	data[y*padWidth + 3 * x + 2] = r;
}

///////////////////////////////////////////////////////////////////////////////////

GaussianBlur::GaussianBlur(double a_sigma, int a_filterheight, int a_filterwidth){

	m_sigma = a_sigma;
	m_filterheight = a_filterheight;
	m_filterwidth = a_filterwidth;
}

GaussianBlur::~GaussianBlur(){

	if (gaussianKernel){

		for (int i = 0; i < m_filterheight; i++) {
			delete[] gaussianKernel[i];
			gaussianKernel[i] = NULL;
		}

		delete[] gaussianKernel;
		gaussianKernel = NULL;

	}
}

void GaussianBlur::createKernel(){

	gaussianKernel = new double*[m_filterheight];

	for (int y = 0; y < m_filterheight; ++y){
		gaussianKernel[y] = new double[m_filterwidth];
	}

	double sum = 0.0;
	for (int i = 0; i < m_filterheight; i++) {
		for (int j = 0; j < m_filterwidth; j++) {
			gaussianKernel[i][j] = exp(-(i*i + j*j) / (2 * m_sigma * m_sigma)) / (2 * M_PI* m_sigma * m_sigma);
			sum += gaussianKernel[i][j];
		}
	}

	for (int i = 0; i < m_filterheight; i++) {
		for (int j = 0; j < m_filterwidth; j++) {
			gaussianKernel[i][j] /= sum;
		}
	}
}


void GaussianBlur::applyFilter(Bitmap *bitmap){


	int newImageHeight = bitmap->height - m_filterheight + 1;
	int newImageWidth = bitmap->width - m_filterwidth + 1;


	COLORREF** RGBMatrixSource;
	RGBMatrixSource = new COLORREF*[bitmap->height];
	for (int y = 0; y < bitmap->height; y++) {
		RGBMatrixSource[y] = new COLORREF[bitmap->width];
		for (int x = 0; x < bitmap->width; x++) {
			RGBMatrixSource[y][x] = bitmap->RGBMatrix[y][x];

		}
	}


	if (bitmap->RGBMatrix){

		for (int y = 0; y < bitmap->height; y++){

			delete[] bitmap->RGBMatrix[y]; // delete array within matrix
			bitmap->RGBMatrix[y] = NULL;
		}
		// delete actual matrix
		delete[] bitmap->RGBMatrix;
		bitmap->RGBMatrix = NULL;
	}

	bitmap->height = newImageHeight;
	bitmap->width = newImageWidth;




	bitmap->RGBMatrix = new COLORREF*[bitmap->height];
	for (int y = 0; y < bitmap->height; ++y){
		bitmap->RGBMatrix[y] = new COLORREF[bitmap->width];
	}

	double r, g, b;

	for (int i = 0; i < newImageHeight; i++) {
		for (int j = 0; j <newImageWidth; j++) {

			r = 0; g = 0; b = 0;


			for (int h = i; h < i + m_filterheight; h++) {
				for (int w = j; w < j + m_filterwidth; w++) {

					r = r + gaussianKernel[h - i][w - j] * GetRValue(RGBMatrixSource[h][w]);
					g = g + gaussianKernel[h - i][w - j] * GetGValue(RGBMatrixSource[h][w]);
					b = b + gaussianKernel[h - i][w - j] * GetBValue(RGBMatrixSource[h][w]);

				}
			}

			bitmap->RGBMatrix[i][j] = RGB((int)r, (int)g, (int)b);
		}
	}


	bitmap->writeBitmap24(bitmap->RGBMatrix, bitmap->height, bitmap->width);

}