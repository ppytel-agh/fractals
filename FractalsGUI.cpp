#include "FractalsGUI.h"

void InputWrapper::putValueIntoBuffer(const char* buffer, unsigned char bufferSize)
{
	Edit_GetLine(
		this->windowHandle,
		NULL,
		buffer,
		bufferSize
	);
}

InputWrapper::InputWrapper(HWND parent, unsigned short offsetX, unsigned short offsetY, unsigned char width, unsigned char height)
{
	this->windowHandle = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"Edit",
		NULL,
		WS_CHILD | WS_VISIBLE,
		(int)offsetX,
		(int)offsetY,
		(int)width,
		(int)height,
		parent,
		NULL,
		(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
		NULL
	);
}

InputWrapper::~InputWrapper()
{
	DestroyWindow(this->windowHandle);
}

float FloatInput::GetValue(void)
{
	const unsigned char bufferSize = 16;
	char buffer[bufferSize];
	putValueIntoBuffer(
		buffer,
		bufferSize
	);
	return (float)atof(buffer);
}

AffineTransformationForm::AffineTransformationForm(
	HWND parent,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned char factorCellWidth
)
{
	const unsigned char inputsOffset = 50;
	const unsigned char elementHeight = 20;
	const unsigned char margin = 2;
	unsigned char doubleMargin = 2 * margin;
	unsigned char elementWidth = factorCellWidth - doubleMargin;
	unsigned char elementOffsetY = offsetY + margin;
	unsigned short elementOffsetX = offsetX + margin;
	a = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
	elementOffsetX += elementWidth + doubleMargin;
	b = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
	elementOffsetX += elementWidth + doubleMargin;
	c = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
	elementOffsetX += elementWidth + doubleMargin;
	d = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
	elementOffsetX += elementWidth + doubleMargin;
	e = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
	elementOffsetX += elementWidth + doubleMargin;
	f = new FloatInput(
		parent,
		elementOffsetX,
		elementOffsetY,
		elementWidth,
		elementHeight
	);
}

AffineTransformationForm::~AffineTransformationForm()
{
	delete a;
	delete b;
	delete c;
	delete d;
	delete e;
	delete f;
}

AffineTransformation AffineTransformationForm::GetValue(void)
{
	return AffineTransformation(
		a->GetValue(),
		b->GetValue(),
		c->GetValue(),
		d->GetValue(),
		e->GetValue(),
		f->GetValue()
	);
}

unsigned int NaturalInput::getValue(void)
{
	const unsigned char bufferSize = 16;
	char buffer[bufferSize];
	putValueIntoBuffer(
		buffer,
		bufferSize
	);
	return atoi(buffer);
}

FractalTransformationsRowForm::FractalTransformationsRowForm(
	HWND parent,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned char probabilityCellWidth,
	unsigned char factorCellWidth
)
{
	probability = new NaturalInput(
		parent,
		offsetX + 2,
		offsetY + 2,
		probabilityCellWidth - 4,
		FractalTransformationsRowForm::height
	);
	affineTransformationForm = new AffineTransformationForm(
		parent,
		offsetX + probabilityCellWidth,
		offsetY,
		factorCellWidth
	);
}

FractalTransformationsRowForm::~FractalTransformationsRowForm()
{
	delete probability;
	delete affineTransformationForm;
}

//AffineTransformationRow FractalTransformationsRowForm::getValue(void)
//{
//	return AffineTransformationRow(
//		probability->getValue(),
//		&affineTransformationForm->GetValue()
//	);
//}

unsigned short FractalTransformationsForm::getHeight()
{
	return factorsUpperLabelHeight + factorsHeight + (FractalTransformationsRowForm::height * 4);
}

