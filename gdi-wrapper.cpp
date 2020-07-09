#include "gdi-wrapper.h"

const float WindowDrawing::maxScaleRatio = 6.0f;

WindowDrawing::WindowDrawing(HWND window, unsigned short width, unsigned short height)
{
	this->windowHandle = window;
	this->width = width;
	this->height = height;
	HDC windowDC = GetDC(window);
	windowClientCompatibleDC = CreateCompatibleDC(windowDC);
	bitmap = CreateCompatibleBitmap(windowDC, width, height);
	SelectObject(windowClientCompatibleDC, bitmap);
	RECT bitmapRect;
	bitmapRect.left = 0;
	bitmapRect.right = width;
	bitmapRect.top = 0;
	bitmapRect.bottom = height;
	HBRUSH whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	FillRect(
		windowClientCompatibleDC,
		&bitmapRect,
		whiteBrush
	);
	ReleaseDC(window, windowDC);
	this->offsetX = 0;
	this->offsetY = 0;
	this->scaleRatio = 1.0f;
}

WindowDrawing::WindowDrawing(const WindowDrawing& original)
{
	this->windowHandle = original.windowHandle;
	this->width = original.width;
	this->height = original.height;
	HDC windowDC = GetDC(original.windowHandle);
	windowClientCompatibleDC = CreateCompatibleDC(windowDC);
	bitmap = CreateCompatibleBitmap(windowDC, width, height);
	SelectObject(windowClientCompatibleDC, bitmap);
	BitBlt(
		windowClientCompatibleDC,
		0,
		0,
		width,
		height,
		original.windowClientCompatibleDC,
		0,
		0,
		SRCCOPY
	);
	ReleaseDC(this->windowHandle, windowDC);
	this->offsetX = original.offsetX;
	this->offsetY = original.offsetY;
	this->scaleRatio = original.scaleRatio;
}

WindowDrawing& WindowDrawing::operator=(const WindowDrawing& original)
{
	this->windowHandle = original.windowHandle;
	this->width = original.width;
	this->height = original.height;
	HDC windowDC = GetDC(original.windowHandle);
	windowClientCompatibleDC = CreateCompatibleDC(windowDC);
	bitmap = CreateCompatibleBitmap(windowDC, width, height);
	SelectObject(windowClientCompatibleDC, bitmap);
	BitBlt(
		windowClientCompatibleDC,
		0,
		0,
		width,
		height,
		original.windowClientCompatibleDC,
		0,
		0,
		SRCCOPY
	);
	ReleaseDC(this->windowHandle, windowDC);
	this->offsetX = original.offsetX;
	this->offsetY = original.offsetY;
	this->scaleRatio = original.scaleRatio;
	return *this;
}

WindowDrawing::~WindowDrawing()
{
	DeleteDC(windowClientCompatibleDC);
	DeleteObject(bitmap);
}

HDC WindowDrawing::getWindowDrawingBuffer(void)
{
	return windowClientCompatibleDC;
}

