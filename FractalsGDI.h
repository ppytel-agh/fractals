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

void MarkMononochromeBitmapAsBackground(
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
	bool isDrawingBitmap;
	HBITMAP bitmapHandle;
	bool bitmapUpdated;
	std::atomic_uchar* pixelCount;
	unsigned int maxNumberOfPointsToProcess;
	unsigned int numberOfPixels;
	bool* pointsIncludedInBitmap;
public:
	FractalBitmapFactory(
		std::shared_ptr<FractalPixels> fractalPixelsCalculator,
		unsigned int maxNumberOfPointsToProcess
	);
	~FractalBitmapFactory();
	bool generateBitmap(
		unsigned int numberOfPixelsToDraw,
		std::shared_ptr<bool> continueOperation
	);
	bool copyIntoBuffer(HDC bitmapBuffer);
	void reset(void);
};

class FractalBitmapFactoryV2
{
private:
	std::shared_ptr<FractalPixels> fractalPixelsCalculator;
	bool isDrawingBitmap;
	std::atomic_uchar* pixelCount;
	unsigned int maxNumberOfPointsToProcess;
	bool* pointsIncludedInBitmap;
	MonochromaticBitmap bitmap;
public:
	FractalBitmapFactoryV2(
		std::shared_ptr<FractalPixels> fractalPixelsCalculator,
		unsigned int maxNumberOfPointsToProcess
	);
	~FractalBitmapFactoryV2();
	bool generateBitmap(
		unsigned int numberOfPixelsToDraw,
		std::shared_ptr<bool> continueOperation
	);
	bool copyIntoBuffer(HDC bitmapBuffer);
	void reset(void);
};

class FractalPixelCalculatorGDI
{
private:
	PixelCalculator pixelCalculator;
	FractalClipping clipping;
	BitmapDimensions bitmapSize;
public:
	FractalPixelCalculatorGDI(
		FractalClipping clipping,
		BitmapDimensions bitmapSize
	);
	BitmapPixel CalculatePixel(Point fractalPoint);
	FractalClipping GetClipping(void);
	BitmapDimensions GetBitmapSize(void);
};