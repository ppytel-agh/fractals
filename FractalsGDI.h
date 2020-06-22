#pragma once

#include "Fractals.h"
#include <windows.h>

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