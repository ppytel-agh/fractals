// Fraktale.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Fraktale.h"
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
#pragma comment(lib, "Synchronization.lib")

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance, WCHAR szWindowClass[]);
BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow, WCHAR szWindowClass[], WCHAR szTitle[], HWND& hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ImportFromPdfProc(HWND, UINT, WPARAM, LPARAM);

//kalkulowanie punktów dla nowego fraktala
DWORD WINAPI CalculateFractalPointsThread(LPVOID);

//inicjalizacja nowej bitmapy i wypełnienie jej punktami, które zostały już skalkulowane
DWORD WINAPI CalculateFractalBitmapThread(LPVOID);

//komunikat kalkulacji piksela
const UINT WM_CALCULATE_POINT_PIXEL = WM_APP + 1;
//nasłuchiwanie na pojawienie się nowych punktów dla bitmapy
DWORD WINAPI DrawFractalBitmapPointsRT(LPVOID);

//nowa funkcja wątku do kalkulacji punktów fraktala
struct FractalPointsThreadData
{
	std::shared_ptr<bool> processThread;
	Fractal fractal;
	unsigned int maxNumberOfPoints;
	std::shared_ptr<concurrency::concurrent_vector<Point>> fractalPointsOutput;
	std::shared_ptr<bool> isLastPointOutput;
};
DWORD WINAPI FractalPointsThread(LPVOID);

struct BitmapPixel
{
	unsigned short x;
	unsigned short y;
};
struct MonochromaticBitmapThreadData
{
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPixelsToProcess;
	unsigned short width;
	unsigned short height;
	DWORD notifyAboutBitmapUpdateThread;
	HWND bitmapWindowHandle;
	std::shared_ptr<concurrency::concurrent_vector<BitmapPixel>> bitmapPixelsInput;
	unsigned short newOffsetX;
	unsigned short newOffsetY;
	unsigned short newScale;
	MovablePicture* outputPicture;
};
DWORD WINAPI MonochromaticBitmapThread(LPVOID);

void MarkMononochromeBitmapAsText(
	BitmapPixel pixel,
	unsigned short bitsPerScanline,
	BYTE* pixelBytes
);

struct FractalPixelsCalculatorThreadData
{
	std::shared_ptr<bool> processThread;
	unsigned int numberOfPointsToProcess;
	unsigned short bitmapWidth;
	unsigned short bitmapHeight;
	FractalClipping clipping;
	std::shared_ptr<concurrency::concurrent_vector<Point>> fractalPointsInput;
	std::shared_ptr<concurrency::concurrent_vector<BitmapPixel>> bitmapPixelsOutput;
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
	MovablePicture* fractalImage;
	Point** calculatedFractalPoints;
	unsigned int numberOfCalculatedPoints;
	BITMAP* fractalBitmap;
	std::chrono::steady_clock::time_point lastPainingTS;
	PixelCalculator* pixelCalculator;
	unsigned short fractalBitmapBitsPerScanline;
	BYTE* fractalBitmapBytes;
	DWORD calculateFractalPointsThreadId;
	DWORD createFractalBitmapThreadId;
	DWORD calculateFractalPixelsThreadId;
	BYTE** bitmapBytesHandle;
	Point*** fractalPointsHandle;
	std::shared_ptr<bool> processFractalPointsThread;
	std::shared_ptr<bool> processFractalPixelsThread;
	std::shared_ptr<bool> processFractalBitmapThread;
	std::shared_ptr<concurrency::concurrent_vector<Point>> currentFractalPoints;
	unsigned int numberOfPointsToProcess;
	float updatedScale;
};

struct FractalFormDialogData
{
	FractalDrawingUI* fractalUI;
	HWND importDialogWindowHandle;
	float drawingScale;
};

void RefreshFractalBitmap(FractalWindowData* windowData);
void RefreshViewport(FractalWindowData* windowData);