void WindowDrawing::redrawWindow(HDC wmPaintDC, PAINTSTRUCT& wmPaintPS)
{
	BOOL result = false;
	int repaintOffsetX = this->offsetX - wmPaintPS.rcPaint.left;
	int repaintOffsetY = this->offsetY - wmPaintPS.rcPaint.top;
	unsigned int repaintWidth = wmPaintPS.rcPaint.right - wmPaintPS.rcPaint.left;
	unsigned int repaintHeight = wmPaintPS.rcPaint.bottom - wmPaintPS.rcPaint.top;
	if (this->scaleRatio == 1.0f)
	{
		int sourceX = 0;
		int sourceY = 0;
		int copiedWidth = 0;
		int copiedHeight = 0;
		int destinationX = 0;
		int destinationY = 0;
		if (repaintOffsetX < 0)
		{
			//lewa krawędź obrazka wystaje za lewą krawędź obszaru rysowania
			copiedWidth = this->width + repaintOffsetX;
			if (copiedWidth <= 0)
			{
				//obrazek znajduje się w całości poza lewą krawędzią odrysowywanego obszaru
				OutputDebugStringW(L"bitmapa poza lewą krawędzią");
				return;
			}
			else if (copiedWidth > repaintWidth)
			{
				//ogranicz kopiwany obszar do obszaru rysowania
				copiedWidth = repaintWidth;
			}
			sourceX = -repaintOffsetX;
		}
		else
		{
			//lewa krawędź obrazka zaczyna się wewnątrz obszaru rysowania
			if (this->offsetX >= wmPaintPS.rcPaint.right)
			{
				//obrazek znajduje się w całości poza prawą krawędzia
				OutputDebugStringW(L"bitmapa poza prawą krawędzią");
				return;
			}
			destinationX = repaintOffsetX;
			copiedWidth = repaintWidth - repaintOffsetX;
		}
		if (repaintOffsetY < 0)
		{
			//górna krawędź obrazka wystaje poza obszar rysowania
			copiedHeight = this->height + repaintOffsetY;
			if (copiedHeight <= 0)
			{
				//obrazek znajduje się w całości poza górną krawędzią odrysowywanego obszaru
				OutputDebugStringW(L"bitmapa poza górną krawędzią");
				return;
			}
			else if (copiedHeight > repaintHeight)
			{
				//ogranicz kopiwany obszar do obszaru rysowania
				copiedHeight = repaintHeight;
			}
			sourceY = -repaintOffsetY;
		}
		else
		{
			//górna krawędź obrazka zaczyna się wewnątrz obszaru rysowania
			if (this->offsetY >= wmPaintPS.rcPaint.bottom)
			{
				//obrazek znajduje się w całości poza dolną krawędzia
				OutputDebugStringW(L"bitmapa poza dolną krawędzią");
				return;
			}
			destinationY = repaintOffsetY;
			copiedHeight = repaintHeight - repaintOffsetY;
		}
		result = BitBlt(
			wmPaintDC,
			destinationX,
			destinationY,
			copiedWidth,
			copiedHeight,
			windowClientCompatibleDC,
			sourceX,
			sourceY,
			SRCCOPY
		);
	}
	else
	{
		int sourceX = 0;
		int sourceY = 0;
		int destinationX = 0;
		int destinationY = 0;
		int destinationWidth = 0;
		int destinationHeight = 0;
		int scaledWidth = this->width * this->scaleRatio;
		int scaledHeight = this->height * this->scaleRatio;
		if (repaintOffsetX < 0)
		{
			sourceX = -repaintOffsetX / this->scaleRatio;
			destinationWidth = scaledWidth + repaintOffsetX;
			if (destinationWidth > repaintWidth)
			{
				destinationWidth = repaintWidth;
			}
		}
		else
		{
			destinationWidth = repaintWidth - repaintOffsetX;
			destinationX = repaintOffsetX;
		}
		int copiedWidth = destinationWidth / this->scaleRatio + 1;
		if (repaintOffsetY < 0)
		{
			sourceY = -repaintOffsetY / this->scaleRatio;
			destinationHeight = scaledHeight + repaintOffsetY;
			if (destinationHeight > repaintHeight)
			{
				destinationHeight = repaintHeight;
			}
		}
		else
		{
			destinationHeight = repaintHeight - repaintOffsetY;
			destinationY = repaintOffsetY;
		}
		int copiedHeight = destinationHeight / this->scaleRatio + 1;
		result = StretchBlt(
			wmPaintDC,
			destinationX,
			destinationY,
			destinationWidth,
			destinationHeight,
			windowClientCompatibleDC,
			sourceX,
			sourceY,
			copiedWidth,
			copiedHeight,
			SRCCOPY
		);
	}
	if (!result)
	{
		OutputDebugStringW(L"Nie udało się narysować bitmapy\n");
		debugLastError();
	}
}

void WindowDrawing::moveRender(short x, short y)
{
	this->offsetX += x;
	unsigned short maxOffsetX = this->width / 2;
	if (this->offsetX > maxOffsetX)
	{
		this->offsetX = maxOffsetX;
	}
	else
	{
		short minOffsetX = maxOffsetX - (this->width * this->scaleRatio);
		if (this->offsetX < minOffsetX)
		{
			this->offsetX = minOffsetX;
		}
	}
	this->offsetY += y;
	unsigned short maxOffsetY = this->height / 2;
	if (this->offsetY > maxOffsetY)
	{
		this->offsetY = maxOffsetY;
	}
	else
	{
		short minOffsetY = maxOffsetY - (this->height * this->scaleRatio);
		if (this->offsetY < minOffsetY)
		{
			this->offsetY = minOffsetY;
		}
	}
}

void WindowDrawing::resetOffset(void)
{
	this->offsetX = 0;
	this->offsetY = 0;
}

