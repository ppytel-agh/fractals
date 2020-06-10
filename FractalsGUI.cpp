#include "FractalsGUI.h"

void InputWrapper::putValueIntoBuffer(
	const TCHAR* buffer,
	unsigned char bufferSize
)
{
	Edit_GetLine(
		this->windowHandle,
		NULL,
		buffer,
		bufferSize
	);
}

InputWrapper::InputWrapper(
	HWND parent,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned char width,
	unsigned char height,
	bool isFirstElementOfNewGroup
)
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	if (isFirstElementOfNewGroup)
	{
		style |= WS_GROUP;
	}
	this->windowHandle = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		L"Edit",
		NULL,
		style,
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
	TCHAR buffer[bufferSize];
	putValueIntoBuffer(
		buffer,
		bufferSize
	);
	return _wtof(buffer);
}

bool FloatInput::isValid(void)
{
	return GetValue();
}

void AffineTransformationForm::ResetTransformation(void)
{
	if (transformation != NULL)
	{
		delete transformation;
		transformation = NULL;
	}
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
	transformation = NULL;
}

AffineTransformationForm::~AffineTransformationForm()
{
	delete a;
	delete b;
	delete c;
	delete d;
	delete e;
	delete f;
	ResetTransformation();
}

void AffineTransformationForm::updateTransformation(void)
{
	ResetTransformation();
	transformation = new AffineTransformation(
		a->GetValue(),
		b->GetValue(),
		c->GetValue(),
		d->GetValue(),
		e->GetValue(),
		f->GetValue()
	);
}

AffineTransformation* AffineTransformationForm::getAffineTransformation(void)
{
	return transformation;
}

unsigned int NaturalInput::getValue(void)
{
	const unsigned char bufferSize = 16;
	TCHAR buffer[bufferSize];
	putValueIntoBuffer(
		buffer,
		bufferSize
	);
	return _wtoi(buffer);
}

void FractalTransformationsRowForm::ResetAffineTransformationRow(void)
{
	if (affineTransformationRow != NULL)
	{
		delete affineTransformationRow;
		affineTransformationRow = NULL;
	}
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
		FractalTransformationsRowForm::height,
		true
	);
	affineTransformationForm = new AffineTransformationForm(
		parent,
		offsetX + probabilityCellWidth,
		offsetY,
		factorCellWidth
	);
	affineTransformationRow = NULL;
}

FractalTransformationsRowForm::~FractalTransformationsRowForm()
{
	delete probability;
	delete affineTransformationForm;
	ResetAffineTransformationRow();
}

bool FractalTransformationsRowForm::isFilled(void)
{
	return probability->getValue() != 0;
}

void FractalTransformationsRowForm::updateAffineTransformationRow(void)
{
	ResetAffineTransformationRow();
	affineTransformationForm->updateTransformation();
	affineTransformationRow = new AffineTransformationRow(
		probability->getValue(),
		affineTransformationForm->getAffineTransformation()
	);
}

AffineTransformationRow* FractalTransformationsRowForm::getAffineTranformationRow(void)
{
	return affineTransformationRow;
}

//AffineTransformationRow FractalTransformationsRowForm::getValue(void)
//{
//	return AffineTransformationRow(
//		probability->getValue(),
//		&affineTransformationForm->GetValue()
//	);
//}

void FractalTransformationsForm::ResetTransformationRows(void)
{
	if (numberOfRows > 0)
	{
		delete[] transformationRows;
		transformationRows = NULL;
		numberOfRows = 0;
	}
}

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
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		transformationRowForms[i] = new FractalTransformationsRowForm(
			parent,
			offsetX,
			offsetY + factorsUpperLabelHeight + factorsHeight + (FractalTransformationsRowForm::height * i),
			probabilityLabelWidth,
			factorWidth
		);
	}
	transformationRows = NULL;
	numberOfRows = 0;
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
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		delete transformationRowForms[i];
	}
}

void FractalTransformationsForm::updateTransformationRows(void)
{
	ResetTransformationRows();
	bool filledRows[4];
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		filledRows[i] = transformationRowForms[i]->isFilled();
		if (filledRows[i])
		{
			numberOfRows++;
		}
	}
	transformationRows = new AffineTransformationRow * [numberOfRows];
	unsigned char transformationRowIndex = 0;
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		if (filledRows[i])
		{
			transformationRowForms[i]->updateAffineTransformationRow();
			transformationRows[transformationRowIndex] = transformationRowForms[i]->getAffineTranformationRow();
			transformationRowIndex++;
		}
	}
}

AffineTransformationRow** FractalTransformationsForm::getTransformationRows(void)
{
	return transformationRows;
}

unsigned char FractalTransformationsForm::getNumberOfTransformationRows(void)
{
	return numberOfRows;
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

void FractalClippingForm::ResetClipping(void)
{
	if (clipping != NULL)
	{
		delete clipping;
		clipping = NULL;
	}
}

FractalClippingForm::FractalClippingForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{	
	unsigned short elementOffsetX = offsetX;
	minX = new FloatInputWithLeftLabel(
		parent,
		L"min X",
		elementOffsetX,
		offsetY,
		labelsWidth,
		true
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
	clipping = NULL;
}

FractalClippingForm::~FractalClippingForm()
{
	delete minX;
	delete maxX;
	delete minY;
	delete maxY;
	ResetClipping();
}

unsigned short FractalClippingForm::getWidth(void)
{
	return width;
}

bool FractalClippingForm::isValid(void)
{
	return minX->getFloatInput()->isValid();
}

void FractalClippingForm::updateFractalClipping(void)
{
	clipping = new FractalClipping(
		minX->getFloatInput()->GetValue(),
		maxX->getFloatInput()->GetValue(),
		minY->getFloatInput()->GetValue(),
		maxY->getFloatInput()->GetValue()
	);
}

FractalClipping* FractalClippingForm::getFractalClipping(void)
{
	return clipping;
}

FloatInputWithLeftLabel::FloatInputWithLeftLabel(
	HWND parent,
	LPCTSTR text,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned char labelWidth,
	bool isFirstElementOfGroup
)
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
		height,
		isFirstElementOfGroup
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

FloatInput* FloatInputWithLeftLabel::getFloatInput(void)
{
	return input;
}

bool FractalDefinitionForm::isFormValid()
{
	return true;
}

void FractalDefinitionForm::ResetFractal(void)
{
	if (fractal != NULL)
	{
		delete fractal;
		fractal = NULL;
	}
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
	fractal = NULL;
}

FractalDefinitionForm::~FractalDefinitionForm()
{
	delete transformations;
	delete clipping;
	ResetFractal();
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

Fractal* FractalDefinitionForm::getFractal(void)
{
	return fractal;	
}

void FractalDefinitionForm::UpdateFractal(void)
{
	ResetFractal();
	transformations->updateTransformationRows();
	clipping->updateFractalClipping();
	fractal = new Fractal(
		transformations->getTransformationRows(),
		transformations->getNumberOfTransformationRows(),
		clipping->getFractalClipping()
	);
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

ButtonWrapper* FractalDrawingUI::getRenderButton(void)
{
	return this->renderFractalButton;
}

FractalDefinitionForm* FractalDrawingUI::getFractalDefinitionForm(void)
{
	return fractalDefinition;
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

bool ButtonWrapper::isCommandFromControl(LPARAM wmCommandlParam)
{
	return (HWND)wmCommandlParam == this->buttonWindow;
}
