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
const UINT WM_PROCESS_NEW_POINT = WM_APP + 2;
struct FractalPointsThreadData
{
	Fractal fractal;
	unsigned int maxNumberOfPoints;
	DWORD newPointCallbackThreadId;
};
DWORD WINAPI FractalPointsThread(LPVOID);

struct BitmapPixel
{
	unsigned short x;
	unsigned short y;
};
struct MonochromaticBitmapThreadData
{
	unsigned short width;
	unsigned short height;
	DWORD notifyAboutBitmapUpdateThread;
};
const UINT WM_MARK_PIXEL_AS_TEXT = WM_APP + 3;
const UINT WM_PUT_BITMAP_IN_HANDLE = WM_APP + 4;
const UINT WM_BITMAP_UPDATED = WM_APP + 5;
const UINT WM_PUT_BITMAP_IN_HANDLE_CALLBACK = WM_APP + 6;
DWORD WINAPI MonochromaticBitmapThread(LPVOID);

void MarkMononochromeBitmapAsText(
	BitmapPixel pixel,
	unsigned short bitsPerScanline,
	BYTE* pixelBytes
);

struct FractalPixelsCalculatorThreadData
{
	unsigned short bitmapWidth;
	unsigned short bitmapHeight;
	FractalClipping clipping;
	DWORD bitmapThreadId;
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
};

struct FractalFormDialogData
{
	FractalDrawingUI* fractalUI;
	HWND importDialogWindowHandle;
	float drawingScale;
};

