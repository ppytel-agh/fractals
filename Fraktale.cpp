// Fraktale.cpp : Defines the entry point for the application.
//

#include "Fraktale.h"

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

	RealTimeMessageLoop msgLoop(
		MessageProcessor(
			hAccelTable,
			dialogHandle,
			fractalDialogData
		),
		fractalWindowData,
		mainWindowHandle
	);
	return msgLoop.messageLoop();
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

				//zainicjuj dane przesuwania obrazka
				windowData->fractalImage = new MovablePicture{};
				windowData->updatedScale = 1.0f;
				windowData->updateOffsetX = 0;
				windowData->updateOffsetY = 0;

				//bufor obrazka fraktala, wypełnij kolorem tła
				HDC windowDC = GetDC(hWnd);
				windowData->fractalImage->deviceContext = CreateCompatibleDC(windowDC);
				ReleaseDC(hWnd, windowDC);
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				windowData->fractalImage->bitmap = CreateCompatibleBitmap(
					windowData->fractalImage->deviceContext,
					clientRect.right,
					clientRect.bottom
				);
				SelectObject(
					windowData->fractalImage->deviceContext,
					windowData->fractalImage->bitmap
				);
				HBRUSH backgroundBrush = (HBRUSH)GetClassLongW(
					hWnd,
					GCL_HBRBACKGROUND
				);
				FillRect(
					windowData->fractalImage->deviceContext,
					&clientRect,
					backgroundBrush
				);

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
				if (bufferBitmapWidth == 0 && bufferBitmapHeight == 0)
				{
					RECT clientRect;
					GetClientRect(hWnd, &clientRect);
					ps.rcPaint = clientRect;
					bufferBitmapWidth = clientRect.right;
					bufferBitmapHeight = clientRect.bottom;
				}
				else
				{
					char x = 'd';
				}
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
					UpdateFractalBitmap(
						windowData,
						newSize.right,
						newSize.bottom,
						0,
						0,
						1.0f
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
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				if (resizeNow)
				{
					UpdateFractalBitmap(
						windowData,
						clientRect.right,
						clientRect.bottom,
						0,
						0,
						1.0f
					);
				}
			}
			break;
		case WM_ERASEBKGND:
			//nie czyść ekranu bez potrzeby, żeby uniknąć migotania
			break;
		case WM_MOUSEMOVE:
			//jeżeli użytkownik przesuwa myszą z wciśniętym przyciskiem to należy przesuwać bitmapę względem viewportu
			/*
			przesuwanie bitmapy względem widoku możnaby owrapować przynajmiej jakąs funkcją
			*/
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
					windowData->updateOffsetX = windowData->fractalImage->offsetX;
					windowData->updateOffsetY = windowData->fractalImage->offsetY;
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
			/*
			Tę operacje również możnaby owrapować
			*/
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
					short referenceToOffsetX = windowData->updateOffsetX - mousePosition.x;
					short referenceToOffsetY = windowData->updateOffsetY - mousePosition.y;
					float originalReferenceToOffsetX = (float)referenceToOffsetX / windowData->updatedScale;
					float originalReferenceToOffsetY = (float)referenceToOffsetY / windowData->updatedScale;
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
				formTest->setMaxNumberOfPointsToRender(initialNumberOfPointsToRender);
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
						Operację "renderowania" należy owrapować w funkcję,
						tak aby była również możliwość jej wywołania po naciśnięciu klawisza.
						*/
						/*
							Po naciśnięciu tego przycisku należy pobrać obiekt fraktala z formularza.
							Jeżeli ten obiekt jest poprawny, to należy wykalkulować jego punkty
							w przestrzeni matematycznej i odpowiednie piksele dla bitmapy.
							W tym miejscu można przekazać operacje do osobnego wątku, tak aby kalkulacja punktów nie
							blokowała innych komunikatów.

						*/
						HWND mainWindow = GetWindow(hDlg, GW_OWNER);
						FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);



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

						//parametry skalowania
						fractalWindowData->updatedScale = 1.0f;
						fractalWindowData->updateOffsetX = 0;
						fractalWindowData->updateOffsetY = 0;

						if (dialogData->fractalUI->getNumberOfPointsToRender()->isValid())
						{
							fractalWindowData->numberOfPointsToProcess = dialogData->fractalUI->getNumberOfPointsToRender()->getValue();
						}
						else
						{
							fractalWindowData->numberOfPointsToProcess = initialNumberOfPointsToRender;
						}


						InitializeFractalPointsCalculator(fractalWindowData);
						InitializeFractalPointsThread(fractalWindowData);
						InitializeFractalRender(
							fractalWindowData,
							bitmapWidth,
							bitmapHeight
						);

						fractalWindowData->fractalImage->offsetX = 0;
						fractalWindowData->fractalImage->offsetY = 0;
						fractalWindowData->fractalImage->scale = 1.0f;
						fractalWindowData->fractalImage->width = bitmapWidth;
						fractalWindowData->fractalImage->height = bitmapHeight;
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
		case WM_HSCROLL:
			{
				int currentPosition = SendMessageW(
					(HWND)lParam,
					TBM_GETPOS,
					0,
					0
				);
				std::wstringstream stream;
				stream << "Pozycja range'a - " << currentPosition << L"\n";
				OutputDebugStringW(stream.str().c_str());
				HWND mainWindow = GetWindow(hDlg, GW_OWNER);
				FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);
				if (fractalWindowData->currentFractalBitmapGenerator != NULL)
				{
					InitializeFractalBitmapThread(
						fractalWindowData,
						(unsigned int)currentPosition
					);
				}
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

