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

class ViewportResizedSubsriberInterface
{
public:
	virtual void OnViewportResized(UShortSize2D newViewportSize) = 0;
};

class Viewport
{
private:
	HWND viewportWindowHandle;
	std::vector<ViewportResizedSubsriberInterface*> resizingSubscribers;
protected:
	RECT GetClientAreaRect(void);
public:
	Viewport(HWND viewportWindowHandle);
	void RefreshViewport(void);
	BitmapDimensions GetViewportDimensions(void);
	void SubscribeToViewportResizing(ViewportResizedSubsriberInterface& newSubscriber);
	void TriggerResizingEvent(UShortSize2D newViewportSize);
	UShortSize2D GetCurrentSize(void);
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
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPixelsToProcess;
	std::shared_ptr<FractalBitmapFactoryV2> fractalBitmapFactory;
	HWND viewWindowHandle;
	HDC viewBufferDC;
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
	IntVector2D operator/(const float& rightHand);
	IntVector2D operator*(const float& rightHand);
	bool operator==(const IntVector2D& rightHand);
};



class BitmapHandleProviderInterface
{
public:
	virtual bool GetBitmapHandle(HBITMAP& output) = 0;
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
	virtual void InitializeBitmap(UShortSize2D bitmapSize) = 0;
	virtual void DrawBitmapBuffer(void) = 0;
};

class AbstractBitmapToSizeGenerator: public BitmapSizeProviderInterface, public BitmapToSizeGeneratorInterface
{
private:
	UShortSize2D bitmapSize;
public:
	void SetBitmapSize(UShortSize2D newSize);
	virtual void InitializeBitmapWithCurrentSize() = 0;

	//BitmapSizeProviderInterface overrides
	virtual UShortSize2D GetBitmapSize(void) override;

	//BitmapToSizeGeneratorInterface
	virtual void InitializeBitmap(UShortSize2D bitmapSize) override;
};

//class FractalBitmap: 
//	public BitmapToSizeGeneratorInterface,
//	public BitmapSizeProviderInterface,
//	public BitmapHandleProviderInterface
//{
//public:
//
//};


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
	void ResetOffset(void);
	bool SetOffset(IntVector2D newOffset);
	IntVector2D GetOffset(void);
};

/*
Ta klasa powinna subskrybować event zmiany rozmiaru widoku.
Można do niej również podpiąć interfejs warstwy renderowania tak aby wyświetlała aktualną skalę.
*/
class ScalableBitmapInViewport: public PaintableInterface
{
private:
	float currentScaleRatio;
	BitmapMovableInViewport& currentMovableBitmap;
	BitmapToSizeGeneratorInterface& bitmapGenerator;
public:
	static const float minScaleRatio;
	static const float maxScaleRatio;
	ScalableBitmapInViewport(BitmapMovableInViewport& bitmap, BitmapToSizeGeneratorInterface& bitmapGenerator);
	bool Zoom(float delta, IntVector2D scalingReferencePoint);
	BitmapMovableInViewport& GetMovableBitmap(void);
	bool SetScaleRatio(float newScaleRatio);

	// Inherited via PaintableInterface
	virtual bool DrawInRepaintBuffer(HDC repaintBufferDC, RECT viewportRepaintRect) override;
};

class PaintingBufferLayerInterface
{
public:
	virtual void DrawInRepaintBuffer(HDC repaintBufferDC, PAINTSTRUCT& windowPaintingData) = 0;
};

class FractalBitmapInterface
{
public:
	virtual void Init(UShortSize2D bitmapSize) = 0;
	virtual void DrawPixel(BitmapPixel pixel, std::vector<unsigned int> pointsAtPixel) = 0;
	virtual bool GetFractalBitmapHandle(HBITMAP& output) = 0;
};

class AbstractFractalBitmap : public FractalBitmapInterface
{
private:
	Bitmap** fractalBitmap;
public:
	AbstractFractalBitmap(Bitmap** bitmap);
	bool GetFractalBitmapHandle(HBITMAP& output) override;
};

