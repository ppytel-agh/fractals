#pragma once

#include "Fractals.h"

Fractal getDragonFractal(void);

bool parseFractalFromPDF(wchar_t* textBuffer, Fractal** output);