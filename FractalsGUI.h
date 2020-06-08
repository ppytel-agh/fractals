#pragma once

#include <Windows.h>
#include "Fractals.h"
#include <windowsx.h>

class InputWrapper
{
private:
	HWND windowHandle;
protected:
	void putValueIntoBuffer(
		const char* buffer,
		unsigned char bufferSize
	);
public:
	InputWrapper(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height
	);
	~InputWrapper();
};

class FloatInput : public InputWrapper
{
	using InputWrapper::InputWrapper;
public:
	float GetValue(void);
};

class NaturalInput : public InputWrapper
{
	using InputWrapper::InputWrapper;
public:
	unsigned int getValue(void);
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
	AffineTransformation GetValue(void);
};

class FractalTransformationsRowForm
{
private:
	NaturalInput* probability;
	AffineTransformationForm* affineTransformationForm;
public:
	FractalTransformationsRowForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalTransformationsRowForm();
	AffineTransformationRow getValue(void);
};

class FractalTransformationsForm
{
private:
	HWND probabilityLabel;
	HWND factorsLabel;
	HWND aFactorLabel;
	HWND bFactorLabel;
	HWND cFactorLabel;
	HWND dFactorLabel;
	HWND eFactorLabel;
	HWND fFactorLabel;
	FractalTransformationsRowForm* ftr1;
	FractalTransformationsRowForm* ftr2;
	FractalTransformationsRowForm* ftr3;
	FractalTransformationsRowForm* ftr4;
};

class FractalClippingForm
{
private:
	HWND minXLabel;
	HWND maxXLabel;
	HWND minYLabel;
	HWND maxYLabel;
	FloatInput* minX;
	FloatInput* maxX;
	FloatInput* minY;
	FloatInput* maxY;
};

class FractalDefinitionForm
{
private:
	FractalTransformationsForm* transformations;
	FractalClippingForm* clipping;
};

class FractalDrawingUI
{
private:
	FractalDefinitionForm* fractalDefinition;
};