void RefreshFractalBitmap(FractalWindowData* windowData);
void RefreshViewport(FractalWindowData* windowData);

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
		if (msg.message == WM_BITMAP_UPDATED)
		{
			DWORD bitmapThreadId = msg.wParam;
			FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(mainWindowHandle, GWL_USERDATA);
			HBITMAP currentBitmapHandle = windowData->fractalImage->bitmap;
			PostThreadMessageW(
				bitmapThreadId,
				WM_PUT_BITMAP_IN_HANDLE,
				GetCurrentThreadId(),
				(LPARAM)&windowData->fractalImage->bitmap
			);
			//WaitOnAddress(&windowData->fractalImage->bitmap, &currentBitmapHandle, 1, INFINITE);
		}
		else if (msg.message == WM_PUT_BITMAP_IN_HANDLE_CALLBACK)
		{
			InvalidateRect(
				mainWindowHandle,
				NULL,
				FALSE
			);
		}
		else
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
				long long noMillisecondsSinceLastPainting = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - windowData->lastPainingTS).count();
				if (noMillisecondsSinceLastPainting < 10)
				{
					return 0;
				}
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
					windowData->fractalImage->offsetX = 0;
					windowData->fractalImage->offsetY = 0;
					windowData->fractalImage->scale = 1.0f;
					if (windowData->createFractalBitmapThreadId != NULL)
					{
						PostThreadMessageW(windowData->createFractalBitmapThreadId, WM_QUIT, 0, 0);
						windowData->createFractalBitmapThreadId = NULL;
					}
					CreateThread(
						NULL,
						0,
						CalculateFractalBitmapThread,
						windowData,
						0,
						&windowData->createFractalBitmapThreadId
					);
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
				if (resizeNow)
				{
					windowData->fractalImage->offsetX = 0;
					windowData->fractalImage->offsetY = 0;
					windowData->fractalImage->scale = 1.0f;
					if (windowData->createFractalBitmapThreadId != NULL)
					{
						PostThreadMessageW(windowData->createFractalBitmapThreadId, WM_QUIT, 0, 0);
						windowData->createFractalBitmapThreadId = NULL;
					}
					CreateThread(
						NULL,
						0,
						CalculateFractalBitmapThread,
						windowData,
						0,
						&windowData->createFractalBitmapThreadId
					);
				}
			}
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
					RefreshViewport(windowData);
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
				float previousScaleRatio = windowData->fractalImage->scale;
				//zaktualizuj skalę o przesunięcie kółka myszy
				windowData->fractalImage->scale += ((float)(wheelDelta) / 1000.0f);
				//nie oddalaj poniżej skali 1.0
				if (windowData->fractalImage->scale < 1.0f)
				{
					windowData->fractalImage->scale = 1.0f;
				}
				else if (windowData->fractalImage->scale > 6.0f)
				{
					windowData->fractalImage->scale = 6.0f;
				}
				if (previousScaleRatio != windowData->fractalImage->scale)
				{
					const WCHAR debugMessageBeginning[] = L"skala bitmapy - ";
					WCHAR scaleRationDebugMessage[sizeof(debugMessageBeginning) + 4] = L"";
					wcscat_s(scaleRationDebugMessage, debugMessageBeginning);
					wcscat_s(scaleRationDebugMessage, floatToString(windowData->fractalImage->scale));
					OutputDebugStringW(scaleRationDebugMessage);

					//wylicz nowy offset obrazu
					short referenceToOffsetX = windowData->fractalImage->offsetX - mousePosition.x;
					short referenceToOffsetY = windowData->fractalImage->offsetY - mousePosition.y;
					float originalReferenceToOffsetX = (float)referenceToOffsetX / previousScaleRatio;
					float originalReferenceToOffsetY = (float)referenceToOffsetY / previousScaleRatio;
					short scaledVectorX = (short)(originalReferenceToOffsetX * windowData->fractalImage->scale);
					short scaledVectorY = (short)(originalReferenceToOffsetY * windowData->fractalImage->scale);
					windowData->fractalImage->offsetX = mousePosition.x + scaledVectorX;
					windowData->fractalImage->offsetY = mousePosition.y + scaledVectorY;

					//rysuj bitmapę fraktala	
					if (windowData->createFractalBitmapThreadId != NULL)
					{
						PostThreadMessageW(windowData->createFractalBitmapThreadId, WM_QUIT, 0, 0);
						windowData->createFractalBitmapThreadId = NULL;
					}
					CreateThread(
						NULL,
						0,
						CalculateFractalBitmapThread,
						windowData,
						0,
						&windowData->createFractalBitmapThreadId
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

						if (fractalWindowData->createFractalBitmapThreadId != NULL)
						{
							PostThreadMessageW(fractalWindowData->createFractalBitmapThreadId, WM_QUIT, 0, 0);
							fractalWindowData->createFractalBitmapThreadId = NULL;
						}
						if (fractalWindowData->calculateFractalPixelsThreadId != NULL)
						{
							PostThreadMessageW(fractalWindowData->calculateFractalPixelsThreadId, WM_QUIT, 0, 0);
							fractalWindowData->calculateFractalPixelsThreadId = NULL;
						}
						if (fractalWindowData->calculateFractalPointsThreadId != NULL)
						{
							PostThreadMessageW(fractalWindowData->calculateFractalPointsThreadId, WM_QUIT, 0, 0);
							fractalWindowData->calculateFractalPointsThreadId = NULL;
						}

						Fractal fractalFromForm = dialogData->fractalUI->getFractalDefinitionForm()->getValue();
						fractalWindowData->fractalImage->scale = 1.0f;
						fractalWindowData->fractalImage->offsetX = 0;
						fractalWindowData->fractalImage->offsetY = 0;

						RECT windowRect;
						GetWindowRect(mainWindow, &windowRect);
						unsigned short bitmapWidth = windowRect.right;
						unsigned short bitmapHeight = windowRect.bottom;

						fractalWindowData->fractalImage->width = bitmapWidth;
						fractalWindowData->fractalImage->height = bitmapHeight;

						MonochromaticBitmapThreadData fractalBitmapThreadData = {};
						fractalBitmapThreadData.width = bitmapWidth;
						fractalBitmapThreadData.height = bitmapHeight;
						fractalBitmapThreadData.notifyAboutBitmapUpdateThread = GetCurrentThreadId();
						HANDLE createFractalBitmapThreadHandle = CreateThread(
							NULL,
							0,
							MonochromaticBitmapThread,
							&fractalBitmapThreadData,
							0,
							&fractalWindowData->createFractalBitmapThreadId
						);
						WaitOnAddress(&fractalBitmapThreadData, &fractalBitmapThreadData, 1, INFINITE);

						FractalPixelsCalculatorThreadData fractalPixelCalculatorData = {};
						fractalPixelCalculatorData.bitmapWidth = bitmapWidth;
						fractalPixelCalculatorData.bitmapHeight = bitmapHeight;
						fractalPixelCalculatorData.clipping = fractalFromForm.getClipping();
						fractalPixelCalculatorData.bitmapThreadId = fractalWindowData->createFractalBitmapThreadId;
						CreateThread(
							NULL,
							0,
							FractalPixelsCalculatorThread,
							&fractalPixelCalculatorData,
							0,
							&fractalWindowData->calculateFractalPixelsThreadId
						);
						WaitOnAddress(&fractalPixelCalculatorData, &fractalPixelCalculatorData, 1, INFINITE);

						FractalPointsThreadData fractalPointsInitData = {};
						fractalPointsInitData.newPointCallbackThreadId = fractalWindowData->calculateFractalPixelsThreadId;
						fractalPointsInitData.fractal = fractalFromForm;
						fractalPointsInitData.maxNumberOfPoints = 100000;
						CreateThread(
							NULL,
							0,
							FractalPointsThread,
							&fractalPointsInitData,
							0,
							&fractalWindowData->calculateFractalPointsThreadId
						);
						WaitOnAddress(&fractalPointsInitData, &fractalPointsInitData, 1, INFINITE);

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

DWORD WINAPI FractalPointsThread(LPVOID dataStackAddress)
{
	FractalPointsThreadData operationData = *(FractalPointsThreadData*)dataStackAddress;
	WakeByAddressSingle(dataStackAddress);
	unsigned char operationState = 0;
	MSG msg = {};
	Point** fractalPoints = NULL;
	unsigned int numberOfCalculatedPoints = 0;
	unsigned int currentPointIndex = 0;
	Point currentPoint;
	while (1)
	{
		while (PeekMessageW(&msg, (HWND)-1, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
				case WM_QUIT:
					if (fractalPoints != NULL)
					{
						Point** initialFractalPointsAddres = fractalPoints;
						concurrency::parallel_for(
							(unsigned int)0,
							numberOfCalculatedPoints,
							(unsigned int)1,
							[&](int i) {
							delete fractalPoints[i];
							if (i == numberOfCalculatedPoints - 1)
							{
								delete fractalPoints;
								fractalPoints = NULL;
							}
						}
						);
						WaitOnAddress(&fractalPoints, &initialFractalPointsAddres, 1, INFINITE);
					}
					return 0;
			}
		}
		switch (operationState)
		{
			case 0: //walidacja fraktala				
				if (operationData.fractal.isValid())
				{
					operationState++;
				}
				else
				{
					return 1;
				}
				break;
			case 1: //alokacja pamięci dla punktów
				{
					fractalPoints = new Point * [operationData.maxNumberOfPoints];
					numberOfCalculatedPoints++;
					operationState++;
				}
				break;
			case 2: //wyliczanie kolejnych punktów
				{
					if (currentPointIndex < operationData.maxNumberOfPoints)
					{
						fractalPoints[currentPointIndex] = new Point(currentPoint);
						currentPoint = operationData.fractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
						//powiadom wątek zwrotny o nowym punkcie
						PostThreadMessageW(
							operationData.newPointCallbackThreadId,
							WM_PROCESS_NEW_POINT,
							currentPointIndex,
							(LPARAM)fractalPoints
						);
						currentPointIndex++;
					}
					else
					{
						operationState++;
					}
				}
				break;
		}

	}
}

DWORD WINAPI MonochromaticBitmapThread(LPVOID inputPointer)
{
	MonochromaticBitmapThreadData operationData = *(MonochromaticBitmapThreadData*)inputPointer;
	WakeByAddressSingle(inputPointer);
	unsigned char operationState = 0;
	BITMAP monochromeBitmap = BITMAP{};
	unsigned short bitsPerScanline = 0;
	BYTE* pixelBytes = NULL;
	std::vector<BitmapPixel> awaitingPixels;
	MSG msg;
	HBITMAP bitmapHandle = NULL;
	bool updateHandle = true;
	while (1)
	{
		while (PeekMessageW(&msg, (HWND)-1, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
				case WM_QUIT:
					if (pixelBytes != NULL)
					{
						delete[] pixelBytes;
					}
					return 0;
				case WM_MARK_PIXEL_AS_TEXT: //dorysuj kolejny piksel do bitmapy		
					{
						BitmapPixel pixel = {};
						pixel.x = LOWORD(msg.wParam);
						pixel.y = HIWORD(msg.wParam);
						if (pixelBytes == NULL)
						{
							//przechwyć i zapisz do czasu utworzenia tablicy bajtów
							awaitingPixels.push_back(pixel);
						}
						else
						{
							MarkMononochromeBitmapAsText(
								pixel,
								bitsPerScanline,
								pixelBytes
							);
							updateHandle = true;
							PostThreadMessageW(
								operationData.notifyAboutBitmapUpdateThread,
								WM_BITMAP_UPDATED,
								GetCurrentThreadId(),
								0
							);
						}
					}
					break;
				case WM_PUT_BITMAP_IN_HANDLE:
					HBITMAP* bitmapHandlePointer = (HBITMAP*)msg.lParam;
					if (pixelBytes != NULL)
					{
						if (updateHandle)
						{
							bitmapHandle = CreateBitmapIndirect(&monochromeBitmap);
							updateHandle = false;
							PostThreadMessageW(
								msg.wParam,
								WM_PUT_BITMAP_IN_HANDLE_CALLBACK,
								0,
								0
							);
						}
						*bitmapHandlePointer = bitmapHandle;
					}
					else
					{
						//jeżeli bitmapa nie została jeszcze utworzona zakomunikuj, że wskaźnik został zmieniony
						WakeByAddressSingle(bitmapHandlePointer);
					}
					break;
			}
		}
		switch (operationState)
		{
			case 0: //ustawianie zmiennych bitmapy			
				monochromeBitmap.bmPlanes = 1;
				monochromeBitmap.bmBitsPixel = 1;
				monochromeBitmap.bmHeight = operationData.height;
				monochromeBitmap.bmWidth = operationData.width;
				operationState++;
				break;
			case 1: //wyliczanie zmiennych bitmapy
				monochromeBitmap.bmWidthBytes = ceil(operationData.width / 16.0f) * 2;
				bitsPerScanline = monochromeBitmap.bmWidthBytes * 8;
				operationState++;
				break;
			case 2: //alokacja bajtów dla pikseli
				{
					unsigned int noBytesRequired = monochromeBitmap.bmWidthBytes * monochromeBitmap.bmHeight;
					monochromeBitmap.bmBits = pixelBytes = new BYTE[noBytesRequired];
					FillMemory(pixelBytes, noBytesRequired, 255);
					operationState++;
				}
				break;
			case 3:
				//narysuj piksele, które zostały zakomunikowane przed utworzeniem tablicy
				concurrency::parallel_for(
					(unsigned int)0,
					awaitingPixels.size(),
					(unsigned int)1,
					[&](unsigned int i)
				{
					MarkMononochromeBitmapAsText(
						awaitingPixels[i],
						bitsPerScanline,
						pixelBytes
					);
					updateHandle = true;
					PostThreadMessageW(
						operationData.notifyAboutBitmapUpdateThread,
						WM_BITMAP_UPDATED,
						GetCurrentThreadId(),
						0
					);
				}
				);
				operationState++;
				break;
		}

	}
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
	FractalPixelsCalculatorThreadData operationData = *(FractalPixelsCalculatorThreadData*)inputPointer;
	WakeByAddressSingle(inputPointer);

	PixelCalculator pixelCalculator(
		operationData.bitmapWidth,
		operationData.bitmapHeight,
		operationData.clipping
	);
	std::vector<BitmapPixel>calculatedPixels;
	MSG msg;
	while (GetMessageW(&msg, (HWND)-1, 0, 0))
	{
		switch (msg.message)
		{
			case WM_QUIT:
				return 0;
			case WM_PROCESS_NEW_POINT:
				Point** pointsArray = (Point**)msg.lParam;
				BitmapPixel pixel = {};
				pixel.x = pixelCalculator.getPixelX(pointsArray[msg.wParam]->GetX());
				pixel.y = pixelCalculator.getPixelY(pointsArray[msg.wParam]->GetY());
				WPARAM wParam = MAKEWPARAM(pixel.x, pixel.y);
				PostThreadMessageW(
					operationData.bitmapThreadId,
					WM_MARK_PIXEL_AS_TEXT,
					wParam,
					0
				);
				calculatedPixels.push_back(pixel);
				break;
		}
	}
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
