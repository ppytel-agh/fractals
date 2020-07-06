#pragma once

#include "Fractals.h"
#include <windows.h>
#include <ppl.h>
#include "gdi-wrapper.h"
#include "fractal-calculations.h"
#include "ppl.h"

class FractalDrawing
{
private:
	unsigned short clientWidth;
	unsigned short clientHeight;
public:
	FractalDrawing(
		unsigned short clientWidth,
		unsigned short clientHeight
	);
	void drawFractal(
		Fractal fractal,
		HDC clientHdc,
		float scale = 1.0f
	);
	unsigned short getClientWidth(void);
	unsigned short getClientHeight(void);
};

bool drawFractal(
	Fractal fractal,
	HDC clientHdc,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);

bool drawFractalV2(
	const FractalClipping* clipping,
	Point** calculatedFractalPoints,
	unsigned int numberOfCalculatedPoints,
	BITMAP* clientBitmap,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight,
	BYTE** bitmapBytesHandle
);

void MarkMononochromeBitmapAsText(
	BitmapPixel pixel,
	unsigned short bitsPerScanline,
	BYTE* pixelBytes
);

class FractalBitmapFactory
{
private:
	std::shared_ptr<FractalPixels> fractalPixelsCalculator;
	BITMAP bitmapData;
	unsigned short bitsPerScanline;
	unsigned int noBytesRequired;
	BYTE* pixelBytes;
	unsigned int numberOfDrawnPixels;
	bool isDrawingBitmap;
	HBITMAP bitmapHandle;
	bool bitmapUpdated;
public:
	FractalBitmapFactory(
		std::shared_ptr<FractalPixels> fractalPixelsCalculator
	);
	bool generateBitmap(
		unsigned int numberOfPixelsToDraw,
		std::shared_ptr<bool> continueOperation
	);
	bool copyIntoBuffer(HDC bitmapBuffer);
	void reset(void);
};