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
	BITMAP* clientBitmap,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight,
	BYTE** bitmapBytesHandle
)
{
	std::chrono::steady_clock::time_point bitmapDrawingStart = std::chrono::high_resolution_clock::now();

	PixelCalculator kalkulatorPikseli(
		bitmapWidth,
		bitmapHeight,
		*clipping
	);

	BYTE* bitmapBytes = (BYTE*)clientBitmap->bmBits;
	
	COLORREF blackColor = (COLORREF)RGB(0, 0, 0);
	BYTE* initialBitmapBytes = *bitmapBytesHandle;
	unsigned short bitsInBitmapScanline = clientBitmap->bmWidthBytes * 8;
	concurrency::parallel_for(
		(unsigned int) 0,
		numberOfCalculatedPoints,
		(unsigned int) 1,
		[&](int i){
		BYTE* currentBitmapBytes = *bitmapBytesHandle;
		//jeżeli zainicjowano nową bitmapę to przestań rysoać piksele
		if (currentBitmapBytes == initialBitmapBytes)
		{
			Point currentPoint = *calculatedFractalPoints[i];
			unsigned short pixelX = kalkulatorPikseli.getPixelX(currentPoint.GetX());
			unsigned short pixelY = kalkulatorPikseli.getPixelY(currentPoint.GetY());
			markMonochromeBitmapPixelBlack(
				bitsInBitmapScanline,
				bitmapBytesHandle,
				pixelX,
				pixelY
			);
		}
	}
	);

	//wyświetl czas rysowania pikseli
	std::chrono::steady_clock::time_point bitmapDrawingEnd = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds bitmapDrawingTime = std::chrono::duration_cast<std::chrono::microseconds>(bitmapDrawingEnd - bitmapDrawingStart);
	std::wstringstream debugString;
	debugString << L"Czas rysowania pikseli bitmapy o rozmiarze " << bitmapWidth << L" na " << bitmapHeight << L" pikseli - " << bitmapDrawingTime.count() << L" qs\n";
	OutputDebugStringW(debugString.str().c_str());
	return true;
}

void MarkMononochromeBitmapAsText(
	BitmapPixel pixel,
	unsigned short bitsPerScanline,
	BYTE* pixelBytes
)
{
	unsigned int pixelBitIndex = (pixel.y * bitsPerScanline) + pixel.x;
	unsigned int byteIndex = pixelBitIndex / 8;
	unsigned char offsetInByte = (pixelBitIndex % 8);
	unsigned char moveToTheLeft = (7 - offsetInByte);
	BYTE pixelByteValue = ~(1 << moveToTheLeft); // ofset bitu w bajcie, dodano inwersję ponieważ fraktal musi przyjąć kolor tekstu czyli 0
	BYTE currentByteValue = pixelBytes[byteIndex];
	BYTE newByteValue = currentByteValue & pixelByteValue;
	pixelBytes[byteIndex] = newByteValue;
}

FractalBitmapFactory::FractalBitmapFactory(
	std::shared_ptr<FractalPixels> fractalPixelsCalculator
)
{
	this->fractalPixelsCalculator = fractalPixelsCalculator;
	this->bitmapData = {};
	this->bitmapData.bmPlanes = 1;
	this->bitmapData.bmBitsPixel = 1;
	this->bitmapData.bmHeight = fractalPixelsCalculator->getBitmapHeight();
	this->bitmapData.bmWidth = fractalPixelsCalculator->getBitmapWidth();
	this->bitmapData.bmWidthBytes = ceil(fractalPixelsCalculator->getBitmapWidth() / 16.0f) * 2;
	this->bitsPerScanline = this->bitmapData.bmWidthBytes * 8;
	this->noBytesRequired = this->bitmapData.bmWidthBytes * this->bitmapData.bmHeight;
	this->pixelBytes = new BYTE[this->noBytesRequired];
	memset(
		this->pixelBytes,
		255,
		this->noBytesRequired
	);
	this->bitmapData.bmBits = (void*)this->pixelBytes;
	this->numberOfDrawnPixels = 0;
	this->isDrawingBitmap = false;
	this->bitmapHandle = NULL;
	this->bitmapUpdated = false;
}

bool FractalBitmapFactory::generateBitmap(
	unsigned int numberOfPixelsToDraw,
	std::shared_ptr<bool> continueOperation,	
	void (*onBitmapUpdate)(FractalBitmapFactory*, unsigned int, void*),
	void* onBitmapUpdateData
)
{
	if (this->isDrawingBitmap)
	{
		return false;
	}
	this->isDrawingBitmap = true;
	unsigned int firstPointIndex = this->numberOfDrawnPixels;
	while (this->numberOfDrawnPixels < numberOfPixelsToDraw)
	{
		if (*continueOperation)
		{
			unsigned int numberOfCalculatedPixels = this->fractalPixelsCalculator->getNumberOfCalculatedPixels();
			if (firstPointIndex > numberOfCalculatedPixels)
			{
				concurrency::parallel_for(
					firstPointIndex,
					numberOfCalculatedPixels,
					(unsigned int)1,
					[&](unsigned int pointIndex)
				{
					BitmapPixel pixel = {};
					if (this->fractalPixelsCalculator->getPixelByPointIndex(pointIndex, pixel))
					{
						if (pixel.x < this->bitmapData.bmWidth && pixel.y < this->bitmapData.bmHeight)
						{
							MarkMononochromeBitmapAsText(
								pixel,
								this->bitsPerScanline,
								this->pixelBytes
							);
							this->bitmapUpdated = true;
							(*onBitmapUpdate)(this, this->numberOfDrawnPixels, onBitmapUpdateData);
							//należy zainkrementować po callbacku aby główna funkcja nie skończyła się przed jego wywołaniem
							this->numberOfDrawnPixels++;
						}
					}
				}
				);				
			}
		}
	}
	this->isDrawingBitmap = false;
	return true;
}

bool FractalBitmapFactory::copyIntoBuffer(HDC bitmapBuffer)
{
	if (this->bitmapUpdated)
	{
		this->bitmapUpdated = false;
		HBITMAP bitmapHandle = CreateBitmapIndirect(&this->bitmapData);
		HDC sourceDC = CreateCompatibleDC(bitmapBuffer);
		SelectObject(sourceDC, bitmapHandle);
		bool result = BitBlt(
			bitmapBuffer,
			0,
			0,
			this->bitmapData.bmWidth,
			this->bitmapData.bmHeight,
			sourceDC,
			0,
			0,
			SRCCOPY
		);
		DeleteDC(sourceDC);
		DeleteObject(bitmapHandle);
		return result;
	}	
	return false;
}

void FractalBitmapFactory::reset(void)
{
	if (this->bitmapHandle != NULL)
	{
		DeleteObject(this->bitmapHandle);
	}
	this->bitmapHandle = NULL;
	memset(
		this->pixelBytes,
		255,
		this->noBytesRequired
	);
	this->numberOfDrawnPixels = 0;
}