DWORD WINAPI FractalPointsThread(LPVOID inputPointer)
{
	FractalPointsThreadData* input = (FractalPointsThreadData*)inputPointer;
	FractalPointsThreadData operationData = *input;
	delete input;
	operationData.fractalPointsOutput->calculatePoints(
		operationData.maxNumberOfPoints,
		operationData.processThread
	);
	return 0;
}

/*
	Tutaj należałoby oddzielić markowanie pikseli bitmapy od tworzenia uchwytu i requestowania odświeżania okna.
*/
DWORD WINAPI MonochromaticBitmapThread(LPVOID inputPointer)
{
	MonochromaticBitmapThreadData* input = (MonochromaticBitmapThreadData*)inputPointer;
	MonochromaticBitmapThreadData operationData = *input;
	delete input;
	//operationData.fractalBitmapFactory->reset();
	bool allPointsProcessed = false;
	do
	{
		allPointsProcessed = operationData.fractalBitmapFactory->generateBitmap(
			operationData.numberOfPixelsToProcess,
			operationData.processThread
		);
	} while (!allPointsProcessed && *operationData.processThread);
	return 0;
}



DWORD WINAPI FractalPixelsCalculatorThread(LPVOID inputPointer)
{
	FractalPixelsCalculatorThreadData* input = (FractalPixelsCalculatorThreadData*)inputPointer;
	FractalPixelsCalculatorThreadData operationData = *input;
	delete input;

	operationData.fractalPixelsOutput->calculatePixels(operationData.processThread);
	return 0;
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
	windowData->updatedScale = newScale;
	windowData->updateOffsetX = newOffsetX;
	windowData->updateOffsetY = newOffsetY;

	//aktualizacja uchwytu bitmapy
	DeleteObject(windowData->fractalImage->bitmap);
	windowData->fractalImage->bitmap = CreateCompatibleBitmap(
		windowData->fractalImage->deviceContext,
		newWidth,
		newHeight
	);
	SelectObject(windowData->fractalImage->deviceContext, windowData->fractalImage->bitmap);
	HBRUSH backgroundBrush = (HBRUSH)GetClassLongW(
		windowData->windowHandle,
		GCL_HBRBACKGROUND
	);
	RECT fillRect = {};
	fillRect.right = newWidth;
	fillRect.bottom = newHeight;
	FillRect(
		windowData->fractalImage->deviceContext,
		&fillRect,
		backgroundBrush
	);

	InitializeFractalRender(
		windowData,
		newWidth,
		newHeight
	);

	windowData->fractalImage->offsetX = newOffsetX;
	windowData->fractalImage->offsetY = newOffsetY;
	windowData->fractalImage->scale = newScale;
	windowData->fractalImage->width = newWidth;
	windowData->fractalImage->height = newHeight;

}