void UpdateFractalBitmap(
	FractalWindowData* windowData,
	unsigned short newWidth,
	unsigned short newHeight,
	short newOffsetX,
	short newOffsetY,
	float newScale
);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//zainicjuj generator liczb
	srand(time(NULL));

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	const unsigned char maxLoadString = 100;
	WCHAR szTitle[maxLoadString];
	WCHAR szWindowClass[maxLoadString];
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, maxLoadString);
	LoadStringW(hInstance, IDC_FRAKTALE, szWindowClass, maxLoadString);
	MyRegisterClass(hInstance, szWindowClass);

	// Perform application initialization:
	HWND mainWindowHandle;
	if (!InitInstance(hInstance, nCmdShow, szWindowClass, szTitle, mainWindowHandle))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FRAKTALE));

	MSG msg;
	FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLong(mainWindowHandle, GWL_USERDATA);
	HWND dialogHandle = fractalWindowData->dialogWindowHandle;
	FractalFormDialogData* fractalDialogData = (FractalFormDialogData*)GetWindowLong(dialogHandle, GWL_USERDATA);


	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		bool isTranslated = TranslateAccelerator(msg.hwnd, hAccelTable, &msg);
		bool isDialog = IsDialogMessage(dialogHandle, &msg);
		if (fractalDialogData->importDialogWindowHandle != NULL)
		{
			isDialog = IsDialogMessage(fractalDialogData->importDialogWindowHandle, &msg);
		}
		if (!isTranslated && !isDialog)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance, WCHAR szWindowClass[])
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FRAKTALE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FRAKTALE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, WCHAR szWindowClass[], WCHAR szTitle[], HWND& hWnd)
{
	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			{
				//utwórz dialog box z formulrzem
				HWND dialogHandle = CreateDialog(
					(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
					MAKEINTRESOURCE(IDD_FRAKTALE_DIALOG),
					hWnd,
					(DLGPROC)FractalFormDialogProc
				);
				ShowWindow(dialogHandle, SW_SHOW);
				//zainicjuj obiekt do rysowania fraktala na bazie rozdzielczości obszaru okna
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(dialogHandle, GWL_USERDATA);
				CREATESTRUCTW* createData = (CREATESTRUCTW*)lParam;
				//dane okna
				FractalWindowData* windowData = new FractalWindowData{};
				windowData->windowHandle = hWnd;
				windowData->dialogWindowHandle = dialogHandle;
				windowData->fractalImage = new MovablePicture{};
				windowData->bitmapBytesHandle = new BYTE*;
				*windowData->bitmapBytesHandle = NULL;
				windowData->fractalPointsHandle = new Point**;
				*windowData->fractalPointsHandle = NULL;
				SetWindowLongW(
					hWnd,
					GWL_USERDATA,
					(LONG)windowData
				);
			}
			break;
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
					case IDM_ABOUT:
						DialogBox(
							(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
							MAKEINTRESOURCE(IDD_ABOUTBOX),
							hWnd,
							About
						);
						break;
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...	
				/*OutputDebugStringW(L"Obszar odrysowania: ");
				debugRectangle(&ps.rcPaint);
				OutputDebugStringW(L"\n");*/

				//bufor obrazu zapobiegający miganiu				
				HDC screenBuffer = CreateCompatibleDC(hdc);
				//bufor ma rozmiar obszaru do odmalowania
				int bufferBitmapWidth = ps.rcPaint.right - ps.rcPaint.left;
				int bufferBitmapHeight = ps.rcPaint.bottom - ps.rcPaint.top;
				HBITMAP screenCompatibleBitmap = CreateCompatibleBitmap(hdc, bufferBitmapWidth, bufferBitmapHeight);
				SelectObject(screenBuffer, screenCompatibleBitmap);
				//wypełnij bufor kolorem tła
				HBRUSH backgroundBrush = (HBRUSH)GetClassLongW(
					hWnd,
					GCL_HBRBACKGROUND
				);
				LOGBRUSH bgBrushData;
				GetObjectW(
					backgroundBrush,
					sizeof(LOGBRUSH),
					&bgBrushData
				);
				RECT backgroundArea = {};
				backgroundArea.right = bufferBitmapWidth;
				backgroundArea.bottom = bufferBitmapHeight;
				FillRect(
					screenBuffer,
					&backgroundArea,
					backgroundBrush
				);

				//przekaż bufor do obiektu, który odmaluje w nim fragment bitmapy fraktala

				if (windowData->fractalImage->bitmap != NULL)
				{
					drawMovablePictureInRepaintBuffer(
						screenBuffer,
						&ps.rcPaint,
						windowData->fractalImage
					);
				}


				//skopiuj bufor na ekran
				BitBlt(
					hdc,
					ps.rcPaint.left,
					ps.rcPaint.top,
					bufferBitmapWidth,
					bufferBitmapHeight,
					screenBuffer,
					0,
					0,
					SRCCOPY
				);
				EndPaint(hWnd, &ps);
				windowData->lastPainingTS = std::chrono::high_resolution_clock::now();
			}
			break;
		case WM_DESTROY:
			{

				FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				PostQuitMessage(0);
				break;
			}
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE || wParam == VK_RETURN)
			{
				return 0;
			}
			break;
		case WM_ENTERSIZEMOVE:
			//przechwytujemy początek zmiany rozmiaru okna, aby ustawić flagę "w trakcie zmiany rozmiaru"
			{
				OutputDebugStringW(L"Początek zmiany rozmiaru\n");
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				windowData->isResizedManually = true;
				RECT currentSize;
				GetClientRect(hWnd, &currentSize);
				windowData->previousWidth = currentSize.right;
				windowData->previousHeight = currentSize.bottom;
			}
			break;
		case WM_EXITSIZEMOVE:
			//Jeżeli rozmiar okna jest inny niż przedtem, to zaktualizuj bitmapę fraktala pod nowy wymiar
			{
				OutputDebugStringW(L"Koniec zmiany rozmiaru\n");
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				windowData->isResizedManually = false;
				RECT newSize;
				GetClientRect(hWnd, &newSize);
				if (
					(windowData->previousWidth != newSize.right)
					||
					(windowData->previousHeight != newSize.bottom)
					)
				{
					OutputDebugStringW(L"Rozmiar okna się zmienił\n");
					//zmień skalę tak, aby pasowała do aktualnego przybliżenia
					if (
						(windowData->fractalImage->width < newSize.right)
						||
						(windowData->fractalImage->height < newSize.bottom)
						)
					{
						OutputDebugStringW(L"Reset skali bitmapy\n");
						UpdateFractalBitmap(
							windowData,
							newSize.right,
							newSize.bottom,
							0,
							0,
							1.0f
						);
					}
					else
					{
						windowData->fractalImage->scale = windowData->fractalImage->width / newSize.right;
					}
				}
			}
			break;
		case WM_SIZE:
			//maksymalizowanie i przywracanie rozmiaru okna powinny skutkować odrysowaniem bitmapy fraktala
			{
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				bool resizeNow = false;
				if (wParam == SIZE_MAXIMIZED)
				{
					OutputDebugStringW(L"Zmaksymalizowano okno\n");
					resizeNow = true;
				}
				else if (wParam == SIZE_RESTORED)
				{
					if (windowData->isResizedManually)
					{
						OutputDebugStringW(L"Ręczna zmiana rozmiaru\n");
					}
					else if (windowData->isMinimized)
					{
						OutputDebugStringW(L"Odminimalizowano okno\n");
						windowData->isMinimized = false;
					}
					else
					{
						OutputDebugStringW(L"Przywrócono rozmiar okna\n");
						resizeNow = true;
					}
				}
				else if (wParam == SIZE_MINIMIZED)
				{
					windowData->isMinimized = true;
				}
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				if (resizeNow)
				{
					
					if (
						(windowData->fractalImage->width < clientRect.right)
						||
						(windowData->fractalImage->height < clientRect.bottom)
						)
					{
						OutputDebugStringW(L"Reset skali bitmapy\n");
						UpdateFractalBitmap(
							windowData,
							clientRect.right,
							clientRect.bottom,
							0,
							0,
							1.0f
						);
					}
					else
					{
						windowData->fractalImage->scale = windowData->fractalImage->width / clientRect.right;
					}
				}
			}
			break;
		case WM_ERASEBKGND:
			//nie czyść ekranu bez potrzeby, żeby uniknąć migotania
			break;
		case WM_MOUSEMOVE:
			//jeżeli użytkownik przesuwa myszą z wciśniętym przyciskiem to należy przesuwać bitmapę względem viewportu
			{
				int mouseX = GET_X_LPARAM(lParam);
				int mouseY = GET_Y_LPARAM(lParam);
				/*const WCHAR debugStringFormat[] = L"Pozycja myszy (%d, %d), przyciski %d\n";
				LPWSTR debugString = new WCHAR[sizeof(debugStringFormat) + 16];
				wsprintfW(debugString, debugStringFormat, mouseX, mouseY, wParam);
				OutputDebugStringW(debugString);*/
				//jeżeli mysz opuści okno, wyślij odpowiedni komunikat
				TRACKMOUSEEVENT trackMouseEventData = {};
				trackMouseEventData.cbSize = sizeof(TRACKMOUSEEVENT);
				trackMouseEventData.dwFlags = TME_LEAVE;
				trackMouseEventData.hwndTrack = hWnd;
				BOOL trackMouseEventResult = TrackMouseEvent(&trackMouseEventData);
				//przesuwanie fraktala
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				if (windowData->isFractalImageMoved)
				{
					//oblicz przesunięcie kursora
					int deltaX = mouseX - windowData->lastPointerPosition->x;
					int deltaY = mouseY - windowData->lastPointerPosition->y;
					const WCHAR fractalMoveDebugStringFormat[] = L"Przesunięcie fraktala - (%d, %d)\n";
					LPWSTR fractalMoveDebugString = new WCHAR[sizeof(fractalMoveDebugStringFormat) + 16];
					wsprintfW(fractalMoveDebugString, fractalMoveDebugStringFormat, deltaX, deltaY);
					OutputDebugStringW(fractalMoveDebugString);
					//przesuń obraz fraktala
					windowData->fractalImage->offsetX += deltaX;
					windowData->fractalImage->offsetY += deltaY;
					InvalidateRect(
						hWnd,
						NULL,
						FALSE
					);
					//zaktualizuj ostatnią pozycję kursora
					windowData->lastPointerPosition->x = mouseX;
					windowData->lastPointerPosition->y = mouseY;
				}
			}
			break;
		case WM_MOUSELEAVE:
			//reset flagi poruszania bitmapy
			OutputDebugStringW(L"Opuszczono główne okno\n");
			{
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				windowData->isFractalImageMoved = false;
				{
					delete windowData->lastPointerPosition;
					windowData->lastPointerPosition = NULL;
				}
			}
			break;
		case WM_MOUSEWHEEL:
			//za pomocą kółka myszy można zoomować bitmapę fraktala
			{
				const WCHAR debugStringFormat[] = L"Scrollowanie: delta - %d, pozycja - (%d, %d)\n";
				LPWSTR debugString = new WCHAR[sizeof(debugStringFormat) + 16];
				char wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				unsigned short positionX = GET_X_LPARAM(lParam);
				unsigned short positionY = GET_Y_LPARAM(lParam);
				wsprintfW(debugString, debugStringFormat, wheelDelta, positionX, positionY);
				OutputDebugStringW(debugString);
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(windowData->dialogWindowHandle, GWL_USERDATA);
				POINT mousePosition = {
					positionX,
					positionY
				};
				ScreenToClient(
					hWnd,
					&mousePosition
				);
				//zaktualizuj skalę o przesunięcie kółka myszy
				float newScaleRatio = windowData->updatedScale + ((float)(wheelDelta) / 1000.0f);
				//nie oddalaj poniżej skali 1.0
				if (newScaleRatio < 1.0f)
				{
					newScaleRatio = 1.0f;
				}
				else if (newScaleRatio > 6.0f)
				{
					newScaleRatio = 6.0f;
				}
				if (newScaleRatio != windowData->updatedScale)
				{
					windowData->updatedScale = newScaleRatio;
					const WCHAR debugMessageBeginning[] = L"skala bitmapy - ";
					WCHAR scaleRationDebugMessage[sizeof(debugMessageBeginning) + 4] = L"";
					wcscat_s(scaleRationDebugMessage, debugMessageBeginning);
					wcscat_s(scaleRationDebugMessage, floatToString(newScaleRatio));
					OutputDebugStringW(scaleRationDebugMessage);

					//nowy rozmiar bitmapy
					RECT clientRect;
					GetClientRect(hWnd, &clientRect);
					unsigned short newWidth = clientRect.right * newScaleRatio;
					unsigned short newHeight = clientRect.bottom * newScaleRatio;

					//wylicz nowy offset obrazu
					short referenceToOffsetX = windowData->fractalImage->offsetX - mousePosition.x;
					short referenceToOffsetY = windowData->fractalImage->offsetY - mousePosition.y;
					float originalReferenceToOffsetX = (float)referenceToOffsetX / windowData->fractalImage->scale;
					float originalReferenceToOffsetY = (float)referenceToOffsetY / windowData->fractalImage->scale;
					short scaledVectorX = (short)(originalReferenceToOffsetX * newScaleRatio);
					short scaledVectorY = (short)(originalReferenceToOffsetY * newScaleRatio);
					short newOffsetX = mousePosition.x + scaledVectorX;
					short newOffsetY = mousePosition.y + scaledVectorY;

					//rysuj bitmapę fraktala
					UpdateFractalBitmap(
						windowData,
						newWidth,
						newHeight,
						newOffsetX,
						newOffsetY,
						newScaleRatio
					);
				}
			}
			break;
		case WM_LBUTTONDOWN:
			//początek przesuwania bitmapy
			OutputDebugStringW(L"Wciśnięto lewy przycisk myszy\n");
			//rozpocznij "przesuwanie" fraktala
			{
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				windowData->isFractalImageMoved = true;
				windowData->lastPointerPosition = new POINT{};
				windowData->lastPointerPosition->x = GET_X_LPARAM(lParam);
				windowData->lastPointerPosition->y = GET_Y_LPARAM(lParam);
			}
			break;
		case WM_LBUTTONUP:
			//koniec przesuwania bitmapy
			OutputDebugStringW(L"Puszczono lewy przycisk myszy\n");
			{
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				windowData->isFractalImageMoved = false;
				if (windowData->lastPointerPosition != NULL)
				{
					delete windowData->lastPointerPosition;
					windowData->lastPointerPosition = NULL;
				}
			}
			break;
		case WM_SETCURSOR:
			//ustaw kursor na "łapkę" jeśli jest wskazywany viewport
			{
				POINT cursorClientPosition = {};
				GetCursorPos(&cursorClientPosition);
				ScreenToClient(hWnd, &cursorClientPosition);
				RECT clientArea = {};
				GetClientRect(hWnd, &clientArea);
				if (
					(cursorClientPosition.x >= 0 && cursorClientPosition.x <= clientArea.right)
					&&
					(cursorClientPosition.y >= 0 && cursorClientPosition.y <= clientArea.bottom)
					)
				{
					FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
					if (windowData->isFractalImageMoved)
					{
						SetCursor(
							LoadCursorW(
								NULL,
								IDC_UPARROW
							)
						);
					}
					else
					{
						SetCursor(
							LoadCursorW(
								NULL,
								IDC_HAND
							)
						);
					}
				}
				else
				{
					//jeżeli kursor jest poza obszarem client to wyświetl domyslny
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK FractalFormDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			{
				//zainicjuj klasę formularza
				const unsigned char margin = 10;
				FractalDrawingUI* formTest = new FractalDrawingUI(
					hDlg,
					margin,
					margin
				);
				//zmień rozmiar dialog boxa aby pomieścił formularz
				RECT rect = {};
				rect.right = formTest->getWidth() + margin * 2;
				rect.bottom = formTest->getHeight() + margin * 2;
				DWORD dialogStyle = (DWORD)GetWindowLong(hDlg, GWL_STYLE);
				DWORD dialogExStyle = (DWORD)GetWindowLong(hDlg, GWL_EXSTYLE);
				AdjustWindowRectEx(
					&rect,
					dialogStyle,
					FALSE,
					dialogExStyle
				);
				SetWindowPos(
					hDlg,
					NULL,
					0,
					0,
					rect.right - rect.left,
					rect.bottom - rect.top,
					SWP_NOZORDER | SWP_NOMOVE
				);
				//zapisz dane w oknie
				FractalFormDialogData* dialogData = new FractalFormDialogData{};
				dialogData->fractalUI = formTest;
				dialogData->drawingScale = 1.0f;
				SetWindowLongW(
					hDlg,
					GWL_USERDATA,
					(LONG)dialogData
				);
				//ustaw wstępne dane formularza
				formTest->getFractalDefinitionForm()->setValue(getDragonFractal());
			}
			return (INT_PTR)TRUE;
		case WM_COMMAND:
			if (lParam == 0)
			{
				WORD commandId = LOWORD(wParam);
				switch (commandId)
				{
					//zapobiegaj zamknięciu modala po naciśnięciu ENTER lub ESC
					case IDOK:
					case IDCANCEL:
						return (INT_PTR)TRUE;
				}
			}
			else
			{
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(hDlg, GWL_USERDATA);
				//controls have lParam value other than 0
				if (dialogData == NULL)
				{
					return (INT_PTR)FALSE;
				}
				if (dialogData->fractalUI->getRenderButton()->isCommandFromControl(lParam))
				{
					//przycisk "Renderuj"					
					WORD notificationCode = HIWORD(wParam);
					if (notificationCode == BN_CLICKED)
					{
						/*
							Po naciśnięciu tego przycisku należy pobrać obiekt fraktala z formularza.
							Jeżeli ten obiekt jest poprawny, to należy wykalkulować jego punkty
							w przestrzeni matematycznej i odpowiednie piksele dla bitmapy.
							W tym miejscu można przekazać operacje do osobnego wątku, tak aby kalkulacja punktów nie
							blokowała innych komunikatów.

						*/
						HWND mainWindow = GetWindow(hDlg, GW_OWNER);
						FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);
						*fractalWindowData->fractalPointsHandle = NULL;



						Fractal fractalFromForm = dialogData->fractalUI->getFractalDefinitionForm()->getValue();
						if (fractalWindowData->fractal != NULL)
						{
							Fractal* fractalPointer = fractalWindowData->fractal;
							fractalWindowData->fractal = NULL;
							delete fractalWindowData->fractal;
						}
						fractalWindowData->fractal = new Fractal(fractalFromForm);						

						RECT windowRect;
						GetClientRect(mainWindow, &windowRect);
						unsigned short bitmapWidth = windowRect.right;
						unsigned short bitmapHeight = windowRect.bottom;


						fractalWindowData->numberOfPointsToProcess = 100000;

						fractalWindowData->currentFractalPoints = std::shared_ptr < concurrency::concurrent_vector<Point>>(new concurrency::concurrent_vector<Point>);
						{
							if (fractalWindowData->processFractalPointsThread != NULL)
							{
								*fractalWindowData->processFractalPointsThread = false;
							}
							FractalPointsThreadData* fractalPointsInitData = new FractalPointsThreadData{};
							fractalPointsInitData->processThread = fractalWindowData->processFractalPointsThread = std::shared_ptr<bool>(new bool{ true });
							fractalPointsInitData->fractalPointsOutput = fractalWindowData->currentFractalPoints;
							fractalPointsInitData->fractal = fractalFromForm;
							fractalPointsInitData->maxNumberOfPoints = fractalWindowData->numberOfPointsToProcess;
							CreateThread(
								NULL,
								0,
								FractalPointsThread,
								fractalPointsInitData,
								0,
								&fractalWindowData->calculateFractalPointsThreadId
							);
						}
						std::shared_ptr < concurrency::concurrent_vector<BitmapPixel >> pixels(new  concurrency::concurrent_vector<BitmapPixel >);
						{
							if (fractalWindowData->processFractalPixelsThread != NULL)
							{
								*fractalWindowData->processFractalPixelsThread = false;
							}
							FractalPixelsCalculatorThreadData* fractalPixelCalculatorData = new FractalPixelsCalculatorThreadData{};
							fractalPixelCalculatorData->numberOfPointsToProcess = fractalWindowData->numberOfPointsToProcess;
							fractalPixelCalculatorData->processThread = fractalWindowData->processFractalPixelsThread = std::shared_ptr<bool>(new bool{ true });
							fractalPixelCalculatorData->bitmapWidth = bitmapWidth;
							fractalPixelCalculatorData->bitmapHeight = bitmapHeight;
							fractalPixelCalculatorData->clipping = fractalFromForm.getClipping();
							fractalPixelCalculatorData->fractalPointsInput = fractalWindowData->currentFractalPoints;
							fractalPixelCalculatorData->bitmapPixelsOutput = pixels;
							CreateThread(
								NULL,
								0,
								FractalPixelsCalculatorThread,
								fractalPixelCalculatorData,
								0,
								&fractalWindowData->calculateFractalPixelsThreadId
							);
						}
						{
							if (fractalWindowData->processFractalBitmapThread != NULL)
							{
								*fractalWindowData->processFractalBitmapThread = false;
							}
							MonochromaticBitmapThreadData* fractalBitmapThreadData = new MonochromaticBitmapThreadData{};
							fractalBitmapThreadData->numberOfPixelsToProcess = fractalWindowData->numberOfPointsToProcess;
							fractalBitmapThreadData->processThread = fractalWindowData->processFractalBitmapThread = std::shared_ptr<bool>(new bool{ true });
							fractalBitmapThreadData->width = bitmapWidth;
							fractalBitmapThreadData->height = bitmapHeight;
							fractalBitmapThreadData->notifyAboutBitmapUpdateThread = GetCurrentThreadId();
							fractalBitmapThreadData->outputPicture = fractalWindowData->fractalImage;
							fractalBitmapThreadData->bitmapWindowHandle = mainWindow;
							fractalBitmapThreadData->bitmapPixelsInput = pixels;
							fractalBitmapThreadData->newScale = 1.0f;
							fractalBitmapThreadData->newOffsetX = 0;
							fractalBitmapThreadData->newOffsetY = 0;
							HANDLE createFractalBitmapThreadHandle = CreateThread(
								NULL,
								0,
								MonochromaticBitmapThread,
								fractalBitmapThreadData,
								0,
								&fractalWindowData->createFractalBitmapThreadId
							);
						}
						return (INT_PTR)TRUE;
					}
				}
				else if (dialogData->fractalUI->getImportbutton()->isCommandFromControl(lParam))
				{
					dialogData->importDialogWindowHandle = CreateDialog(
						(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
						MAKEINTRESOURCE(IDD_FRAKTALE_IMPORT_FROM_PDF),
						hDlg,
						(DLGPROC)ImportFromPdfProc
					);
					ShowWindow(dialogData->importDialogWindowHandle, SW_SHOW);
				}
				else
				{
					WORD notificationCode = HIWORD(wParam);
					dialogData->fractalUI->getFractalDefinitionForm()->processControlCommand(
						notificationCode,
						(HWND)lParam
					);
				}
			}
			break;
		case WM_DESTROY:
			{
				//zniszcz obiekt formularza
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(hDlg, GWL_USERDATA);
				delete dialogData->fractalUI;
			}
			break;
		case WM_NOTIFY:
			{
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(hDlg, GWL_USERDATA);
				NMHDR* message = (NMHDR*)lParam;
				dialogData->fractalUI->getFractalDefinitionForm()->processNotification(message);
			}
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ImportFromPdfProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_COMMAND:
			if (lParam != 0)
			{
				DWORD notificationElementId = LOWORD(wParam);
				WORD notificationCode = HIWORD(wParam);
				if (notificationCode == BN_CLICKED)
				{
					switch (notificationElementId)
					{
						case IDOK:
							{

								//przetwórz wartość z editboxa
								HWND editControl = GetDlgItem(
									hDlg,
									IDC_EDIT1
								);
								//utwórz bufor o długości wprowadzonego tekstu + 1 dla znaku końca ciągu
								unsigned short minBufferSize = GetWindowTextLengthW(editControl) + 1;
								//pobierz tekst do bufora
								WCHAR* textBuffer = new WCHAR[minBufferSize];
								GetWindowTextW(
									editControl,
									textBuffer,
									minBufferSize
								);
								HWND fractalFormDialogHandle = GetWindow(hDlg, GW_OWNER);
								FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(fractalFormDialogHandle, GWL_USERDATA);
								Fractal** fractalFromPdf = new Fractal*;
								if (parseFractalFromPDF(
									textBuffer,
									fractalFromPdf
								))
								{
									dialogData->fractalUI->getFractalDefinitionForm()->setValue(
										**fractalFromPdf
									);
									delete* fractalFromPdf;
									fractalFromPdf = NULL;
								}
								delete fractalFromPdf;
								delete[] textBuffer;
								DestroyWindow(hDlg);
								return (INT_PTR)TRUE;
							}
						case IDCANCEL:
							DestroyWindow(hDlg);
							return (INT_PTR)TRUE;
					}
				}
			}
			break;
		case WM_DESTROY:
			{
				HWND fractalFormDialogHandle = GetWindow(hDlg, GW_OWNER);
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(fractalFormDialogHandle, GWL_USERDATA);
				dialogData->importDialogWindowHandle = NULL;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

DWORD __stdcall CalculateFractalPointsThread(LPVOID dataAddress)
{
	OutputDebugStringW(L"Wyliczam nowy fraktal\n");
	FractalWindowData* fractalWindowData = (FractalWindowData*)dataAddress;
	*fractalWindowData->fractalPointsHandle = NULL;
	FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(fractalWindowData->dialogWindowHandle, GWL_USERDATA);
	FractalDefinitionForm* fractalForm = dialogData->fractalUI->getFractalDefinitionForm();
	if (!fractalForm->isValid())
	{
		return FALSE;
	}
	Fractal providedFractal = fractalForm->getValue();
	if (fractalWindowData->fractal != NULL)
	{
		delete fractalWindowData->fractal;
		fractalWindowData->fractal = NULL;
	}
	fractalWindowData->fractal = new Fractal(providedFractal);
	//usuń poprzednią tablicę punktów
	if (fractalWindowData->calculatedFractalPoints != NULL)
	{
		Point** fractalPoints = fractalWindowData->calculatedFractalPoints;
		fractalWindowData->calculatedFractalPoints = NULL;
		for (unsigned int i = 0; i < fractalWindowData->numberOfCalculatedPoints; i++)
		{
			Point* pointPointer = fractalPoints[i];
			pointPointer = NULL;
			delete pointPointer;
		}
		delete[] fractalPoints;
	}
	const unsigned int numberOfPointsToCalculate = 100000;

	std::chrono::steady_clock::time_point pointCalculationStart = std::chrono::high_resolution_clock::now();
	//wykalkuluj nowe punkty
	if (providedFractal.isValid())
	{
		//zainicjalizuj bitmapę
		if (fractalWindowData->createFractalBitmapThreadId != NULL)
		{
			PostThreadMessageW(fractalWindowData->createFractalBitmapThreadId, WM_QUIT, 0, 0);
			fractalWindowData->createFractalBitmapThreadId = NULL;
		}
		CreateThread(
			NULL,
			0,
			CalculateFractalBitmapThread,
			dataAddress,
			0,
			&fractalWindowData->createFractalBitmapThreadId
		);
		//utwórz wątek do nasłuchiwania nowych punktów fraktala
		DWORD drawPointRTThreadId;
		CreateThread(
			NULL,
			0,
			DrawFractalBitmapPointsRT,
			dataAddress,
			0,
			&drawPointRTThreadId
		);
		fractalWindowData->calculatedFractalPoints = new Point * [numberOfPointsToCalculate];
		ZeroMemory(fractalWindowData->calculatedFractalPoints, sizeof(Point*) * numberOfPointsToCalculate);
		*fractalWindowData->fractalPointsHandle = fractalWindowData->calculatedFractalPoints;
		Point currentPoint;
		MSG msg;
		for (unsigned int i = 0; i < numberOfPointsToCalculate; i++)
		{
			while (PeekMessageW(&msg, (HWND)-1, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					*fractalWindowData->fractalPointsHandle = NULL;
					PostThreadMessageW(drawPointRTThreadId, WM_QUIT, 0, 0);
					return 0;
				}
			}
			fractalWindowData->calculatedFractalPoints[i] = new Point(currentPoint);
			currentPoint = providedFractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
			//requestuj rysowanie punktu na bitmapie
			PostThreadMessageW(
				drawPointRTThreadId,
				WM_CALCULATE_POINT_PIXEL,
				(WPARAM)i,
				(LPARAM)fractalWindowData->calculatedFractalPoints
			);
			fractalWindowData->numberOfCalculatedPoints = i + 1;
		}
		PostThreadMessageW(drawPointRTThreadId, WM_QUIT, 0, 0);
	}

	std::chrono::steady_clock::time_point pointCalculationEnd = std::chrono::high_resolution_clock::now();
	std::chrono::microseconds pointCalculationDeltaMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(pointCalculationEnd - pointCalculationStart);
	std::wstringstream debugString;
	debugString << L"Czas kalkulacji " << numberOfPointsToCalculate << L" punktów fraktala - " << pointCalculationDeltaMicroseconds.count() << L" qs\n";
	OutputDebugStringW(debugString.str().c_str());


	fractalWindowData->fractalImage->offsetX = 0;
	fractalWindowData->fractalImage->offsetY = 0;
	fractalWindowData->fractalImage->scale = 1.0f;

	return 0;
}

DWORD __stdcall CalculateFractalBitmapThread(LPVOID dataAddress)
{
	OutputDebugStringW(L"Tworzę nową bitmapę fraktala\n");
	FractalWindowData* windowData = (FractalWindowData*)dataAddress;
	if (windowData->fractal != NULL)
	{
		*windowData->bitmapBytesHandle = NULL;
		if (windowData->fractalBitmapBytes != NULL)
		{
			BYTE* bytesPointer = windowData->fractalBitmapBytes;
			windowData->fractalBitmapBytes = NULL;
			delete[] bytesPointer;
		}
		HWND windowHandle = windowData->windowHandle;
		//utwórz nową bitmapę fraktala		
		RECT windowClientRect = {};
		GetClientRect(windowHandle, &windowClientRect);
		windowData->fractalImage->width = windowClientRect.right * windowData->fractalImage->scale;
		windowData->fractalImage->height = windowClientRect.bottom * windowData->fractalImage->scale;
		if (windowData->pixelCalculator != NULL)
		{
			delete windowData->pixelCalculator;
			windowData->pixelCalculator = NULL;
		}
		windowData->pixelCalculator = new PixelCalculator(
			windowData->fractalImage->width,
			windowData->fractalImage->height,
			windowData->fractal->getClipping()
		);

		std::chrono::steady_clock::time_point bitmapCreationStart = std::chrono::high_resolution_clock::now();
		//CreateBitmapIndirect tworzy kopię bitmapy dletego należy usunąć dane struktury aby uniknąć wycieków pamięci
		if (windowData->fractalBitmap != NULL)
		{
			delete windowData->fractalBitmap;
		}
		BITMAP* fractalBitmap = new BITMAP{};
		windowData->fractalBitmap = fractalBitmap;
		fractalBitmap->bmWidth = windowData->fractalImage->width;
		fractalBitmap->bmHeight = windowData->fractalImage->height;
		unsigned short bytesPerRow = ceil(windowData->fractalImage->width / 16.0f) * 2;//segment wiersza to 16 bitów
		fractalBitmap->bmWidthBytes = bytesPerRow;
		windowData->fractalBitmapBitsPerScanline = bytesPerRow * 8;
		fractalBitmap->bmPlanes = 1;
		fractalBitmap->bmBitsPixel = 1;
		unsigned int numberOfBytesForData = bytesPerRow * fractalBitmap->bmHeight;
		fractalBitmap->bmBits = new BYTE[numberOfBytesForData];
		windowData->fractalBitmapBytes = (BYTE*)fractalBitmap->bmBits;
		*windowData->bitmapBytesHandle = windowData->fractalBitmapBytes;
		FillMemory(fractalBitmap->bmBits, numberOfBytesForData, 255);//aby bitmapa była biała musi być wypełniona jedynkami

		//wyświetl czas tworzenia
		std::chrono::steady_clock::time_point bitmapCreationEnd = std::chrono::high_resolution_clock::now();
		std::chrono::microseconds bitmapCreationTime = std::chrono::duration_cast<std::chrono::microseconds>(bitmapCreationEnd - bitmapCreationStart);
		std::wstringstream debugString;
		debugString << L"Czas tworzenia obiektu bitmapy o rozmiarze " << windowData->fractalImage->width << L" na " << windowData->fractalImage->height << L" pikseli - " << bitmapCreationTime.count() << L" qs\n";
		OutputDebugStringW(debugString.str().c_str());
		//narysuj piksele na bitmapie
		if (drawFractalV2(
			&windowData->fractal->getClipping(),
			windowData->calculatedFractalPoints,
			windowData->numberOfCalculatedPoints,
			fractalBitmap,
			windowData->fractalImage->width,
			windowData->fractalImage->height,
			windowData->bitmapBytesHandle
		))
		{
			RefreshFractalBitmap(windowData);
			RefreshViewport(windowData);
			return 0;
		}
		else
		{
			return 2;
		}
	}
	return 1;
}

DWORD __stdcall DrawFractalBitmapPointsRT(LPVOID dataAddress)
{
	OutputDebugStringW(L"Uruchamiam listener rysowania punktów fraktala\n");
	FractalWindowData* windowData = (FractalWindowData*)dataAddress;
	MSG msg;
	bool bitmapInitialized = false;
	BYTE* initialiBitmapBytes = NULL;
	Point** initialzFractalPoints = *windowData->fractalPointsHandle;
	while (GetMessageW(&msg, (HWND)-1, 0, 0))
	{
		switch (msg.message)
		{
			case WM_QUIT:
				return 0;
			case WM_CALCULATE_POINT_PIXEL:
				{
					if (bitmapInitialized)
					{
						BYTE* currentBitmapBytes = *windowData->bitmapBytesHandle;
						Point** currentFractalPoints = *windowData->fractalPointsHandle;
						Point** sentFractalPoints = (Point**)msg.lParam;
						if (currentBitmapBytes == initialiBitmapBytes && sentFractalPoints == currentFractalPoints)
						{
							unsigned int fractalPointIndex = (unsigned int)msg.wParam;
							Point* pointPointer = sentFractalPoints[fractalPointIndex];
							if (pointPointer == NULL)
							{
								return 1;
							}
							Point pointToProcess = *pointPointer;
							unsigned short pixelX = windowData->pixelCalculator->getPixelX(pointToProcess.GetX());
							unsigned short pixelY = windowData->pixelCalculator->getPixelY(pointToProcess.GetY());
							markMonochromeBitmapPixelBlack(
								windowData->fractalBitmapBitsPerScanline,
								windowData->bitmapBytesHandle,
								pixelX,
								pixelY
							);
							RefreshFractalBitmap(windowData);
							RefreshViewport(windowData);
						}
					}
					else
					{
						if (*windowData->bitmapBytesHandle != NULL)
						{
							initialiBitmapBytes = *windowData->bitmapBytesHandle;
							OutputDebugStringW(L"Bitmapa zaincjowana, więc kopiuję punkty, które już zostały wyliczone\n");
							//skopiuj piksele, które już zostały zapisane
							drawFractalV2(
								&windowData->fractal->getClipping(),
								windowData->calculatedFractalPoints,
								windowData->numberOfCalculatedPoints,
								windowData->fractalBitmap,
								windowData->fractalImage->width,
								windowData->fractalImage->height,
								windowData->bitmapBytesHandle
							);
							RefreshFractalBitmap(windowData);
							RefreshViewport(windowData);
							bitmapInitialized = true;
						}
					}
				}
				break;
		}
	}
	return 0;
}

DWORD WINAPI FractalPointsThread(LPVOID inputPointer)
{
	FractalPointsThreadData* input = (FractalPointsThreadData*) inputPointer;
	FractalPointsThreadData operationData = *input;
	delete input;
	unsigned int currentPointIndex = 0;
	Point currentPoint;
	while (*operationData.processThread)
	{
		if (currentPointIndex < operationData.maxNumberOfPoints)
		{
			operationData.fractalPointsOutput->push_back(currentPoint);
			currentPoint = operationData.fractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
			currentPointIndex++;
		}
		else
		{
			*operationData.processThread = false;
		}
	}
	return 0;
}

DWORD WINAPI MonochromaticBitmapThread(LPVOID inputPointer)
{
	MonochromaticBitmapThreadData* input = (MonochromaticBitmapThreadData*)inputPointer;
	MonochromaticBitmapThreadData operationData = *input;
	delete input;

	BITMAP monochromeBitmap = BITMAP{};
	monochromeBitmap.bmPlanes = 1;
	monochromeBitmap.bmBitsPixel = 1;
	monochromeBitmap.bmHeight = operationData.height;
	monochromeBitmap.bmWidth = operationData.width;
	monochromeBitmap.bmWidthBytes = ceil(operationData.width / 16.0f) * 2;
	unsigned short bitsPerScanline = monochromeBitmap.bmWidthBytes * 8;
	unsigned int noBytesRequired = monochromeBitmap.bmWidthBytes * monochromeBitmap.bmHeight;
	BYTE* pixelBytes = new BYTE[noBytesRequired];
	FillMemory(pixelBytes, noBytesRequired, 255);
	monochromeBitmap.bmBits = (void*)pixelBytes;
	std::chrono::steady_clock::time_point lastBitmapRefresh = std::chrono::high_resolution_clock::now();
	unsigned int lastProcessedPixelIndex = 0;
	bool firstBitmapUpdate = true;
	while (*operationData.processThread)
	{
		unsigned int numberOfOutputtedPixelIndex = operationData.bitmapPixelsInput->size();
		unsigned int numberOfProcessedPixels = 0;
		unsigned int numberOfPixelsToProcess = numberOfOutputtedPixelIndex - lastProcessedPixelIndex;
		//for (unsigned int pixelIndex = lastProcessedPixelIndex; pixelIndex < numberOfOutputtedPixelIndex; pixelIndex++)
		//{
		//	if (*operationData.processThread)
		//	{			
		//		BitmapPixel pixel = {};
		//		try {
		//			pixel = (BitmapPixel)operationData.bitmapPixelsInput->at(pixelIndex);
		//		}
		//		catch (const std::out_of_range& ex)
		//		{
		//			continue;
		//		}
		//		if (pixel.x < operationData.width && pixel.y < operationData.height)
		//		{
		//			MarkMononochromeBitmapAsText(
		//				pixel,
		//				bitsPerScanline,
		//				pixelBytes
		//			);
		//		}
		//		//numberOfProcessedPixels++;
		//	}
		//}
		if (numberOfPixelsToProcess > 0)
		{
			concurrency::parallel_for(
				(unsigned int)lastProcessedPixelIndex,
				numberOfOutputtedPixelIndex,
				(unsigned int)1,
				[&](unsigned int pixelIndex)
			{
					BitmapPixel pixel = {};
					bool processPixel = true;
					try {
						pixel = operationData.bitmapPixelsInput->at(pixelIndex);
					}
					catch (const std::out_of_range& ex)
					{
						processPixel = false;
					}
					if (processPixel)
					{
						if (pixel.x < operationData.width && pixel.y < operationData.height)
						{
							MarkMononochromeBitmapAsText(
								pixel,
								bitsPerScanline,
								pixelBytes
							);
						}
					}
			}
			);
			if (numberOfOutputtedPixelIndex == operationData.numberOfPixelsToProcess)
			{
				HBITMAP previousBitmap = operationData.outputPicture->bitmap;
				operationData.outputPicture->bitmap = CreateBitmapIndirect(&monochromeBitmap);
				if (previousBitmap != NULL)
				{
					DeleteObject(previousBitmap);
				}
				if (firstBitmapUpdate)
				{
					operationData.outputPicture->offsetX = operationData.newOffsetX;
					operationData.outputPicture->offsetY = operationData.newOffsetY;
					operationData.outputPicture->scale = operationData.newScale;
					operationData.outputPicture->width = operationData.width;
					operationData.outputPicture->height = operationData.height;
					firstBitmapUpdate = false;
				}
				InvalidateRect(
					operationData.bitmapWindowHandle,
					NULL,
					FALSE
				);
				*operationData.processThread = false;
			}
			else
			{
				lastProcessedPixelIndex = numberOfOutputtedPixelIndex;

				std::chrono::steady_clock::time_point currentTS = std::chrono::high_resolution_clock::now();
				long long noMillisecondsSinceLastPainting = std::chrono::duration_cast<std::chrono::milliseconds>(currentTS - lastBitmapRefresh).count();
				if (noMillisecondsSinceLastPainting >= 16)
				{
					HBITMAP previousBitmap = operationData.outputPicture->bitmap;
					operationData.outputPicture->bitmap = CreateBitmapIndirect(&monochromeBitmap);
					if (previousBitmap != NULL)
					{
						DeleteObject(previousBitmap);
					}
					if (firstBitmapUpdate)
					{
						operationData.outputPicture->offsetX = operationData.newOffsetX;
						operationData.outputPicture->offsetY = operationData.newOffsetY;
						operationData.outputPicture->scale = operationData.newScale;
						operationData.outputPicture->width = operationData.width;
						operationData.outputPicture->height = operationData.height;
						firstBitmapUpdate = false;
					}
					InvalidateRect(
						operationData.bitmapWindowHandle,
						NULL,
						FALSE
					);
					lastBitmapRefresh = currentTS;
				}
			}
		}
	}
	delete[] pixelBytes;
	return 0;
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

DWORD WINAPI FractalPixelsCalculatorThread(LPVOID inputPointer)
{
	FractalPixelsCalculatorThreadData* input = (FractalPixelsCalculatorThreadData*)inputPointer;
	FractalPixelsCalculatorThreadData operationData = *input;
	delete input;
	
	PixelCalculator pixelCalculator(
		operationData.bitmapWidth,
		operationData.bitmapHeight,
		operationData.clipping
	);
	unsigned int lastProcessedPointIndex = 0;
	unsigned int lastOutputtedPointIndex = 0;
	unsigned int numberOfProcessedPoints = 0;
	unsigned int numberOfPointsToProcess = 0;
	while (*operationData.processThread)
	{
		lastOutputtedPointIndex = operationData.fractalPointsInput->size();
		numberOfProcessedPoints = 0;
		numberOfPointsToProcess = lastOutputtedPointIndex - lastProcessedPointIndex;
		if (numberOfPointsToProcess > 0)
		{
			/*for (unsigned int pointIndex = lastProcessedPointIndex; pointIndex < lastOutputtedPointIndex; pointIndex++)
			{
				if (*operationData.processThread)
				{
					Point pointBuffer;
					try {
						pointBuffer = fractalPointsInput->at(pointIndex);
					}
					catch (const std::out_of_range& ex)
					{
						continue;
					}
					BitmapPixel pixel = {};
					pixel.x = pixelCalculator.getPixelX(pointBuffer.GetX());
					pixel.y = pixelCalculator.getPixelY(pointBuffer.GetY());
					bitmapPixelsOutput->push_back(pixel);
				}
			}*/
			concurrency::parallel_for(
				(unsigned int)lastProcessedPointIndex,
				lastOutputtedPointIndex,
				(unsigned int)1,
				[&](unsigned int pointIndex)
			{
				Point pointBuffer;
				BitmapPixel pixel = {};
				bool processPoint = true;
				try {
					pointBuffer = operationData.fractalPointsInput->at(pointIndex);
				}
				catch (const std::out_of_range& ex)
				{
					processPoint = false;
				}
				if (processPoint)
				{					
					pixel.x = pixelCalculator.getPixelX(pointBuffer.GetX());
					pixel.y = pixelCalculator.getPixelY(pointBuffer.GetY());
					operationData.bitmapPixelsOutput->push_back(pixel);
				}
			}
			);
			if (lastOutputtedPointIndex == (operationData.numberOfPointsToProcess - 1)) {
				*operationData.processThread = false;
			}
			else
			{
				lastProcessedPointIndex = lastOutputtedPointIndex;
			}
		}
	}
	return 0;
}

void RefreshFractalBitmap(FractalWindowData* windowData)
{
	if (windowData->fractalBitmap != NULL)
	{
		HBITMAP newBitmap = CreateBitmapIndirect(
			windowData->fractalBitmap
		);
		if (newBitmap == NULL)
		{
			OutputDebugStringW(L"Błąd tworzenia uchwytu bitmapy\n");
			debugLastError();
			return;
		}
		HDC windowDeviceContext = GetDC(windowData->windowHandle);
		HDC fractalDrawingDC = CreateCompatibleDC(windowDeviceContext);
		ReleaseDC(windowData->windowHandle, windowDeviceContext);

		SelectObject(fractalDrawingDC, newBitmap);
		//dodaj ramkę
		HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RECT frameRect = {};
		frameRect.right = windowData->fractalImage->width;
		frameRect.bottom = windowData->fractalImage->height;
		FrameRect(fractalDrawingDC, &frameRect, blackBrush);
		//wywołaj przerysowanie okna
		DeleteDC(fractalDrawingDC);
		//skopiuj uchwyt i usuń starą bitmapę na sam koniec aby obraz nie migał
		if (windowData->fractalImage->bitmap != NULL)
		{
			HBITMAP oldBitmap = windowData->fractalImage->bitmap;
			windowData->fractalImage->bitmap = newBitmap;
			DeleteObject(oldBitmap);
		}
		else
		{
			windowData->fractalImage->bitmap = newBitmap;
		}
	}
}

void RefreshViewport(FractalWindowData* windowData)
{
	long long noMillisecondsSinceLastPainting = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - windowData->lastPainingTS).count();
	if (noMillisecondsSinceLastPainting >= 10)
	{
		InvalidateRect(
			windowData->windowHandle,
			NULL,
			FALSE
		);
	}
}

void UpdateFractalBitmap(
	FractalWindowData* windowData,
	unsigned short newWidth,
	unsigned short newHeight,
	short newOffsetX,
	short newOffsetY,
	float newScale
)
{
	if (windowData->fractal == NULL)
	{
		return;
	}
	std::shared_ptr < concurrency::concurrent_vector<BitmapPixel >> pixels(new  concurrency::concurrent_vector<BitmapPixel >);
	std::shared_ptr<bool> isLastPixel(new bool{ false });
	{
		if (windowData->processFractalPixelsThread != NULL)
		{
			*windowData->processFractalPixelsThread = false;
		}
		FractalPixelsCalculatorThreadData* fractalPixelCalculatorData = new FractalPixelsCalculatorThreadData{};
		fractalPixelCalculatorData->processThread = windowData->processFractalPixelsThread = std::shared_ptr<bool>(new bool{ true });
		fractalPixelCalculatorData->numberOfPointsToProcess = windowData->numberOfPointsToProcess;
		fractalPixelCalculatorData->bitmapWidth = newWidth;
		fractalPixelCalculatorData->bitmapHeight = newHeight;
		fractalPixelCalculatorData->clipping = windowData->fractal->getClipping();
		fractalPixelCalculatorData->fractalPointsInput = windowData->currentFractalPoints;
		fractalPixelCalculatorData->bitmapPixelsOutput = pixels;
		CreateThread(
			NULL,
			0,
			FractalPixelsCalculatorThread,
			fractalPixelCalculatorData,
			0,
			&windowData->calculateFractalPixelsThreadId
		);
	}
	{
		if (windowData->processFractalBitmapThread != NULL)
		{
			*windowData->processFractalBitmapThread = false;
		}
		MonochromaticBitmapThreadData* fractalBitmapThreadData = new MonochromaticBitmapThreadData{};
		fractalBitmapThreadData->processThread = windowData->processFractalBitmapThread = std::shared_ptr<bool>(new bool{ true });
		fractalBitmapThreadData->numberOfPixelsToProcess = windowData->numberOfPointsToProcess;
		fractalBitmapThreadData->width = newWidth;
		fractalBitmapThreadData->height = newHeight;
		fractalBitmapThreadData->notifyAboutBitmapUpdateThread = GetCurrentThreadId();
		fractalBitmapThreadData->outputPicture = windowData->fractalImage;
		fractalBitmapThreadData->bitmapWindowHandle = windowData->windowHandle;
		fractalBitmapThreadData->bitmapPixelsInput = pixels;
		fractalBitmapThreadData->newOffsetX = newOffsetX;
		fractalBitmapThreadData->newOffsetY = newOffsetY;
		fractalBitmapThreadData->newScale = newScale;
		HANDLE createFractalBitmapThreadHandle = CreateThread(
			NULL,
			0,
			MonochromaticBitmapThread,
			fractalBitmapThreadData,
			0,
			&windowData->createFractalBitmapThreadId
		);
	}
	windowData->fractalImage->width = newWidth;
	windowData->fractalImage->height = newHeight;
}
