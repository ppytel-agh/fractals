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

void InputWrapper::setValueFromString(
	LPCWSTR newValue
)
{
	Edit_SetText(
		this->windowHandle,
		newValue
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

void InputWrapper::reset(void)
{
	setValueFromString(L"");
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

void FloatInput::setValue(float newValue)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << newValue;
	std::string outputString = stream.str();
	const char* cString = outputString.c_str();
	LPCWSTR floatString = (LPCWSTR)ansiToUnicode(outputString.c_str());
	setValueFromString(floatString);
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

AffineTransformation AffineTransformationForm::getValue(void)
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

void AffineTransformationForm::setValue(AffineTransformation newValue)
{
	a->setValue(newValue.getA());
	b->setValue(newValue.getB());
	c->setValue(newValue.getC());
	d->setValue(newValue.getD());
	e->setValue(newValue.getE());
	f->setValue(newValue.getF());
}

void AffineTransformationForm::reset(void)
{
	this->a->reset();
	this->b->reset();
	this->c->reset();
	this->d->reset();
	this->e->reset();
	this->f->reset();
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

void NaturalInput::setValue(int newValue)
{
	std::string valueString = std::to_string(newValue);
	const char* cString = valueString.c_str();
	LPCWSTR integerString = (LPCWSTR)ansiToUnicode(cString);
	setValueFromString(integerString);
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
}

FractalTransformationsRowForm::~FractalTransformationsRowForm()
{
	delete probability;
	delete affineTransformationForm;
}

bool FractalTransformationsRowForm::isFilled(void)
{
	return probability->getValue() != 0;
}

AffineTransformationRow FractalTransformationsRowForm::getValue(void)
{
	return AffineTransformationRow(
		this->probability->getValue(),
		this->affineTransformationForm->getValue()
	);
}

void FractalTransformationsRowForm::setValue(AffineTransformationRow newValue)
{
	this->probability->setValue(
		newValue.getProbability()
	);
	this->affineTransformationForm->setValue(
		newValue.getTransformation()
	);
}

void FractalTransformationsRowForm::reset(void)
{
	this->probability->reset();
	this->affineTransformationForm->reset();
}

unsigned short FractalTransformationsForm::getHeight()
{
	return factorsUpperLabelHeight + factorsHeight + (FractalTransformationsRowForm::height * 4);
}

unsigned short FractalTransformationsForm::getWidth()
{
	return probabilityLabelWidth + (6 * factorWidth);
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

AffineTransformationRowsGroup FractalTransformationsForm::getValue(void)
{
	unsigned char numberOfRows = 0;
	bool filledRows[maxNumberOfTransformations];
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		filledRows[i] = transformationRowForms[i]->isFilled();
		if (filledRows[i])
		{
			numberOfRows++;
		}
	}
	AffineTransformationRow* transformationRows = (AffineTransformationRow*)malloc(sizeof(AffineTransformationRow) * numberOfRows);
	unsigned char transformationRowIndex = 0;
	for (int i = 0; i < maxNumberOfTransformations; i++)
	{
		if (filledRows[i])
		{
			transformationRows[transformationRowIndex] = transformationRowForms[i]->getValue();
			transformationRowIndex++;
		}
	}
	AffineTransformationRowsGroup result(
		transformationRows,
		numberOfRows
	);
	free(transformationRows);
	return result;
}

void FractalTransformationsForm::setValue(AffineTransformationRowsGroup newValue)
{
	for (unsigned char i = 0; i < newValue.getNumberOfRows(); i++)
	{
		this->transformationRowForms[i]->setValue(
			newValue.getAffineTransformation(i)
		);
	}
	for (unsigned char i = newValue.getNumberOfRows(); i < maxNumberOfTransformations; i++)
	{
		this->transformationRowForms[i]->reset();
	}
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

bool FractalClippingForm::isValid(void)
{
	return minX->getFloatInput()->isValid();
}

FractalClipping FractalClippingForm::getValue(void)
{
	return FractalClipping(
		this->minX->getFloatInput()->GetValue(),
		this->maxX->getFloatInput()->GetValue(),
		this->minY->getFloatInput()->GetValue(),
		this->maxY->getFloatInput()->GetValue()
	);
}

void FractalClippingForm::setValue(FractalClipping newValue)
{
	this->minX->getFloatInput()->setValue(
		newValue.getXMin()
	);
	this->maxX->getFloatInput()->setValue(
		newValue.getXMax()
	);
	this->minY->getFloatInput()->setValue(
		newValue.getYMin()
	);
	this->maxY->getFloatInput()->setValue(
		newValue.getYMax()
	);
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

FractalDefinitionForm::FractalDefinitionForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	clipping = new FractalClippingForm(
		parent,
		offsetX,
		offsetY
	);
	unsigned short transformationsOffsetY = offsetY + FloatInputWithLeftLabel::height + transformationsAndClippingOffsetY;
	transformations = new FractalTransformationsForm(
		parent,
		offsetX,
		transformationsOffsetY
	);
	height = transformations->getHeight() + transformationsAndClippingOffsetY + FloatInputWithLeftLabel::height;
	width = transformations->getWidth();
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

unsigned short FractalDefinitionForm::getWidth(void)
{
	return width;
}

Fractal FractalDefinitionForm::getValue(void)
{
	return Fractal(
		this->transformations->getValue(),
		this->clipping->getValue()
	);
}

void FractalDefinitionForm::setValue(Fractal newValue)
{
	this->transformations->setValue(
		newValue.getTransformationRows()
	);
	this->clipping->setValue(
		newValue.getClipping()
	);
}

FractalDrawingUI::FractalDrawingUI(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	this->fractalDefinition = new FractalDefinitionForm(
		parent,
		offsetX,
		offsetY
	);
	unsigned short buttonOffsetX = offsetX + this->fractalDefinition->getTransformationsForm()->getWidth() - buttonWidth;
	unsigned short buttonOffsetY = offsetY + this->fractalDefinition->getHeight() + 5;
	this->renderFractalButton = new ButtonWrapper(
		parent,
		L"Renderuj",
		buttonOffsetX,
		buttonOffsetY,
		buttonWidth,
		buttonHeight
	);
	unsigned importButtonOffsetX = buttonOffsetX - 5 - importButtonWidth;
	this->importValuesFromPDFButton = new ButtonWrapper(
		parent,
		L"Importuj z PDF-a",
		importButtonOffsetX,
		buttonOffsetY,
		importButtonWidth,
		buttonHeight
	);
	height = this->fractalDefinition->getHeight() + buttonHeight + 5;
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

unsigned short FractalDrawingUI::getWidth(void)
{
	return this->fractalDefinition->getWidth();
}

ButtonWrapper* FractalDrawingUI::getRenderButton(void)
{
	return this->renderFractalButton;
}

FractalDefinitionForm* FractalDrawingUI::getFractalDefinitionForm(void)
{
	return fractalDefinition;
}

ButtonWrapper* FractalDrawingUI::getImportbutton(void)
{
	return importValuesFromPDFButton;
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

LPWSTR ansiToUnicode(const char* cString)
{
	size_t size = strlen(cString) + 1;
	LPWSTR unicodeBuffer = new WCHAR[size];
	size_t outSize;
	mbstowcs_s(&outSize, unicodeBuffer, size, cString, size - 1);
	return unicodeBuffer;
}
