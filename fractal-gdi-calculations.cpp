#include "fractal-gdi-calculations.h"

FractalPixelCalculatorGDI::FractalPixelCalculatorGDI() : pixelCalculator(), clipping(), bitmapSize()
{
}

FractalPixelCalculatorGDI::FractalPixelCalculatorGDI(
	FractalClipping clipping,
	BitmapDimensions bitmapSize
) : pixelCalculator(
	bitmapSize.GetWidth(),
	bitmapSize.GetHeight(),
	clipping
), bitmapSize(bitmapSize), clipping(clipping)
{
	this->clipping = clipping;
	this->bitmapSize = bitmapSize;
}

BitmapPixel FractalPixelCalculatorGDI::CalculatePixel(Point fractalPoint)
{
	return BitmapPixel{
		this->pixelCalculator.getPixelX(fractalPoint.GetX()),
		this->pixelCalculator.getPixelY(fractalPoint.GetY()),
	};
}

FractalClipping FractalPixelCalculatorGDI::GetClipping(void)
{
	return this->clipping;
}

BitmapDimensions FractalPixelCalculatorGDI::GetBitmapSize(void)
{
	return this->bitmapSize;
}