class MonochromaticFractalBitmap: public AbstractFractalBitmap
{
private:
	MonochromaticBitmap* monoBitmap;
public:
	MonochromaticFractalBitmap();
	// Inherited via FractalBitmapPixelsInterface
	virtual void Init(UShortSize2D bitmapSize) override;
	virtual void DrawPixel(BitmapPixel pixel, std::vector<unsigned int> pointsAtPixel) override;
};

class AbstractFractalDataProcessing
{
private:
	Fractal fractalDefinition;
	unsigned int maxNumberOfPointsToProcess;
	Point firstPoint;
};

class FractalBitmapDrawingInterface
{
public:
	virtual void DrawFractalPixels(unsigned int numberOfPointsToProcess) = 0;
};

class AbstractFractalProcessing: 
	public BitmapHandleProviderInterface,
	public AbstractBitmapToSizeGenerator
{
private:
	Fractal fractalDefinition;
	unsigned int maxNumberOfPoints;
	unsigned int numberOfPointsToDraw;	
protected:
	Fractal GetFractalDefinition(void);
	unsigned int GetMaxNumberOfPoints(void);
	unsigned int GetNumberOfPointsToDraw(void);
public:
	AbstractFractalProcessing();
	virtual ~AbstractFractalProcessing();
	virtual void CalculateFractalPoints(void) = 0;
	virtual void ConvertPointsToPixels(void) = 0;
	void SetFractalDefinition(Fractal fractal);
	void SetMaxNumberOfFractalPoints(unsigned int maxNumberOfPoints);
	void SetNumberOfPointsToRender(unsigned int numberOfPointsToDraw);	
};

class AbstractFratalProcessingWithBitmapInterface: public AbstractFractalProcessing
{
private:
	FractalBitmapInterface& fractalDrawing;
protected:
	FractalBitmapInterface& GetFractalDrawing(void);
public:
	AbstractFratalProcessingWithBitmapInterface(
		FractalBitmapInterface& fractalDrawing
	);
	virtual ~AbstractFratalProcessingWithBitmapInterface() {};
	virtual bool GetBitmapHandle(HBITMAP& output) override;
	virtual void InitializeBitmapWithCurrentSize(void) override;
};

class SimpleFractalProcessing : public AbstractFratalProcessingWithBitmapInterface
{
private:
	static const unsigned int numberOfPointsToCalculate = 100000;
	SynchronousFractalPointsCalculator* fractalPointsCalculator;
	std::vector<Point> fractalPoints;
	FractalPixelCalculatorGDI* fractalPixelCalculator;
	BitmapDimensions bitmapDimensions;
	std::vector<unsigned int>** pixelPoints;
	void InitializeFractalPointsCalculator(void);
	void InitializeFractalPixelCalculator(void);
	void InitializePixelPoints(void);
public:
	SimpleFractalProcessing(
		FractalBitmapInterface& fractalDrawing
	);
	virtual void CalculateFractalPoints(void) override;
	virtual void ConvertPointsToPixels(void) override;
	virtual void DrawBitmapBuffer(void) override;
};

class FractalFacade: public PaintingBufferLayerInterface, public FractalUIRenderingSubsriberInterface, public ViewportResizedSubsriberInterface
{
private:
	AbstractFractalProcessing& fractalProcessing;
	Viewport& viewport;
	BitmapInViewport fractalBitmapInViewport;
	BitmapMovableInViewport fractalMovableBitmap;
	ScalableBitmapInViewport scalableFractalBitmap;
public:
	FractalFacade(
		FractalDrawingUI& fractalUI,
		Viewport& viewport,
		AbstractFractalProcessing& fractalProcessing
	);

	// Inherited via PaintingBufferLayerInterface
	virtual void DrawInRepaintBuffer(HDC repaintBufferDC, PAINTSTRUCT& windowPaintingData) override;

	// Inherited via FractalUIRenderingSubsriberInterface
	virtual void RenderFractal(FractalRenderingData formData) override;

	// Inherited via ViewportResizedSubsriberInterface
	virtual void OnViewportResized(UShortSize2D newViewportSize) override;

	void MoveFractalImageInViewport(IntVector2D moveVector);
	void ZoomFractalBitmap(float zoomDelta, BitmapPixel dilationCenterInViewport);	
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