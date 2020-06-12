#pragma once

#include <Windows.h>
#include "Fractals.h"
#include <windowsx.h>
#include <sstream>
#include <iomanip>
#include <string>

class InputWrapper
{
private:
	HWND windowHandle;
protected:
	void putValueIntoBuffer(
		const TCHAR* buffer,
		unsigned char bufferSize
	);
	void setValueFromString(
		LPCWSTR newValue
	);
public:
	InputWrapper(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height,
		bool isFirstElementOfNewGroup = false
	);
	~InputWrapper();
	void reset(void);
};

class FloatInput : public InputWrapper
{
	using InputWrapper::InputWrapper;
public:
	float GetValue(void);
	bool isValid(void);
	void setValue(float newValue);
};

class NaturalInput : public InputWrapper
{
	using InputWrapper::InputWrapper;
public:
	unsigned int getValue(void);
	void setValue(int newValue);
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
	AffineTransformation getValue(void);
	void setValue(AffineTransformation newValue);
	void reset(void);
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
	bool isFilled(void);
	AffineTransformationRow getValue(void);
	void setValue(AffineTransformationRow newValue);
	void reset(void);
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
	static const unsigned char maxNumberOfTransformations = 4;
	LabelWrapper* probabilityLabel;
	LabelWrapper* factorsLabel;
	LabelWrapper* aFactorLabel;
	LabelWrapper* bFactorLabel;
	LabelWrapper* cFactorLabel;
	LabelWrapper* dFactorLabel;
	LabelWrapper* eFactorLabel;
	LabelWrapper* fFactorLabel;
	FractalTransformationsRowForm* transformationRowForms[maxNumberOfTransformations];
public:
	unsigned short getHeight();
	unsigned short getWidth();
	FractalTransformationsForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalTransformationsForm();
	AffineTransformationRowsGroup getValue(void);
	void setValue(AffineTransformationRowsGroup newValue);
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
		unsigned char labelWidth,
		bool isFirstElementOfGroup = false
	);
	~FloatInputWithLeftLabel();
	unsigned char getWidth(void);
	FloatInput* getFloatInput(void);
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
	FractalClipping* clipping;
public:
	FractalClippingForm(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalClippingForm();
	unsigned short getWidth(void);
	bool isValid(void);
	FractalClipping getValue(void);
	void setValue(FractalClipping newValue);
};

class FractalDefinitionForm
{
private:
	static const unsigned char transformationsAndClippingOffsetY = 5;
	FractalTransformationsForm* transformations;
	FractalClippingForm* clipping;
	unsigned short height;
	unsigned short width;
	bool isFormValid();
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
	unsigned short getWidth(void);
	Fractal getValue(void);
	void setValue(Fractal newValue);
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
	bool isCommandFromControl(LPARAM wmCommandlParam);
};

class FractalDrawingUI
{
private:
	const unsigned char buttonWidth = 100;
	const unsigned char buttonHeight = 27;
	const unsigned char importButtonWidth = 200;
	FractalDefinitionForm* fractalDefinition;
	ButtonWrapper* renderFractalButton;
	unsigned short height;
	ButtonWrapper* importValuesFromPDFButton;
public:
	FractalDrawingUI(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY
	);
	~FractalDrawingUI();
	unsigned short getHeight(void);
	unsigned short getWidth(void);
	ButtonWrapper* getRenderButton(void);
	FractalDefinitionForm* getFractalDefinitionForm(void);
	ButtonWrapper* getImportbutton(void);
};

LPWSTR ansiToUnicode(const char* cString);