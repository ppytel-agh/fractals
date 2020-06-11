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

/*
	Klasa wrapująca bufor GDI rysowanego okna.
	Umożliwia rysowanie po buforze w tle.
	Parametry WM_PAINT należy przekazać do redrawWindow aby odrysować daną część okna.
	Kopiowanie obiektu tworzy niezależne obiekty GDI zawierające dotychczasową zawartość kopiowanej instancji.
*/
class WindowDrawing
{
private:
	HWND windowHandle;
	HDC windowClientCompatibleDC;
	HBITMAP bitmap;
	unsigned short width;
	unsigned short height;
public:
	WindowDrawing(
		HWND window,
		unsigned short width,
		unsigned short height
	);
	WindowDrawing(const WindowDrawing& original);
	WindowDrawing& operator= (const WindowDrawing& original);
	~WindowDrawing();
	HDC getWindowDrawingBuffer(void);
	void redrawWindow(HDC wmPaintDC, PAINTSTRUCT& wmPaintPS);
};

class FractalFormDialog
{
private:
	FractalDrawingUI* formTest;
public:
	FractalFormDialog(HWND hDlg, unsigned char margin);
	~FractalFormDialog();
	FractalDrawingUI* getFormTest(void);
};

class FractalWindow
{
private:
	HWND dialogHandle;
	WindowDrawing* fractalBuffer;
public:
	FractalWindow(HWND windowHandle); //należy wywoływać w WM_CREATE
	~FractalWindow();
	void setFractalBuffer(WindowDrawing* fractalBuffer);
	void redrawFractal(HDC paintingHdc, PAINTSTRUCT& ps); //należy wywoływać w WM_PAINT
};

const unsigned short fractalRenderWidth = 800;
const unsigned short fractalRenderHeight = 600;

Fractal getDragonFractal(void);

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
		FractalWindow* fractalWindow = (FractalWindow*)GetWindowLongW(hWnd, GWL_USERDATA);
		fractalWindow->redrawFractal(hdc, ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		FractalWindow* fractalWindow = (FractalWindow*) GetWindowLongW(hWnd, GWL_USERDATA);
		delete fractalWindow;
		PostQuitMessage(0);
		break;
	case WM_CREATE:		
	{
		//
		FractalDrawing fractalDrawing = FractalDrawing(fractalRenderWidth, fractalRenderHeight);
		FractalWindow* fractalWindow = new FractalWindow(hWnd);
		SetWindowLongW(
			hWnd,
			GWL_USERDATA,
			(LONG) fractalWindow
		);		
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
			FractalFormDialog* dialogData = new FractalFormDialog(hDlg, 10);
			SetWindowLongW(
				hDlg,
				GWL_USERDATA,
				(LONG) dialogData
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
			FractalFormDialog* dialogData = (FractalFormDialog*)GetWindowLongW(hDlg, GWL_USERDATA);
			//controls have lParam value other than 0
			if (dialogData->getFormTest()->getRenderButton()->isCommandFromControl(lParam))
			{
				//przycisk "Renderuj"
				WORD notificationCode = HIWORD(wParam);
				if (notificationCode == BN_CLICKED)
				{
					//pobierz fraktal z formularza
					Fractal providedFractal = dialogData->getFormTest()->getFractalDefinitionForm()->getValue();					
					HWND mainWindow = GetParent(hDlg);
					//narysuj fraktal w buforze
					WindowDrawing* fractalBuffer = new WindowDrawing(
						mainWindow,
						fractalRenderWidth,
						fractalRenderHeight
					);
					fractalDrawing.drawFractal(
						providedFractal,
						fractalBuffer->getWindowDrawingBuffer()
					);
					//ustaw nowy bufor okna
					FractalWindow* fractalWindow = (FractalWindow*)GetWindowLongW(mainWindow, GWL_USERDATA);
					fractalWindow->setFractalBuffer(fractalBuffer);
					//wywołaj przerysowanie okna
					InvalidateRect(
						mainWindow,
						NULL,
						FALSE
					);
					return (INT_PTR)TRUE;
				}
			}
			else if (dialogData->getFormTest()->getRenderButton()->isCommandFromControl(lParam))
			{
				//przycisk "Importuj z PDF-a"
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

Fractal getDragonFractal(void)
{
	const unsigned char dragonFractalRowsNumber = 2;
	AffineTransformationRow dragonFractalRows[dragonFractalRowsNumber] = {
				AffineTransformationRow(
					18,
					AffineTransformation(
						-0.4f,
						0.0f,
						-1.0f,
						0.0f,
						-0.4f,
						0.1f
					)
				),
				AffineTransformationRow(
					82,
					AffineTransformation(
						0.76f,
						-0.4f,
						0.0f,
						0.4f,
						0.76f,
						0.0f
					)
				)
	};
	return Fractal(
		AffineTransformationRowsGroup(
			dragonFractalRows,
			dragonFractalRowsNumber
		),
		FractalClipping(
			-1.3f,
			0.6f,
			-0.9f,
			0.45f
		)
	);
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
	BitBlt(
		wmPaintDC,
		0,
		0,
		width,
		height,
		windowClientCompatibleDC,
		0,
		0,
		SRCCOPY
	);
}

FractalWindow::FractalWindow(HWND windowHandle)
{
	dialogHandle = CreateDialog(
		hInst,
		MAKEINTRESOURCE(IDD_FRAKTALE_DIALOG),
		windowHandle,
		(DLGPROC)FractalFormDialogProc
	);
	ShowWindow(dialogHandle, SW_SHOW);
}

FractalWindow::~FractalWindow()
{
	delete fractalBuffer;
}

void FractalWindow::setFractalBuffer(WindowDrawing* fractalBuffer)
{
	this->fractalBuffer = fractalBuffer;
}

void FractalWindow::redrawFractal(HDC paintingHdc, PAINTSTRUCT& ps)
{
	if (fractalBuffer != NULL)
	{
		fractalBuffer->redrawWindow(
			paintingHdc,
			ps
		);
	}
}

FractalFormDialog::FractalFormDialog(HWND hDlg, unsigned char margin)
{
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

FractalFormDialog::~FractalFormDialog()
{
	delete formTest;
}

FractalDrawingUI* FractalFormDialog::getFormTest(void)
{
	return formTest;
}
