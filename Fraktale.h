#pragma once

#include "resource.h"
#include "framework.h"
#include "Fractals.h"
#include <cstdlib>
#include <ctime>
#include "FractalsGUI.h"
#include "FractalsGDI.h"
#include "gdi-wrapper.h"
#include "fraktale-misc.h"
#include <chrono>
#include <vector>
#include <synchapi.h>
#include <memory>
#include < concurrent_vector.h>
#include "fractal-calculations.h"
#include "Synchapi.h"
#pragma comment(lib, "Synchronization.lib")

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance, WCHAR szWindowClass[]);
BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow, WCHAR szWindowClass[], WCHAR szTitle[], HWND& hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcV2(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProcV2(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ImportFromPdfProc(HWND, UINT, WPARAM, LPARAM);

//stałe
const unsigned int initialNumberOfPointsToRender = 100000;

//nowa funkcja wątku do kalkulacji punktów fraktala
struct FractalPointsThreadData
{
	std::shared_ptr<bool> processThread;
	Fractal fractal;
	unsigned int maxNumberOfPoints;
	std::shared_ptr<FractalPoints> fractalPointsOutput;
};
DWORD WINAPI FractalPointsThread(LPVOID);

struct MonochromaticBitmapThreadData
{
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPixelsToProcess;
	std::shared_ptr<FractalBitmapFactory> fractalBitmapFactory;
};
DWORD WINAPI MonochromaticBitmapThread(LPVOID);


struct FractalPixelsCalculatorThreadData
{
	std::shared_ptr<bool> processThread;
	std::shared_ptr<FractalPixels> fractalPixelsOutput;
};
DWORD WINAPI FractalPixelsCalculatorThread(LPVOID);

class Viewport
{
private:
	HWND viewportWindowHandle;
public:
	Viewport(HWND viewportWindowHandle);
	void RefreshViewport(void);
	BitmapDimensions GetViewportDimensions(void);
};

class PaintableInterface
{
public:
	virtual bool  DrawInRepaintBuffer(HDC repaintBufferDC, RECT viewportRepaintRect) = 0;
};




struct FractalFormDialogData
{
	FractalDrawingUI* fractalUI;
	HWND importDialogWindowHandle;
	float drawingScale;
};



struct MonochromaticBitmapThreadDataV2
{
	MonochromaticBitmapThreadDataV2(Viewport& viewport) : viewport(viewport) {};
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPixelsToProcess;
	std::shared_ptr<FractalBitmapFactoryV2> fractalBitmapFactory;
	HWND viewWindowHandle;
	HDC viewBufferDC;
	Viewport& viewport;
};
DWORD WINAPI MonochromaticBitmapThreadV2(LPVOID);


struct FractalPixelsCalculatorThreadDataV2
{
	std::shared_ptr<bool> processThread;
	std::shared_ptr<FractalPixelsV2> fractalPixelsOutput;
	unsigned int numberOfPointsToProcess;
	
};
DWORD WINAPI FractalPixelsCalculatorThreadV2(LPVOID);

class IntVector2D
{
private:
	short x;
	short y;
public:
	IntVector2D();
	IntVector2D(short x, short y);
	IntVector2D(const IntVector2D& prototype);
	short GetX(void);
	short GetY(void);
	IntVector2D operator+(const IntVector2D& rightHand);
	IntVector2D operator-(const IntVector2D& rightHand);
	void operator+=(const IntVector2D& rightHand);
	void operator-=(const IntVector2D& rightHand);
};



class BitmapHandleProviderInterface
{
public:
	virtual HBITMAP GetBitmapHandle(void) = 0;
};

class BitmapInViewport
{
private:
	Viewport& viewport;
	BitmapHandleProviderInterface& bitmapHandle;
public:
	BitmapInViewport(
		Viewport& viewport,
		BitmapHandleProviderInterface& bitmapHandle
	);
	Viewport& GetViewport(void);
	bool CopyIntoBuffer(
		HDC viewportBufferDC,
		int sourceX,
		int sourceY,
		int destinationX,
		int destinationY,
		int widthOfCopiedRect,
		int heightOfCopiedRect
	);
};

class RECTProcessor
{
private:
	RECT rect;
public:
	RECTProcessor(RECT rect);
	unsigned short GetWidth(void);
	unsigned short GetHeight(void);
	IntVector2D GetTopLeftVector(void);
	IntVector2D GetBottomRightVector(void);
	UShortSize2D GetSize(void);
};

class BitmapDimensionsInterface
{
public:
	virtual BitmapDimensions& GetBitmapDimensions(void) = 0;
};

class BitmapSizeProviderInterface
{
public: 
	virtual UShortSize2D GetBitmapSize(void) = 0;
};

class BitmapToSizeGeneratorInterface
{
public:
	virtual void GenerateBitmap(UShortSize2D bitmapSize) = 0;
};

class FractalBitmap: 
	public BitmapToSizeGeneratorInterface,
	public BitmapSizeProviderInterface,
	public BitmapHandleProviderInterface
{
public:

};


class BitmapMovableInViewport: public PaintableInterface
{
private:
	IntVector2D offset;
	BitmapInViewport& bitmapInViewport;
	BitmapSizeProviderInterface& bitmapSizeProvider;
public:
	BitmapMovableInViewport(BitmapInViewport& bitmapInViewport, BitmapSizeProviderInterface& bitmapSizeProvider);
	void MoveBitmap(IntVector2D delta);
	BitmapInViewport& GetBitmapInViewport(void);
	bool DrawInRepaintBuffer(
		HDC repaintBufferDC,
		RECT viewportRepaintRect
	) override;
};

class ScalableBitmapInViewport
{
private:
	float currentScaleRatio;
	BitmapMovableInViewport& currentMovableBitmap;
	BitmapToSizeGeneratorInterface& bitmapGenerator;
public:
	static const float minScaleRatio;
	static const float maxScaleRatio;
	ScalableBitmapInViewport(BitmapMovableInViewport& bitmap, BitmapToSizeGeneratorInterface& bitmapGenerator);
	bool Zoom(float delta, BitmapPixel scalingReferencePoint);
	BitmapMovableInViewport& GetMovableBitmap(void);
};

class PaintingBufferLayerInterface
{
public:
	virtual void DrawInRepaintBuffer(HDC repaintBufferDC, PAINTSTRUCT& windowPaintingData) = 0;
};

class FractalFacade: public PaintingBufferLayerInterface
{
private:
	FractalDrawingUI& fractalUI;
	BitmapMovableInViewport fractalMovableBitmap;
public:
	FractalFacade(FractalDrawingUI& fractalUI) :fractalUI(fractalUI) {};

	// Inherited via PaintingBufferLayerInterface
	virtual void DrawInRepaintBuffer(HDC repaintBufferDC, PAINTSTRUCT& windowPaintingData) override;
	void Render(void);
	void MoveFractalImageInViewport(IntVector2D moveVector);
	void ZoomFractalBitmap(float zoomDelta);
};

/*
	Klasa do przetwarzania WM_PAINT.
	Należy utworzyć obiekt na stosie i wywołać rysowanie odpowiednich warstw.
	Zdjęcie obiektu ze stosu skutkuje przerysowaniem bufora do okna.
*/
class WindowPaintingPipeline
{
private:
	HWND windowHandle;
	PAINTSTRUCT paintStruct;
	HDC paintingDC;
	HDC bufferDC;
	HBITMAP paintingBufferBitmap;
public:
	WindowPaintingPipeline(HWND windowHandle);
	~WindowPaintingPipeline();
	void DrawLayer(PaintingBufferLayerInterface& paintingLayer);
};

class WindowManualResizing
{
private:
	Viewport& viewport;
	bool operationInProgress;
	UShortSize2D sizeAtBeginningOfResizing;
	UShortSize2D sizeAtEndOfResizing;
public:
	WindowManualResizing(Viewport& viewport);
	bool IsWindowResizedManually(void);
	void BeginManualResizing(void);
	void EndManualResizing(void);
	bool WindowSizeChangedDuringResizing(void);
	UShortSize2D GetNewSize(void);
};

class VectorTracking2D
{
private:
	IntVector2D lastPosition;
public:
	void UpdateLastPosition(IntVector2D position);
	IntVector2D GetDelta(IntVector2D currentPosition);
};

struct FractalWindowData
{
	FractalWindowData(Viewport& viewport, FractalFacade& fractalFacade, WindowManualResizing resizing, VectorTracking2D LMBPressedTracking)
		: viewport(viewport), fractalFacade(fractalFacade), resizing(resizing), LMBPressedTracking(LMBPressedTracking) {
		this->isLMBPressed = false;
	};
	HWND windowHandle;
	HWND dialogWindowHandle;
	bool isResizedManually;
	bool isMinimized;
	unsigned short previousWidth;
	unsigned short previousHeight;
	bool isFractalImageMoved;
	POINT* lastPointerPosition;
	Fractal* fractal;
	/*
	Przy wywołaniu UpdateFractalBitmap przez chwilę mogą istnieć dwa wątki
	zwracające uchwyt do tego samego pola w pamięci co może skutkować lekkim gliczowaniem obrazu.
	Potencjalne rozwiązanie to reset wskaźnika do uchwytu bitmapy w tej funkcji.
	*/
	MovablePicture* fractalImage;
	std::chrono::steady_clock::time_point lastPainingTS;
	DWORD calculateFractalPointsThreadId;
	DWORD createFractalBitmapThreadId;
	DWORD calculateFractalPixelsThreadId;
	std::shared_ptr<bool> processFractalPointsThread;
	std::shared_ptr<bool> processFractalPixelsThread;
	std::shared_ptr<bool> processFractalBitmapThread;
	std::shared_ptr<FractalPoints> currentFractalPoints;
	unsigned int numberOfPointsToProcess;
	float updatedScale;
	short updateOffsetX;
	short updateOffsetY;
	std::shared_ptr<FractalBitmapFactory> currentFractalBitmapGenerator;
	std::shared_ptr<FractalPixels> fractalPixelsCalculator;
	std::shared_ptr<FractalBitmapFactoryV2> currentFractalBitmapGeneratorV2;
	std::shared_ptr<FractalPixelsV2> fractalPixelsCalculatorV2;
	//nowe klasy
	Viewport& viewport;
	FractalFacade& fractalFacade;
	WindowManualResizing& resizing;
	bool isLMBPressed;
	VectorTracking2D& LMBPressedTracking;
};

struct FractalWindowDataV2
{
	FractalWindowDataV2(Viewport& viewport, FractalFacade& fractalFacade, WindowManualResizing resizing, VectorTracking2D LMBPressedTracking)
		: viewport(viewport), fractalFacade(fractalFacade), resizing(resizing), LMBPressedTracking(LMBPressedTracking) {
		this->isLMBPressed = false;
		this->isMinimized = false;
	};
	Viewport& viewport;
	FractalFacade& fractalFacade;
	bool isMinimized;
	WindowManualResizing& resizing;
	bool isLMBPressed;
	VectorTracking2D& LMBPressedTracking;
};

void UpdateFractalBitmap(
	FractalWindowData* windowData,
	unsigned short newWidth,
	unsigned short newHeight,
	short newOffsetX,
	short newOffsetY,
	float newScale
);

class MessageProcessor
{
private:
	HACCEL hAccelTable;
	HWND dialogHandle; //formDialogHandle
	FractalFormDialogData* fractalDialogData;
public:
	MessageProcessor(
		HACCEL hAccelTable,
		HWND dialogHandle,
		FractalFormDialogData* fractalDialogData
	);
	void ProcessMessage(MSG msg);
};

class RealTimeMessageLoop
{
private:
	MessageProcessor msgProcessor;
	FractalWindowData* fractalWindowData;
	HWND mainWindowHandle;
	std::chrono::steady_clock::time_point lastPaintingTS;
	unsigned char framecapMS;
public:
	RealTimeMessageLoop(
		MessageProcessor msgProcessor,
		FractalWindowData* fractalWindowData,
		HWND mainWindowHandle
	);
	int messageLoop(void);
};

void InitializeFractalPointsCalculator(
	FractalWindowData* fractalWindowData
);
void InitializeFractalPointsThread(
	FractalWindowData* fractalWindowData
);

void InitializeFractalPixelsCalculator(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);
void InitializeFractalPixelsThread(
	FractalWindowData* fractalWindowData
);

void InitializeFractalBitmapGenerator(
	FractalWindowData* fractalWindowData
);
void InitializeFractalBitmapThread(
	FractalWindowData* fractalWindowData,
	unsigned int numberOfPointsToRender
);

void InitializeFractalRender(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);

void InitializeFractalPixelsCalculatorV2(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);
void InitializeFractalPixelsThreadV2(
	FractalWindowData* fractalWindowData,
	unsigned int numberOfPointsToRender
);

void InitializeFractalBitmapGeneratorV2(
	FractalWindowData* fractalWindowData
);
void InitializeFractalBitmapThreadV2(
	FractalWindowData* fractalWindowData,
	unsigned int numberOfPointsToRender
);

void InitializeFractalRenderV1(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);

void InitializeFractalRenderV2(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
);