void WindowDrawing::scale(short promilePoints, unsigned short referencePointX, unsigned short referencePointY)
{
	float previousScaleRatio = this->scaleRatio;
	this->scaleRatio += ((float)(promilePoints) / 1000.0f);
	//nie oddalaj poniżej skali 1.0
	if (this->scaleRatio < 1.0f)
	{
		this->scaleRatio = 1.0f;
	}
	else if (this->scaleRatio > this->maxScaleRatio)
	{
		this->scaleRatio = this->maxScaleRatio;
	}
	const WCHAR debugMessageBeginning[] = L"skala bitmapy - ";
	WCHAR scaleRationDebugMessage[sizeof(debugMessageBeginning) + 4] = L"";
	wcscat_s(scaleRationDebugMessage, debugMessageBeginning);
	wcscat_s(scaleRationDebugMessage, floatToString(this->scaleRatio));
	OutputDebugStringW(scaleRationDebugMessage);

	short referenceToOffsetX = this->offsetX - referencePointX;
	short referenceToOffsetY = this->offsetY - referencePointY;
	float originalReferenceToOffsetX = (float)referenceToOffsetX / previousScaleRatio;
	float originalReferenceToOffsetY = (float)referenceToOffsetY / previousScaleRatio;
	short scaledVectorX = (short)(originalReferenceToOffsetX * this->scaleRatio);
	short scaledVectorY = (short)(originalReferenceToOffsetY * this->scaleRatio);
	this->offsetX = referencePointX + scaledVectorX;
	this->offsetY = referencePointY + scaledVectorY;
}

bool drawMovablePictureInRepaintBuffer(
	const HDC bufferDC,
	const RECT* repaintRect,
	const MovablePicture* picture
)
{
	int repaintOffsetX = picture->offsetX - repaintRect->left;
	int repaintOffsetY = picture->offsetY - repaintRect->top;
	unsigned int repaintWidth = repaintRect->right - repaintRect->left;
	unsigned int repaintHeight = repaintRect->bottom - repaintRect->top;

	int sourceX = 0;
	int sourceY = 0;
	int copiedWidth = 0;
	int copiedHeight = 0;
	int destinationX = 0;
	int destinationY = 0;
	if (repaintOffsetX < 0)
	{
		//lewa krawędź obrazka wystaje za lewą krawędź obszaru rysowania
		copiedWidth = picture->width + repaintOffsetX;
		if (copiedWidth <= 0)
		{
			//obrazek znajduje się w całości poza lewą krawędzią odrysowywanego obszaru
			OutputDebugStringW(L"bitmapa poza lewą krawędzią");
			return false;
		}
		else if (copiedWidth > repaintWidth)
		{
			//ogranicz kopiwany obszar do obszaru rysowania
			copiedWidth = repaintWidth;
		}
		sourceX = -repaintOffsetX;
	}
	else
	{
		//lewa krawędź obrazka zaczyna się wewnątrz obszaru rysowania
		if (picture->offsetX >= repaintRect->right)
		{
			//obrazek znajduje się w całości poza prawą krawędzia
			OutputDebugStringW(L"bitmapa poza prawą krawędzią");
			return false;
		}
		destinationX = repaintOffsetX;
		copiedWidth = repaintWidth - repaintOffsetX;
	}
	if (repaintOffsetY < 0)
	{
		//górna krawędź obrazka wystaje poza obszar rysowania
		copiedHeight = picture->height + repaintOffsetY;
		if (copiedHeight <= 0)
		{
			//obrazek znajduje się w całości poza górną krawędzią odrysowywanego obszaru
			OutputDebugStringW(L"bitmapa poza górną krawędzią");
			return false;
		}
		else if (copiedHeight > repaintHeight)
		{
			//ogranicz kopiwany obszar do obszaru rysowania
			copiedHeight = repaintHeight;
		}
		sourceY = -repaintOffsetY;
	}
	else
	{
		//górna krawędź obrazka zaczyna się wewnątrz obszaru rysowania
		if (picture->offsetY >= repaintRect->bottom)
		{
			//obrazek znajduje się w całości poza dolną krawędzia
			OutputDebugStringW(L"bitmapa poza dolną krawędzią");
			return false;
		}
		destinationY = repaintOffsetY;
		copiedHeight = repaintHeight - repaintOffsetY;
	}
	bool result = BitBlt(
		bufferDC,
		destinationX,
		destinationY,
		copiedWidth,
		copiedHeight,
		picture->deviceContext,
		sourceX,
		sourceY,
		SRCCOPY
	);
	return result;
}

void markMonochromeBitmapPixelBlack(
	unsigned short bitsInBitmapScanline,
	BYTE** bitmapBytesHandle,
	unsigned short pixelX,
	unsigned short pixelY
)
{
	BYTE* initialBitmapBytes = *bitmapBytesHandle;
	if (initialBitmapBytes == NULL)
	{
		return;
	}
	if (pixelX < 1 || pixelY < 1)
	{
		return;
	}
	unsigned int pixelBitIndex = ((pixelY - 1) * bitsInBitmapScanline) + (pixelX - 1);
	unsigned int byteIndex = pixelBitIndex / 8;
	unsigned char offsetInByte = (pixelBitIndex % 8);
	unsigned char moveToTheLeft = (7 - offsetInByte);
	BYTE pixelByteValue = ~(1 << moveToTheLeft); // ofset bitu w bajcie, dodano inwersję ponieważ fraktal musi przyjąć kolor tekstu czyli 0
	BYTE* bitmapBytes = *bitmapBytesHandle;
	BYTE currentByteValue = 0;
	if (*bitmapBytesHandle == initialBitmapBytes)
	{
		currentByteValue = bitmapBytes[byteIndex];
	}
	currentByteValue &= pixelByteValue;
	if (*bitmapBytesHandle == initialBitmapBytes)
	{
		bitmapBytes[byteIndex] = currentByteValue;
	}
}