FractalTransformationsForm::FractalTransformationsForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{	
	this->probabilityLabel = new LabelWrapper(
		parent,
		L"prawdobodobieństwo",
		offsetX,
		offsetY,
		probabilityLabelWidth,
		factorsUpperLabelHeight + factorsHeight
	);
	this->factorsLabel = new LabelWrapper(
		parent,
		L"współczynniki przekształceń afinicznych",
		offsetX + probabilityLabelWidth,
		offsetY,
		6 * factorWidth,
		factorsUpperLabelHeight
	);
	this->aFactorLabel = new LabelWrapper(
		parent,
		L"a",
		offsetX + probabilityLabelWidth,
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->bFactorLabel = new LabelWrapper(
		parent,
		L"b",
		offsetX + probabilityLabelWidth + factorWidth,
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->cFactorLabel = new LabelWrapper(
		parent,
		L"c",
		offsetX + probabilityLabelWidth + (factorWidth * 2),
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->dFactorLabel = new LabelWrapper(
		parent,
		L"d",
		offsetX + probabilityLabelWidth + (factorWidth * 3),
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->eFactorLabel = new LabelWrapper(
		parent,
		L"e",
		offsetX + probabilityLabelWidth + (factorWidth * 4),
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->fFactorLabel = new LabelWrapper(
		parent,
		L"f",
		offsetX + probabilityLabelWidth + (factorWidth * 5),
		offsetY + factorsUpperLabelHeight,
		factorWidth,
		factorsHeight,
		LabelHorizontalAlignment::center
	);
	this->ftr1 = new FractalTransformationsRowForm(
		parent,
		offsetX,
		offsetY + factorsUpperLabelHeight + factorsHeight,
		probabilityLabelWidth,
		factorWidth
	);
	this->ftr2 = new FractalTransformationsRowForm(
		parent,
		offsetX,
		offsetY + factorsUpperLabelHeight + factorsHeight + FractalTransformationsRowForm::height,
		probabilityLabelWidth,
		factorWidth
	);
	this->ftr3 = new FractalTransformationsRowForm(
		parent,
		offsetX,
		offsetY + factorsUpperLabelHeight + factorsHeight + (FractalTransformationsRowForm::height * 2),
		probabilityLabelWidth,
		factorWidth
	);
	this->ftr4 = new FractalTransformationsRowForm(
		parent,
		offsetX,
		offsetY + factorsUpperLabelHeight + factorsHeight + (FractalTransformationsRowForm::height * 3),
		probabilityLabelWidth,
		factorWidth
	);
}

FractalTransformationsForm::~FractalTransformationsForm()
{
	delete this->probabilityLabel;
	delete this->factorsLabel;
	delete this->aFactorLabel;
	delete this->bFactorLabel;
	delete this->cFactorLabel;
	delete this->dFactorLabel;
	delete this->eFactorLabel;
	delete this->fFactorLabel;
	delete this->ftr1;
	delete this->ftr2;
	delete this->ftr3;
	delete this->ftr4;
}

LabelWrapper::LabelWrapper(
	HWND parent,
	LPCTSTR text,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned short width,
	unsigned short height,
	LabelHorizontalAlignment alignment
)
{
	this->labelWindow = CreateWindowExW(
		NULL,
		L"Static",
		text,
		WS_CHILD | WS_VISIBLE | alignment,
		offsetX,
		offsetY,
		width,
		height,
		parent,
		NULL,
		(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
		NULL
	);
}

LabelWrapper::~LabelWrapper()
{
	DestroyWindow(this->labelWindow);
}

FractalClippingForm::FractalClippingForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{	
	unsigned short elementOffsetX = offsetX;
	minX = new FloatInputWithLeftLabel(
		parent,
		L"min X",
		elementOffsetX,
		offsetY,
		labelsWidth
	);
	elementOffsetX += minX->getWidth();
	maxX = new FloatInputWithLeftLabel(
		parent,
		L"max X",
		elementOffsetX,
		offsetY,
		labelsWidth
	);
	elementOffsetX += maxX->getWidth();
	minY = new FloatInputWithLeftLabel(
		parent,
		L"min Y",
		elementOffsetX,
		offsetY,
		labelsWidth
	);
	elementOffsetX += minY->getWidth();
	maxY = new FloatInputWithLeftLabel(
		parent,
		L"max Y",
		elementOffsetX,
		offsetY,
		labelsWidth
	);
	width = elementOffsetX + maxY->getWidth();
}

FractalClippingForm::~FractalClippingForm()
{
	delete minX;
	delete maxX;
	delete minY;
	delete maxY;
}

unsigned short FractalClippingForm::getWidth(void)
{
	return width;
}

FloatInputWithLeftLabel::FloatInputWithLeftLabel(HWND parent, LPCTSTR text, unsigned short offsetX, unsigned short offsetY, unsigned char labelWidth)
{	
	label = new LabelWrapper(
		parent,
		text,
		offsetX,
		offsetY,
		labelWidth,
		height,
		LabelHorizontalAlignment::right
	);
	input = new FloatInput(
		parent,
		offsetX + labelWidth,
		offsetY,
		inputWidth,
		height
	);
	width = labelWidth + inputWidth;
}

FloatInputWithLeftLabel::~FloatInputWithLeftLabel()
{
	delete label;
	delete input;
}

unsigned char FloatInputWithLeftLabel::getWidth(void)
{
	return width;
}

FractalDefinitionForm::FractalDefinitionForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	transformations = new FractalTransformationsForm(
		parent,
		offsetX,
		offsetY
	);
	unsigned short clippingFormOffsetY = offsetY + transformations->getHeight() + transformationsAndClippingOffsetY;
	clipping = new FractalClippingForm(
		parent,
		offsetX,
		clippingFormOffsetY
	);
	height = transformations->getHeight() + transformationsAndClippingOffsetY + FloatInputWithLeftLabel::height;
}

FractalDefinitionForm::~FractalDefinitionForm()
{
	delete transformations;
	delete clipping;
}

FractalTransformationsForm* FractalDefinitionForm::getTransformationsForm()
{
	return this->transformations;
}

FractalClippingForm* FractalDefinitionForm::getClippingForm()
{
	return this->clipping;
}

unsigned short FractalDefinitionForm::getHeight(void)
{
	return height;
}

FractalDrawingUI::FractalDrawingUI(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	this->fractalDefinition = new FractalDefinitionForm(
		parent,
		offsetX,
		offsetY
	);
	unsigned short buttonOffsetX = offsetX + this->fractalDefinition->getClippingForm()->getWidth() + 2;
	unsigned short buttonOffsetY = offsetY + this->fractalDefinition->getTransformationsForm()->getHeight() + 2;
	this->renderFractalButton = new ButtonWrapper(
		parent,
		L"Renderuj",
		buttonOffsetX,
		buttonOffsetY,
		buttonWidth,
		buttonHeight
	);
	height = this->fractalDefinition->getHeight() + 2;
}

FractalDrawingUI::~FractalDrawingUI()
{
	delete fractalDefinition;
	delete renderFractalButton;
}

unsigned short FractalDrawingUI::getHeight(void)
{
	return height;
}

ButtonWrapper::ButtonWrapper(HWND parent, LPCTSTR label, unsigned short offsetX, unsigned short offsetY, unsigned char width, unsigned char height)
{
	this->buttonWindow = CreateWindowExW(
		NULL,
		L"Button",
		label,
		WS_CHILD | WS_VISIBLE,
		offsetX,
		offsetY,
		width,
		height,
		parent,
		NULL,
		(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
		NULL
	);
}

ButtonWrapper::~ButtonWrapper()
{
	DestroyWindow(buttonWindow);
}
