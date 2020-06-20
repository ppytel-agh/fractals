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
		for (int i = 0; i < 100000; i++)
		{
			RECT drawingPoint;
			drawingPoint.left = kalkulatorPikseli.getPixelX(currentPoint.GetX());
			drawingPoint.right = drawingPoint.left + 1;
			drawingPoint.top = kalkulatorPikseli.getPixelY(currentPoint.GetY());
			drawingPoint.bottom = drawingPoint.top + 1;
			pointPrim = fractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
			currentPoint = pointPrim;
			FillRect(
				clientHdc,
				&drawingPoint,
				blackBrush
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