void Bitmap::bitmapUpdated(void)
{
	this->updateHandle = true;
}

bool Bitmap::IsPixelValid(BitmapPixel pixel)
{
	return (pixel.x < this->bitmapData.bmWidth&& pixel.y < this->bitmapData.bmHeight);
}

bool Bitmap::IsPixelIndexValid(unsigned int pixelIndex)
{
	return pixelIndex < this->numberOfPixels;
}

Bitmap::Bitmap(unsigned short width, unsigned short height)
{
	this->bitmapData = {};
	this->bitmapData.bmWidth = width;
	this->bitmapData.bmHeight = height;
	this->updateHandle = false;
	this->bitmapHandle = NULL;
	this->numberOfPixels = width * height;
}

unsigned int Bitmap::GetPixelIndex(
	BitmapPixel pixel
)
{
	return pixel.y * this->bitmapData.bmWidth + pixel.x;
}

unsigned short Bitmap::GetWidth(void)
{
	return this->bitmapData.bmWidth;
}

unsigned short Bitmap::GetHeight(void)
{
	return this->bitmapData.bmHeight;
}

unsigned int Bitmap::GetNumberOfPixels(void)
{
	return this->numberOfPixels;
}

bool Bitmap::copyIntoBuffer(HDC bitmapBuffer, bool& handleWasUpdated)
{
	if (this->updateHandle)
	{
		DeleteObject(this->bitmapHandle);
		this->bitmapHandle = CreateBitmapIndirect(&this->bitmapData);
		this->updateHandle = false;
		handleWasUpdated = true;
	}
	else
	{
		handleWasUpdated = false;
	}
	if (this->bitmapHandle == NULL)
	{
		return false;
	}
	HDC sourceDC = CreateCompatibleDC(bitmapBuffer);
	SelectObject(sourceDC, this->bitmapHandle);
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
	if (!result)
	{
		DWORD error = GetLastError();
		std::wstringstream outputStream;
		outputStream << L"błąd kopiowania bitmapy do bufora - " << error << L"\n";
		OutputDebugStringW(outputStream.str().c_str());
	}
	DeleteDC(sourceDC);
	return result;
}

MonochromaticPixelData MonochromaticBitmap::GetPixelData(BitmapPixel pixel)
{
	unsigned int pixelBitIndex = (pixel.y * this->bitsPerScanline) + pixel.x;
	unsigned char offsetInByte = (pixelBitIndex % 8);
	unsigned char moveToTheLeft = (7 - offsetInByte);
	return MonochromaticPixelData{
		pixelBitIndex / 8,
		(BYTE)(1 << moveToTheLeft)
	};
}

MonochromaticBitmap::MonochromaticBitmap(unsigned short width, unsigned short height) : Bitmap(width, height)
{
	LONG bytesPerScanline = ceil(width / 16.0f) * 2;
	this->bitsPerScanline = bytesPerScanline * 8;
	this->noBytesRequired = bytesPerScanline * this->GetHeight();
	this->pixelBitClusters = new BYTE[this->noBytesRequired];
	this->bitmapData.bmBitsPixel = 1;
	this->bitmapData.bmPlanes = 1;
	this->bitmapData.bmWidthBytes = bytesPerScanline;
	this->bitmapData.bmBits = (void*)this->pixelBitClusters;
}

MonochromaticBitmap::~MonochromaticBitmap()
{
	delete[] this->pixelBitClusters;
}

void MonochromaticBitmap::MarkPixelAsText(MonochromaticPixelData pixelData)
{
	this->pixelBitClusters[pixelData.byteIndex] &= ~pixelData.byteBitPointer;//dodano inwersję ponieważ fraktal musi przyjąć kolor tekstu czyli 0
}

void MonochromaticBitmap::MarkPixelAsBackground(MonochromaticPixelData pixelData)
{
	this->pixelBitClusters[pixelData.byteIndex] |= pixelData.byteBitPointer;
}

void MonochromaticBitmap::Clear(void)
{
	memset(
		this->pixelBitClusters,
		255,
		this->noBytesRequired
	);
}
