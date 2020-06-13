// Fraktale.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Fraktale.h"
#include "Fractals.h"
#include <cstdlib>
#include <ctime>
#include "FractalsGUI.h"

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance, WCHAR szWindowClass[]);
BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow, WCHAR szWindowClass[], WCHAR szTitle[], HWND& hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FractalFormDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ImportFromPdfProc(HWND, UINT, WPARAM, LPARAM);

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
	unsigned short getClientWidth(void);
	unsigned short getClientHeight(void);
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

Fractal getDragonFractal(void);

struct FractalWindowData
{
	HWND dialogWindowHandle;
	FractalDrawing* fractalDrawing;
};

struct FractalFormDialogData
{
	FractalDrawingUI* fractalUI;
	WindowDrawing* fractalBuffer;
	HWND importDialogWindowHandle;
};

void updateFractal(
	HWND windowHandle,
	HWND dialogHandle,
	FractalWindowData* windowData
);

void parseImportAndPutIntoForm(
	WCHAR* textBuffer,
	FractalDefinitionForm* fractalForm
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
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...				
				FractalWindowData* windowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(windowData->dialogWindowHandle, GWL_USERDATA);
				if (dialogData->fractalBuffer != NULL)
				{
					dialogData->fractalBuffer->redrawWindow(hdc, ps);
				}
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			{

				FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				delete fractalWindowData->fractalDrawing;
				PostQuitMessage(0);
				break;
			}
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
				windowData->dialogWindowHandle = dialogHandle;
				windowData->fractalDrawing = new FractalDrawing(
					createData->cx,
					createData->cy
				);
				SetWindowLongW(
					hWnd,
					GWL_USERDATA,
					(LONG)windowData
				);
			}
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE || wParam == VK_RETURN)
			{
				return 0;
			}
			break;
		case WM_SIZE:
			{

				RECT newSize;
				GetClientRect(hWnd, &newSize);
				FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(hWnd, GWL_USERDATA);
				delete fractalWindowData->fractalDrawing;
				fractalWindowData->fractalDrawing = new FractalDrawing(
					newSize.right,
					newSize.bottom
				);

				updateFractal(
					hWnd,
					fractalWindowData->dialogWindowHandle,
					fractalWindowData
				);
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
				SetWindowLongW(
					hDlg,
					GWL_USERDATA,
					(LONG)dialogData
				);
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
						HWND mainWindow = GetWindow(hDlg, GW_OWNER);
						FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);
						updateFractal(
							mainWindow,
							hDlg,
							fractalWindowData
						);
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
			}
			break;
		case WM_DESTROY:
			//zniszcz obiekt formularza
			FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(hDlg, GWL_USERDATA);
			delete dialogData->fractalUI;
			if (dialogData->fractalBuffer != NULL)
			{
				delete dialogData->fractalBuffer;
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
								parseImportAndPutIntoForm(
									textBuffer,
									dialogData->fractalUI->getFractalDefinitionForm()
								);
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

void updateFractal(
	HWND windowHandle,
	HWND dialogHandle,
	FractalWindowData* windowData
)
{
	//przekaż fraktal do głównego okna w celu przerysowania bufora
	FractalFormDialogData* dialogData = (FractalFormDialogData*)GetWindowLongW(dialogHandle, GWL_USERDATA);
	if (dialogData->fractalBuffer != NULL)
	{
		delete dialogData->fractalBuffer;
		dialogData->fractalBuffer = NULL;
	}
	dialogData->fractalBuffer = new WindowDrawing(
		windowHandle,
		windowData->fractalDrawing->getClientWidth(),
		windowData->fractalDrawing->getClientHeight()
	);
	//pobierz fraktal z formularza
	//Fractal providedFractal = getDragonFractal();
	FractalDefinitionForm* fractalForm = dialogData->fractalUI->getFractalDefinitionForm();
	if (!fractalForm->isValid())
	{
		return;
	}
	Fractal providedFractal = fractalForm->getValue();
	windowData->fractalDrawing->drawFractal(
		providedFractal,
		dialogData->fractalBuffer->getWindowDrawingBuffer()
	);
	//wywołaj przerysowanie okna
	InvalidateRect(
		windowHandle,
		NULL,
		FALSE
	);
}

void parseImportAndPutIntoForm(
	WCHAR* textBuffer,
	FractalDefinitionForm* fractalForm
)
{
	//policz liczbę linii tekstu
	WCHAR newline[] = L"\r\n";
	unsigned char newlineLength = wcslen(newline);
	WCHAR* substringPointer = textBuffer;
	int numberOfLines = 1;
	while (substringPointer = wcsstr(substringPointer, newline))
	{
		substringPointer += newlineLength;
		numberOfLines++;
	}
	//utwórz tablicę z linijkami
	unsigned char* lineLengths = new unsigned char[numberOfLines];
	WCHAR** linesArray = new WCHAR * [numberOfLines];
	substringPointer = textBuffer;
	for (unsigned char i = 0; i < (numberOfLines - 1); i++)
	{
		WCHAR* lineStart = substringPointer;
		substringPointer = wcsstr(substringPointer, newline);
		unsigned char lineLength = (unsigned char)(substringPointer - lineStart);
		linesArray[i] = new WCHAR[lineLength + 1];
		memcpy((void*)linesArray[i], (void*)lineStart, sizeof(WCHAR) * lineLength);
		linesArray[i][lineLength] = L'\0';
		substringPointer += newlineLength;
		lineLengths[i] = lineLength;
	}
	//ostatnia linijka
	unsigned char lastLineLength = wcslen(substringPointer);
	linesArray[numberOfLines - 1] = new WCHAR[lastLineLength + 1];
	memcpy((void*)linesArray[numberOfLines - 1], (void*)substringPointer, sizeof(WCHAR) * (lastLineLength + 1));
	//iteracja linijek
	for (unsigned char i = 0; i < numberOfLines; i++)
	{
		WCHAR* line = linesArray[i];
		char x = 'd';
	}
	//walidacja nagłówków
	if (numberOfLines > 2)
	{
		int firstLineComparisonResult = wcscmp(linesArray[0], L"prawdopodobieństwo");
		int secondLineComparisonResult = wcscmp(linesArray[1], L"Współczynniki odwzorowania");
		if ((firstLineComparisonResult == 0) && (secondLineComparisonResult == 0))
		{
			//waliacja wierszy
			const unsigned char numberOfLinesPerRow = 7;
			unsigned char expectedNumberOfRows = (numberOfLines - 2) / numberOfLinesPerRow;
			AffineTransformationRow* transformationRowsArray = (AffineTransformationRow*)malloc(sizeof(AffineTransformationRow) * expectedNumberOfRows);
			for (unsigned char i = 0; i < expectedNumberOfRows; i++)
			{
				unsigned char probabilityRow = 2 + (i * 7);
				unsigned char probLineLength = wcslen(linesArray[probabilityRow]);
				unsigned char probability = 0;
				float paramValues[6] = {};
				if (linesArray[probabilityRow][probLineLength - 1] == L'%')
				{
					probability = (unsigned char)_wtoi(linesArray[probabilityRow]);
				}
				else
				{
					return;
				}
				for (unsigned char j = 97; j <= 102; j++)
				{
					WCHAR letter = (WCHAR)(char)(j);
					unsigned char lineIndex = 2 + (i * 7) + (j - 96);
					unsigned char lineLength = wcslen(linesArray[probabilityRow]);
					if (lineLength >= 2)
					{
						if (linesArray[lineIndex][0] == letter && linesArray[lineIndex][1] == L'=')
						{
							paramValues[j - 97] = _wtof((WCHAR*)&linesArray[lineIndex][2]);
						}
						else
						{
							return;
						}
					}
					else
					{
						return;
					}
				}
				AffineTransformation transformation(
					paramValues[0],
					paramValues[1],
					paramValues[2],
					paramValues[3],
					paramValues[4],
					paramValues[5]
				);
				transformationRowsArray[i] = AffineTransformationRow(
					probability,
					transformation
				);
			}
			AffineTransformationRowsGroup rowsGroup(
				transformationRowsArray,
				expectedNumberOfRows
			);
			free(transformationRowsArray);
			LPCWSTR lastLineParts[] = {
			   L"zakres wyświetlanych punktów na ekranie: xmin=",
			   L"; xmax=",
			   L"; ymin=",
			   L"; ymax=",
			};
			const unsigned char noParts = sizeof(lastLineParts) / sizeof(LPCWSTR);
			LPWSTR lastLine = linesArray[numberOfLines - 1];
			LPWSTR substring = lastLine;
			LPWSTR nextSubstring = NULL;
			float clippingParts[noParts] = {};
			for (unsigned char i = 0; i < (noParts-1); i++)
			{
				substring = wcsstr(substring, lastLineParts[i]);
				if (substring != NULL)
				{
					nextSubstring = wcsstr(substring, lastLineParts[i + 1]);
					if (nextSubstring != NULL)
					{
						unsigned char lastLinePartLen = wcslen(lastLineParts[i]);
						unsigned char clippingPartLength = (unsigned char)(nextSubstring - substring) - lastLinePartLen;
						LPWSTR clippingPartSubstring = substring + lastLinePartLen;
						LPWSTR clippingPartString = new WCHAR[clippingPartLength + 1];
						memcpy(clippingPartString, clippingPartSubstring, sizeof(WCHAR) * clippingPartLength);
						clippingPartString[clippingPartLength] = L'\0';
						//zastąp przecinek kropką
						WCHAR* delimiterPointer = wcsrchr(clippingPartString, L',');
						*delimiterPointer = L'.';
						float conversionResult = _wtof(clippingPartString);
						clippingParts[i] = conversionResult;
					}
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
				
			}
			//unsigned char lastLineLength = wcslen(lastLine);
			unsigned char lastPartIndex = noParts - 1;
			unsigned char lastPartLength = wcslen(lastLineParts[lastPartIndex]);
			LPWSTR lastPartSubstring = nextSubstring + lastPartLength;
			LPWSTR lastPartString = new WCHAR[lastPartLength + 1];
			memcpy(lastPartString, lastPartSubstring, sizeof(WCHAR) * lastPartLength);
			lastPartString[lastPartLength] = L'\0';
			WCHAR* delimiterPointer = wcsrchr(lastPartString, L',');
			*delimiterPointer = L'.';
			clippingParts[lastPartIndex] = (float)_wtof(lastPartString);
			FractalClipping clipping(
				clippingParts[0],
				clippingParts[1],
				clippingParts[2],
				clippingParts[3]
			);
			Fractal importedFractal(
				rowsGroup,
				clipping
			);
			fractalForm->setValue(importedFractal);
		}
	}	
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
			this->clientWidth,
			this->clientHeight,
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

unsigned short FractalDrawing::getClientWidth(void)
{
	return clientWidth;
}

unsigned short FractalDrawing::getClientHeight(void)
{
	return this->clientHeight;
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