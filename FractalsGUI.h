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
		unsigned short offsetY,
		unsigned char factorCellWidth
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
	static const unsigned char height = 20;
	FractalTransformationsRowForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char probabilityCellWidth,
		unsigned char factorCellWidth
	);
	~FractalTransformationsRowForm();
	//AffineTransformationRow getValue(void);
};

enum LabelHorizontalAlignment
{
	left = SS_LEFT,
	center = SS_CENTER,
	right = SS_RIGHT
};

class LabelWrapper
{
private:
	HWND labelWindow;
public:
	LabelWrapper(
		HWND parent,
		LPCTSTR text,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned short width,
		unsigned short height,
		LabelHorizontalAlignment alignment = LabelHorizontalAlignment::left
	);
	~LabelWrapper();
};

class FractalTransformationsForm
{
private:
	static const unsigned char probabilityLabelWidth = 150;
	static const unsigned char factorsUpperLabelHeight = 25;
	static const unsigned char factorsHeight = 20;
	static const unsigned char factorWidth = 50;
	LabelWrapper* probabilityLabel;
	LabelWrapper* factorsLabel;
	LabelWrapper* aFactorLabel;
	LabelWrapper* bFactorLabel;
	LabelWrapper* cFactorLabel;
	LabelWrapper* dFactorLabel;
	LabelWrapper* eFactorLabel;
	LabelWrapper* fFactorLabel;
	FractalTransformationsRowForm* ftr1;
	FractalTransformationsRowForm* ftr2;
	FractalTransformationsRowForm* ftr3;
	FractalTransformationsRowForm* ftr4;
public:
	unsigned short getHeight();
	FractalTransformationsForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalTransformationsForm();
};

class FloatInputWithLeftLabel
{
private:
	LabelWrapper* label;
	FloatInput* input;
	unsigned char width;
public:
	static const unsigned char inputWidth = 50;
	static const unsigned char height = 25;
	FloatInputWithLeftLabel(
		HWND parent,
		LPCTSTR text,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char labelWidth
	);
	~FloatInputWithLeftLabel();
	unsigned char getWidth(void);
};

class FractalClippingForm
{
private:
	static const unsigned char labelsWidth = 45;
	FloatInputWithLeftLabel* minX;
	FloatInputWithLeftLabel* maxX;
	FloatInputWithLeftLabel* minY;
	FloatInputWithLeftLabel* maxY;
	unsigned short width;
public:
	FractalClippingForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalClippingForm();
	unsigned short getWidth(void);
};

class FractalDefinitionForm
{
private:
	static const unsigned char transformationsAndClippingOffsetY = 5;
	FractalTransformationsForm* transformations;
	FractalClippingForm* clipping;
	unsigned short height;
public:
	FractalDefinitionForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalDefinitionForm();
	FractalTransformationsForm* getTransformationsForm();
	FractalClippingForm* getClippingForm();
	unsigned short getHeight(void);
};

//class RenderingFrameSizeForm
//{
//private:
//
//};
//
class ButtonWrapper
{
private:
	HWND buttonWindow;
public:
	ButtonWrapper(
		HWND parent,
		LPCTSTR label,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height
	);
	~ButtonWrapper();
};

class FractalDrawingUI
{
private:
	const unsigned char buttonWidth = 100;
	const unsigned char buttonHeight = 27;
	FractalDefinitionForm* fractalDefinition;
	ButtonWrapper* renderFractalButton;
	unsigned short height;
public:
	FractalDrawingUI(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalDrawingUI();
	unsigned short getHeight(void);
};