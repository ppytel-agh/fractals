#pragma once
#include "Fractals.h"
#include "gdi-wrapper.h"

class FractalPixelCalculatorGDI
{
private:
	PixelCalculator pixelCalculator;
	FractalClipping clipping;
	BitmapDimensions bitmapSize;
public:
	FractalPixelCalculatorGDI();
	FractalPixelCalculatorGDI(
		FractalClipping clipping,
		BitmapDimensions bitmapSize
	);
	FractalPixelCalculatorGDI(const FractalPixelCalculatorGDI& prototype);
	BitmapPixel CalculatePixel(Point fractalPoint);
	FractalClipping GetClipping(void);
	BitmapDimensions GetBitmapSize(void);
};