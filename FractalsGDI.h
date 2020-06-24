#pragma once

#include "Fractals.h"
#include <windows.h>
#include <ppl.h>
#include "gdi-wrapper.h"

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
	unsigned short bitmapHeight
);