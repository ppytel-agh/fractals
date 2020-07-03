#pragma once

#include <Windows.h>
#include "Fractals.h"
#include <windowsx.h>
#include "string-processing.h"

//nagłówki potrzebne do tooltipów
#include <CommCtrl.h>
#pragma comment( lib, "comctl32.lib")
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")


class InputWrapper
{
private:
	HWND windowHandle;
	LPWSTR inputBuffer;
	unsigned char inputLength;
	bool valueIsChangedViaSetter;
protected:
	void setValueFromString(
		LPCWSTR newValue
	);
	void updateInputBuffer(void);
	LPWSTR getInputBuffer(void);
	unsigned char getInputLength(void);
	HWND getInputWindowHandle(void);
	bool isValueChangedViaSetter(void);
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
	bool isEmpty(void);
	void displayError(LPCWSTR message);
};

class FloatInput : public InputWrapper
{
public:
	FloatInput(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height,
		bool isFirstElementOfNewGroup = false
	) : InputWrapper(parent, offsetX, offsetY, width, height, isFirstElementOfNewGroup) {};
	float GetValue(void);
	void setValue(float newValue);
	bool isValid(void);
};


class FloatInputWithStepping : public FloatInput
{
private:
	HWND upDownWindowHandle;
	char min;
	char max;
	void updateUpDownPos(short newValue);
	void updateFloatInput(void);
public:
	FloatInputWithStepping(
		HWND parent,
		unsigned short offsetX,
		unsigned short offsetY,
		unsigned char width,
		unsigned char height,
		char min,
		char max,
		bool isFirstElementOfNewGroup = false
	) : FloatInput(parent, offsetX, offsetY, width, height, isFirstElementOfNewGroup) {
		this->upDownWindowHandle = CreateWindowExW(
			WS_EX_LEFT | WS_EX_LTRREADING,
			UPDOWN_CLASS,
			NULL,
			WS_CHILDWINDOW | WS_VISIBLE
			| UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
			0, 0,
			0, 0,         // Set to zero to automatically size to fit the buddy window.
			parent,
			NULL,
			(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
			NULL
		);
		this->min = min;
		this->max = max;
		short maxPos = this->max * 100;
		short minPos = this->min * 100;
		SendMessageW(upDownWindowHandle, UDM_SETRANGE, 0, MAKELPARAM(maxPos, minPos));
	};
	void processChange(const NMUPDOWN* upDownMessage);
	void processInputChange(const HWND changedInputWindowHandle);
	bool isValid(void);
	void setValue(float newValue);
	float GetValue(void);
};

class NaturalInput : public InputWrapper
{
	using InputWrapper::InputWrapper;
public:
	unsigned int getValue(void);
	void setValue(int newValue);
	bool isValid(void);
};

class AffineTransformationForm
{
private:
	static const unsigned char numberOfParams = 6;
	FloatInputWithStepping* params[numberOfParams] = {};
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
	bool isValid(void);
	void processUpDownNotification(const NMUPDOWN* upDownMessage);
	void processInputChange(const HWND changedInputWindowHandle);
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
	bool isValid(void);
	bool isEmpty(void);
	void displayError(LPCWSTR message);
	void processUpDownNotification(const NMUPDOWN* upDownMessage);
	void processInputChange(const HWND changedInputWindowHandle);
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
	static const unsigned char factorWidth = 60;
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
	bool isValid(void);
	void processUpDownNotification(const NMUPDOWN* upDownMessage);
	void processInputChange(const HWND changedInputWindowHandle);
};

class FloatInputWithLeftLabel
{
private:
	LabelWrapper* label;
	FloatInputWithStepping* input;
	unsigned char width;
public:
	static const unsigned char inputWidth = 60;
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
	FloatInputWithStepping* getFloatInput(void);
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
	FractalClipping getValue(void);
	void setValue(FractalClipping newValue);
	bool isValid(void);
	void processUpDownNotification(const NMUPDOWN* upDownMessage);
	void processInputChange(const HWND changedInputWindowHandle);
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
	bool isValid(void);
	void processNotification(const NMHDR* message);
	void processControlCommand(
		WORD notificationCode,
		HWND controlWindowHandle
	);
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
	const unsigned char layersOffsetY = 5;
	const unsigned char numberOfPointsControlWidth = 75;
	const unsigned int maxNumberOfPointsToRender = 1000000;
	FractalDefinitionForm* fractalDefinition;
	ButtonWrapper* renderFractalButton;
	unsigned short height;
	ButtonWrapper* importValuesFromPDFButton;
	NaturalInput* numberOfPointsToRender;
	LabelWrapper* numberOfPointsToRenderLabel;
	HWND numberOfPointsUpDownHandle;
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
	NaturalInput* getNumberOfPointsToRender(void);
};

LPWSTR ansiToUnicode(const char* cString);