#include "FractalsGDI.h"

FractalDrawing::FractalDrawing(unsigned short clientWidth, unsigned short clientHeight)
{
	this->clientWidth = clientWidth;
	this->clientHeight = clientHeight;
}

void FractalDrawing::drawFractal(Fractal fractal, HDC clientHdc)
{
	if (fractal.isValid())
	{
		PixelCalculator kalkulatorPikseli(
			this->clientWidth,
			this->clientHeight,
			fractal.getClipping()
		);
		Point currentPoint;
		Point pointPrim;
		HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		unsigned char noProbabilityRows = fractal.getNumberOfProbabilities() + 1;
		COLORREF* colors = new COLORREF[noProbabilityRows];
		for (unsigned char i = 0; i < noProbabilityRows; i++)
		{
			unsigned char rSeed = rand() % 256;
			unsigned char gSeed = rand() % 256;
			unsigned char bSeed = rand() % 256;
			colors[i] = (COLORREF) RGB(
				rSeed,
				gSeed,
				bSeed
			);
		}
		unsigned char* selectedRow = new unsigned char;
		*selectedRow = 0;
		for (int i = 0; i < 100000; i++)
		{
			RECT drawingPoint;
			drawingPoint.left = kalkulatorPikseli.getPixelX(currentPoint.GetX());
			drawingPoint.right = drawingPoint.left + 1;
			drawingPoint.top = kalkulatorPikseli.getPixelY(currentPoint.GetY());
			drawingPoint.bottom = drawingPoint.top + 1;
			pointPrim = fractal.getAffineTransformation(
				rand(),
				selectedRow
			).calculatePrim(currentPoint);
			currentPoint = pointPrim;
			SetPixel(
				clientHdc,
				kalkulatorPikseli.getPixelX(currentPoint.GetX()),
				kalkulatorPikseli.getPixelY(currentPoint.GetY()),
				colors[*selectedRow]
			);
		}
		RECT frameRect = {};
		frameRect.right = this->clientWidth;
		frameRect.bottom = this->clientHeight;
		FrameRect(clientHdc, &frameRect, blackBrush);
	}
}

unsigned short FractalDrawing::getClientWidth(void)
{
	return clientWidth;
}

unsigned short FractalDrawing::getClientHeight(void)
{
	return this->clientHeight;
}