void InitializeFractalPointsCalculator(FractalWindowData* fractalWindowData)
{
	fractalWindowData->currentFractalPoints = std::shared_ptr < FractalPoints>(
		new FractalPoints(
			*fractalWindowData->fractal,
			Point(0, 0)
		)
	);
}

void InitializeFractalPointsThread(
	FractalWindowData* fractalWindowData
)
{
	if (fractalWindowData->processFractalPointsThread != NULL)
	{
		*fractalWindowData->processFractalPointsThread = false;
	}
	FractalPointsThreadData* fractalPointsInitData = new FractalPointsThreadData{};
	fractalPointsInitData->processThread = fractalWindowData->processFractalPointsThread = std::shared_ptr<bool>(new bool{ true });
	fractalPointsInitData->fractalPointsOutput = fractalWindowData->currentFractalPoints;
	fractalPointsInitData->fractal = *fractalWindowData->fractal;
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

void InitializeFractalPixelsCalculator(
	FractalWindowData* fractalWindowData,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
)
{
	fractalWindowData->fractalPixelsCalculator = std::shared_ptr < FractalPixels>(new  FractalPixels(
		fractalWindowData->currentFractalPoints,
		fractalWindowData->fractal->getClipping(),
		bitmapWidth,
		bitmapHeight,
		fractalWindowData->numberOfPointsToProcess
	));
}

void InitializeFractalPixelsThread(FractalWindowData* fractalWindowData)
{
	if (fractalWindowData->processFractalPixelsThread != NULL)
	{
		*fractalWindowData->processFractalPixelsThread = false;
	}
	FractalPixelsCalculatorThreadData* fractalPixelCalculatorData = new FractalPixelsCalculatorThreadData{};
	fractalPixelCalculatorData->processThread = fractalWindowData->processFractalPixelsThread = std::shared_ptr<bool>(new bool{ true });
	fractalPixelCalculatorData->fractalPixelsOutput = fractalWindowData->fractalPixelsCalculator;
	CreateThread(
		NULL,
		0,
		FractalPixelsCalculatorThread,
		fractalPixelCalculatorData,
		0,
		&fractalWindowData->calculateFractalPixelsThreadId
	);
}

void InitializeFractalBitmapGenerator(FractalWindowData* fractalWindowData)
{
	fractalWindowData->currentFractalBitmapGenerator = std::shared_ptr<FractalBitmapFactory>(new FractalBitmapFactory(
		fractalWindowData->fractalPixelsCalculator,
		fractalWindowData->numberOfPointsToProcess
	));
}

void InitializeFractalBitmapThread(FractalWindowData* fractalWindowData, unsigned int numberOfPointsToRender)
{
	if (fractalWindowData->processFractalBitmapThread != NULL)
	{
		*fractalWindowData->processFractalBitmapThread = false;
	}
	MonochromaticBitmapThreadData* fractalBitmapThreadData = new MonochromaticBitmapThreadData{};
	fractalBitmapThreadData->numberOfPixelsToProcess = numberOfPointsToRender;
	fractalBitmapThreadData->processThread = fractalWindowData->processFractalBitmapThread = std::shared_ptr<bool>(new bool{ true });
	fractalBitmapThreadData->fractalBitmapFactory = fractalWindowData->currentFractalBitmapGenerator;
	HANDLE createFractalBitmapThreadHandle = CreateThread(
		NULL,
		0,
		MonochromaticBitmapThread,
		fractalBitmapThreadData,
		0,
		&fractalWindowData->createFractalBitmapThreadId
	);
}

void InitializeFractalRender(FractalWindowData* fractalWindowData, unsigned short bitmapWidth, unsigned short bitmapHeight)
{
	InitializeFractalRenderV2(
		fractalWindowData,
		bitmapWidth,
		bitmapHeight
	);
}

DWORD __stdcall MonochromaticBitmapThreadV2(LPVOID inputPointer)
{
	MonochromaticBitmapThreadDataV2* input = (MonochromaticBitmapThreadDataV2*)inputPointer;
	MonochromaticBitmapThreadDataV2 operationData = *input;
	delete input;
	//operationData.fractalBitmapFactory->reset();
	bool allPointsProcessed = false;
	do
	{
		allPointsProcessed = operationData.fractalBitmapFactory->generateBitmap(
			operationData.numberOfPixelsToProcess,
			operationData.processThread
		);
		if (operationData.fractalBitmapFactory->copyIntoBuffer(
			operationData.viewBufferDC
		))
		{
			InvalidateRect(
				operationData.viewWindowHandle,
				NULL,
				FALSE
			);
		}
	} while (!allPointsProcessed && *operationData.processThread);
	return 0;
}

DWORD __stdcall FractalPixelsCalculatorThreadV2(LPVOID inputPointer)
{
	FractalPixelsCalculatorThreadDataV2* input = (FractalPixelsCalculatorThreadDataV2*)inputPointer;
	FractalPixelsCalculatorThreadDataV2 operationData = *input;
	delete input;

	bool allPointsProcessed = false;
	do
	{
		allPointsProcessed = operationData.fractalPixelsOutput->calculatePixels(
			operationData.processThread,
			operationData.numberOfPointsToProcess
		);		
	} while (!allPointsProcessed && *operationData.processThread);

	return 0;
}

void InitializeFractalPixelsCalculatorV2(FractalWindowData* fractalWindowData, unsigned short bitmapWidth, unsigned short bitmapHeight)
{
	fractalWindowData->fractalPixelsCalculatorV2 = std::shared_ptr < FractalPixelsV2>(new  FractalPixelsV2(
		fractalWindowData->currentFractalPoints,
		FractalPixelCalculatorGDI(
			fractalWindowData->fractal->getClipping(),
			BitmapDimensions(
				bitmapWidth,
				bitmapHeight
			)
		)
	));
}

void InitializeFractalPixelsThreadV2(FractalWindowData* fractalWindowData, unsigned int numberOfPointsToRender)
{
	if (fractalWindowData->processFractalPixelsThread != NULL)
	{
		*fractalWindowData->processFractalPixelsThread = false;
	}
	FractalPixelsCalculatorThreadDataV2* fractalPixelCalculatorData = new FractalPixelsCalculatorThreadDataV2{};
	fractalPixelCalculatorData->processThread = fractalWindowData->processFractalPixelsThread = std::shared_ptr<bool>(new bool{ true });
	fractalPixelCalculatorData->fractalPixelsOutput = fractalWindowData->fractalPixelsCalculatorV2;
	fractalPixelCalculatorData->numberOfPointsToProcess = numberOfPointsToRender;	
	CreateThread(
		NULL,
		0,
		FractalPixelsCalculatorThreadV2,
		fractalPixelCalculatorData,
		0,
		&fractalWindowData->calculateFractalPixelsThreadId
	);
}

void InitializeFractalBitmapGeneratorV2(FractalWindowData* fractalWindowData)
{
	fractalWindowData->currentFractalBitmapGeneratorV2 = std::shared_ptr<FractalBitmapFactoryV2>(new FractalBitmapFactoryV2(
		fractalWindowData->fractalPixelsCalculatorV2,
		fractalWindowData->numberOfPointsToProcess
	));
}

void InitializeFractalBitmapThreadV2(FractalWindowData* fractalWindowData, unsigned int numberOfPointsToRender)
{
	if (fractalWindowData->processFractalBitmapThread != NULL)
	{
		*fractalWindowData->processFractalBitmapThread = false;
	}
	MonochromaticBitmapThreadDataV2* fractalBitmapThreadData = new MonochromaticBitmapThreadDataV2{};
	fractalBitmapThreadData->numberOfPixelsToProcess = numberOfPointsToRender;
	fractalBitmapThreadData->processThread = fractalWindowData->processFractalBitmapThread = std::shared_ptr<bool>(new bool{ true });
	fractalBitmapThreadData->fractalBitmapFactory = fractalWindowData->currentFractalBitmapGeneratorV2;
	fractalBitmapThreadData->viewWindowHandle = fractalWindowData->windowHandle;
	fractalBitmapThreadData->viewBufferDC = fractalWindowData->fractalImage->deviceContext;
	HANDLE createFractalBitmapThreadHandle = CreateThread(
		NULL,
		0,
		MonochromaticBitmapThreadV2,
		fractalBitmapThreadData,
		0,
		&fractalWindowData->createFractalBitmapThreadId
	);
}

void InitializeFractalRenderV1(FractalWindowData* fractalWindowData, unsigned short bitmapWidth, unsigned short bitmapHeight)
{
	InitializeFractalPixelsCalculator(
		fractalWindowData,
		bitmapWidth,
		bitmapHeight
	);
	InitializeFractalPixelsThread(fractalWindowData);
	InitializeFractalBitmapGenerator(fractalWindowData);
	InitializeFractalBitmapThread(
		fractalWindowData,
		fractalWindowData->numberOfPointsToProcess
	);
}

void InitializeFractalRenderV2(FractalWindowData* fractalWindowData, unsigned short bitmapWidth, unsigned short bitmapHeight)
{
	InitializeFractalPixelsCalculatorV2(
		fractalWindowData,
		bitmapWidth,
		bitmapHeight
	);
	InitializeFractalPixelsThreadV2(
		fractalWindowData,
		fractalWindowData->numberOfPointsToProcess
	);
	InitializeFractalBitmapGeneratorV2(fractalWindowData);
	InitializeFractalBitmapThreadV2(
		fractalWindowData,
		fractalWindowData->numberOfPointsToProcess
	);
}

MessageProcessor::MessageProcessor(
	HACCEL hAccelTable,
	HWND dialogHandle,
	FractalFormDialogData* fractalDialogData
)
{
	this->hAccelTable = hAccelTable;
	this->dialogHandle = dialogHandle;
	this->fractalDialogData = fractalDialogData;
}

void MessageProcessor::ProcessMessage(MSG msg)
{
	bool isTranslated = TranslateAccelerator(msg.hwnd, hAccelTable, &msg);
	/*
	Wywołania IsDialogMessage są lipne, bo jakakolwiek zmiana w liście okien aplikacji wymaga dostosowania tego kodu.
	Przydałaby się drobna klasa do zarządzania oknami dialogowymi w ramach danego wątku.
	*/
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

RealTimeMessageLoop::RealTimeMessageLoop(
	MessageProcessor msgProcessor,
	FractalWindowData* fractalWindowData,
	HWND mainWindowHandle
) : msgProcessor(msgProcessor)
{
	this->fractalWindowData = fractalWindowData;
	this->mainWindowHandle = mainWindowHandle;
	this->lastPaintingTS = std::chrono::high_resolution_clock::now();
	this->framecapMS = 16;
}

int RealTimeMessageLoop::messageLoop(void)
{
	MSG msg;
	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{

			switch (msg.message)
			{
				case WM_QUIT:
					return (int)msg.wParam;
				case WM_KEYUP:
					{
						bool framecapChanged = false;
						switch (msg.wParam)
						{
							case VK_ADD:
								if (framecapMS < 50)
								{
									framecapMS++;
									framecapChanged = true;
								}
								break;
							case VK_SUBTRACT:
								if (framecapMS > 0)
								{
									framecapMS--;
									framecapChanged = true;
								}
								break;
						}
						if (framecapChanged)
						{
							std::wstringstream stream;
							stream << L"Nowy framecap - " << framecapMS << L"\n";
							OutputDebugStringW(stream.str().c_str());
						}
					}
					break;
			}

			this->msgProcessor.ProcessMessage(msg);
		}

		Sleep(1);
		//sprawdzanie, czy bitmapa fraktala została zaktualizowana w celu wywołania przerysowania okna
		if (fractalWindowData->currentFractalBitmapGenerator != NULL)
		{
			std::chrono::steady_clock::time_point currentTS = std::chrono::high_resolution_clock::now();
			long long noMillisecondsSinceLastPainting = std::chrono::duration_cast<std::chrono::milliseconds>(currentTS - lastPaintingTS).count();
			if (noMillisecondsSinceLastPainting > framecapMS)
			{
				lastPaintingTS = currentTS;
				if (fractalWindowData->currentFractalBitmapGenerator->copyIntoBuffer(
					fractalWindowData->fractalImage->deviceContext
				))
				{
					InvalidateRect(
						mainWindowHandle,
						NULL,
						FALSE
					);
				}
			}
		}

	}
}
