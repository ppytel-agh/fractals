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
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProc(HWND, UINT, WPARAM, LPARAM);
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

struct FractalFormDialogData
{
	FractalDrawingUI* fractalUI;
	HWND importDialogWindowHandle;
	float drawingScale;
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

struct MonochromaticBitmapThreadDataV2
{
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPixelsToProcess;
	std::shared_ptr<FractalBitmapFactoryV2> fractalBitmapFactory;
};
DWORD WINAPI MonochromaticBitmapThreadV2(LPVOID);


struct FractalPixelsCalculatorThreadDataV2
{
	std::shared_ptr<bool> processThread;
	std::shared_ptr<FractalPixelsV2> fractalPixelsOutput;
	unsigned int numberOfPointsToProcess;
};
DWORD WINAPI FractalPixelsCalculatorThreadV2(LPVOID);

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