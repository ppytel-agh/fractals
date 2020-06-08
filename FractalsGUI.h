#pragma once

#include <Windows.h>
#include "Fractals.h"
#include <windowsx.h>

class FloatInput
{
private:
	HWND hWnd;
public:

	FloatInput(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height
	);
	~FloatInput();
	float GetValue(void);
};

class AffineTransformationForm
{
private:
	FloatInput* a;
	FloatInput* b;
	FloatInput* c;
	FloatInput* d;
	FloatInput* e;
	FloatInput* f;
public:
	AffineTransformationForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~AffineTransformationForm();
	void GetValue(AffineTransformation& output);
};