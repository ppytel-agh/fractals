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