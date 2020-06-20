#pragma once

#include <windef.h>
#include <WinUser.h>

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
	short offsetX;
	short offsetY;
	float scaleRatio;
	const static float maxScaleRatio;
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
	void moveRender(short x, short y);
	void resetOffset(void);
	void scale(short promilePoints, unsigned short referencePointX, unsigned short referencePointY);
};