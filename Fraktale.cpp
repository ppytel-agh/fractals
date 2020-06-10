// Fraktale.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Fraktale.h"
#include "Fractals.h"
#include <cstdlib>
#include <ctime>
#include "FractalsGUI.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProc(HWND, UINT, WPARAM, LPARAM);

class FractalDrawing
{
private:
	unsigned short clientWidth;
	unsigned short clientHeight;
public:
	FractalDrawing(
		unsigned short clientWidth,
		unsigned short clientHeight
	);
	void drawFractal(
		Fractal fractal,
		HDC clientHdc
	);
};

FractalDrawing fractalDrawing = FractalDrawing(800, 600);
FractalDrawingUI* formTest;
Fractal* definedFractalPointer = NULL;

HWND dialogHandle;

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

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_FRAKTALE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FRAKTALE));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		bool isTranslated = TranslateAccelerator(msg.hwnd, hAccelTable, &msg);
		bool isDialog = IsDialogMessage(dialogHandle, &msg);
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
ATOM MyRegisterClass(HINSTANCE hInstance)
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		if (definedFractalPointer != NULL)
		{
			if (definedFractalPointer->isValid())
			{
				fractalDrawing.drawFractal(
					*definedFractalPointer,
					hdc
				);
			}
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		delete formTest;
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		/*pAffineTransformationForm = new AffineTransformationForm(
			hWnd,
			50,
			50
		);*/
	{
		dialogHandle = CreateDialog(
			hInst,
			MAKEINTRESOURCE(IDD_FRAKTALE_DIALOG),
			hWnd,
			(DLGPROC)FractalFormDialogProc
		);
		ShowWindow(dialogHandle, SW_SHOW);
	}
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE || wParam == VK_RETURN)
		{
			return 0;
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

			unsigned char margin = 10;
			formTest = new FractalDrawingUI(
				hDlg,
				margin,
				margin
			);
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
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (lParam == 0)
		{
			WORD commandId = LOWORD(wParam);
			switch (commandId)
			{
			case IDOK:
			case IDCANCEL:
				return (INT_PTR)TRUE;
			}
		}
		else
		{
			//controls have lParam value other than 0
			if (formTest->getRenderButton()->isCommandFromControl(lParam))
			{
				//przycisk "Renderuj"
				WORD notificationCode = HIWORD(wParam);
				if (notificationCode == BN_CLICKED)
				{
					if (definedFractalPointer != NULL)
					{
						delete definedFractalPointer;
						definedFractalPointer = NULL;
					}
					Fractal tmpFractal = formTest->getFractalDefinitionForm()->getValue();
					definedFractalPointer = new Fractal(tmpFractal);
					InvalidateRect(
						GetParent(hDlg),
						NULL,
						FALSE
					);
					return (INT_PTR)TRUE;
				}
			}
			else if (formTest->getRenderButton()->isCommandFromControl(lParam))
			{
				//przycisk "Importuj z PDF-a"
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

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
			800,
			600,
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
	}
}
