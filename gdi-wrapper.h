#pragma once

#include <Windows.h>
#include "string-processing.h"
#include "debug-helper.h"

/*
	Klasa wrapująca bufor GDI rysowanego okna.
	Umożliwia rysowanie po buforze w tle.
	Parametry WM_PAINT należy przekazać do redrawWindow aby odrysować daną część okna.
	Kopiowanie obiektu tworzy niezależne obiekty GDI zawierające dotychczasową zawartość kopiowanej instancji.
*/
class WindowDrawing
{
private:
	HWND windowHandle;
	HDC windowClientCompatibleDC;
	HBITMAP bitmap;
	unsigned short width;
	unsigned short height;
	short offsetX;
	short offsetY;
	float scaleRatio;
	const static float maxScaleRatio;
public:
	WindowDrawing(
		HWND window,
		unsigned short width,
		unsigned short height
	);
	WindowDrawing(const WindowDrawing& original);
	WindowDrawing& operator= (const WindowDrawing& original);
	~WindowDrawing();
	HDC getWindowDrawingBuffer(void);
	void redrawWindow(HDC wmPaintDC, PAINTSTRUCT& wmPaintPS);
	void moveRender(short x, short y);
	void resetOffset(void);
	void scale(short promilePoints, unsigned short referencePointX, unsigned short referencePointY);
};

class WindowBuffer
{
private:
	HBITMAP bufferBitmap;
	HDC deviceContext;
public:
	WindowBuffer(
		HWND windowHandle
	);
};

struct MovablePicture
{
	HBITMAP bitmap;
	HDC deviceContext;
	unsigned short width;
	unsigned short height;
	short offsetX;
	short offsetY;
	float scale;
};

bool drawMovablePictureInRepaintBuffer(
	const HDC bufferDC,
	const RECT* repaintRect,
	const MovablePicture* picture
);

void markMonochromeBitmapPixelBlack(
	unsigned short bitsInBitmapScanline,
	BYTE** bitmapBytesHandle,
	unsigned short pixelX,
	unsigned short pixelY
);

struct BitmapPixel
{
	unsigned short x;
	unsigned short y;
};

struct UShortSize2D
{
	unsigned short width;
	unsigned short height;
};

class BitmapDimensions
{
private:
	unsigned short width;
	unsigned short height;
	unsigned int numberOfPixels;
public:
	BitmapDimensions();
	BitmapDimensions(
		unsigned short width,
		unsigned short height
	);
	BitmapDimensions(UShortSize2D size);
	BitmapDimensions(const BitmapDimensions& prototype);
	unsigned short GetWidth(void);
	unsigned short GetHeight(void);
	UShortSize2D GetSize(void);
	unsigned int GetNumberOfPixels(void);
	unsigned int GetPixelIndex(BitmapPixel pixel);
	BitmapPixel GetBitmapPixel(unsigned int pixelIndex);
	bool IsPixelValid(BitmapPixel pixel);
	bool IsPixelIndexValid(unsigned int pixelIndex);
};

class Bitmap
{
private:
	bool updateHandle;
	HBITMAP bitmapHandle;
	unsigned int numberOfPixels;
protected:
	BITMAP bitmapData;
public:
	Bitmap();
	Bitmap(
		unsigned short width,
		unsigned short height
	);
	unsigned short GetWidth(void);
	unsigned short GetHeight(void);
	unsigned int GetNumberOfPixels(void);
	bool IsPixelValid(BitmapPixel pixel);
	bool IsPixelIndexValid(unsigned int pixelIndex);
	unsigned int GetPixelIndex(BitmapPixel pixel);
	bool copyIntoBuffer(HDC bitmapBuffer, bool& handleWasUpdated);
	void bitmapUpdated(void);
};

struct MonochromaticPixelData
{
	unsigned int byteIndex;
	BYTE byteBitPointer;
};
class MonochromaticBitmap : public Bitmap
{
private:	
	unsigned short bitsPerScanline;
	unsigned int noBytesRequired;
	BYTE* pixelBitClusters;
public:
	MonochromaticBitmap();
	MonochromaticBitmap(
		unsigned short width,
		unsigned short height
	);
	~MonochromaticBitmap();
	MonochromaticPixelData GetPixelData(BitmapPixel pixel);
	void MarkPixelAsText(MonochromaticPixelData pixelData);
	void MarkPixelAsBackground(MonochromaticPixelData pixelData);
	void Clear(void);
};