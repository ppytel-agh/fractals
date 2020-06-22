#include "FractalsGDI.h"
#include <chrono>
#include <sstream>

FractalDrawing::FractalDrawing(unsigned short clientWidth, unsigned short clientHeight)
{
	this->clientWidth = clientWidth;
	this->clientHeight = clientHeight;
}

void FractalDrawing::drawFractal(
	Fractal fractal,
	HDC clientHdc,
	float scale
)
{
	if (fractal.isValid())
	{
		unsigned short bitmapWidth = this->clientWidth * scale;
		unsigned short bitmapHeight = this->clientHeight * scale;
		PixelCalculator kalkulatorPikseli(
			bitmapWidth,
			bitmapHeight,
			fractal.getClipping()
		);

		HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		unsigned char noProbabilityRows = fractal.getNumberOfProbabilities() + 1;
		COLORREF* colors = new COLORREF[noProbabilityRows];
		for (unsigned char i = 0; i < noProbabilityRows; i++)
		{
			unsigned char rSeed = rand() % 256;
			unsigned char gSeed = rand() % 256;
			unsigned char bSeed = rand() % 256;
			colors[i] = (COLORREF)RGB(
				rSeed,
				gSeed,
				bSeed
			);
		}
		unsigned char* selectedRow = new unsigned char;
		*selectedRow = 0;
		Point currentPoint;
		unsigned char rSeed = rand() % 256;
		unsigned char gSeed = rand() % 256;
		unsigned char bSeed = rand() % 256;
		COLORREF pixelColor = (COLORREF)RGB(
			rSeed,
			gSeed,
			bSeed
		);
		for (int i = 0; i < 100000; i++)
		{
			SetPixel(
				clientHdc,
				kalkulatorPikseli.getPixelX(currentPoint.GetX()),
				kalkulatorPikseli.getPixelY(currentPoint.GetY()),
				pixelColor
			);
			currentPoint = fractal.getAffineTransformation(
				rand(),
				selectedRow
			).calculatePrim(currentPoint);
			pixelColor = colors[*selectedRow];
		}
		delete selectedRow;

		RECT frameRect = {};
		frameRect.right = bitmapWidth;
		frameRect.bottom = bitmapHeight;
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

bool drawFractal(Fractal fractal, HDC clientHdc, unsigned short bitmapWidth, unsigned short bitmapHeight)
{
	if (fractal.isValid())
	{
		PixelCalculator kalkulatorPikseli(
			bitmapWidth,
			bitmapHeight,
			fractal.getClipping()
		);

		HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		unsigned char noProbabilityRows = fractal.getNumberOfProbabilities() + 1;
		COLORREF* colors = new COLORREF[noProbabilityRows];
		for (unsigned char i = 0; i < noProbabilityRows; i++)
		{
			unsigned char rSeed = rand() % 256;
			unsigned char gSeed = rand() % 256;
			unsigned char bSeed = rand() % 256;
			colors[i] = (COLORREF)RGB(
				rSeed,
				gSeed,
				bSeed
			);
		}
		unsigned char* selectedRow = new unsigned char;
		*selectedRow = 0;
		Point currentPoint;
		unsigned char rSeed = rand() % 256;
		unsigned char gSeed = rand() % 256;
		unsigned char bSeed = rand() % 256;
		COLORREF pixelColor = (COLORREF)RGB(
			rSeed,
			gSeed,
			bSeed
		);
		for (int i = 0; i < 100000; i++)
		{
			SetPixel(
				clientHdc,
				kalkulatorPikseli.getPixelX(currentPoint.GetX()),
				kalkulatorPikseli.getPixelY(currentPoint.GetY()),
				pixelColor
			);
			currentPoint = fractal.getAffineTransformation(
				rand(),
				selectedRow
			).calculatePrim(currentPoint);
			pixelColor = colors[*selectedRow];
		}
		delete selectedRow;

		//dodaj ramkę
		RECT frameRect = {};
		frameRect.right = bitmapWidth;
		frameRect.bottom = bitmapHeight;
		FrameRect(clientHdc, &frameRect, blackBrush);
		return true;
	}
	return false;
}

bool drawFractalV2(
	const FractalClipping* clipping,
	Point** calculatedFractalPoints,
	unsigned int numberOfCalculatedPoints,
	HDC clientHdc,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
)
{
	std::chrono::steady_clock::time_point bitmapDrawingStart = std::chrono::high_resolution_clock::now();

	PixelCalculator kalkulatorPikseli(
		bitmapWidth,
		bitmapHeight,
		*clipping
	);

	COLORREF blackColor = (COLORREF)RGB(0, 0, 0);
	for (int i = 0; i < numberOfCalculatedPoints; i++)
	{
		Point currentPoint = *calculatedFractalPoints[i];
		SetPixel(
			clientHdc,
			kalkulatorPikseli.getPixelX(currentPoint.GetX()),
			kalkulatorPikseli.getPixelY(currentPoint.GetY()),
			blackColor
		);
	}

	//dodaj ramkę
	HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RECT frameRect = {};
	frameRect.right = bitmapWidth;
	frameRect.bottom = bitmapHeight;
	FrameRect(clientHdc, &frameRect, blackBrush);
	//wyświetl czas rysowania pikseli
	std::chrono::steady_clock::time_point bitmapDrawingEnd = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds bitmapDrawingTime = std::chrono::duration_cast<std::chrono::microseconds>(bitmapDrawingEnd - bitmapDrawingStart);
	std::wstringstream debugString;
	debugString << L"Czas rysowania pikseli bitmapy o rozmiarze " << bitmapWidth << L" na " << bitmapHeight << L" pikseli - " << bitmapDrawingTime.count() << L" qs\n";
	OutputDebugStringW(debugString.str().c_str());
	return true;
}
