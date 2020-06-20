#pragma once

#include "Fractals.h"
#include <windef.h>

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
		HDC clientHdc
	);
	unsigned short getClientWidth(void);
	unsigned short getClientHeight(void);
};