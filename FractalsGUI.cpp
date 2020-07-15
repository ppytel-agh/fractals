#include "FractalsGUI.h"

void InputWrapper::setValueFromString(
	LPCWSTR newValue
)
{
	this->valueIsChangedViaSetter = true;
	SetWindowTextW(
		this->windowHandle,
		newValue
	);
	this->valueIsChangedViaSetter = false;
}

void InputWrapper::updateInputBuffer(void)
{
	if (this->inputBuffer != NULL)
	{
		delete inputBuffer;
	}
	this->inputLength = GetWindowTextLengthW(this->windowHandle);
	unsigned char bufferLength = this->inputLength + 1;
	this->inputBuffer = new WCHAR[bufferLength];
	GetWindowTextW(
		this->windowHandle,
		this->inputBuffer,
		bufferLength
	);
}

LPWSTR InputWrapper::getInputBuffer(void)
{
	return this->inputBuffer;
}

unsigned char InputWrapper::getInputLength(void)
{
	return this->inputLength;
}

HWND InputWrapper::getInputWindowHandle(void)
{
	return this->windowHandle;
}

bool InputWrapper::isValueChangedViaSetter(void)
{
	return valueIsChangedViaSetter;
}

void InputWrapper::displayError(LPCWSTR message)
{
	EDITBALLOONTIP ebt;
	ebt.cbStruct = sizeof(EDITBALLOONTIP);
	ebt.pszText = L"Błąd!";
	ebt.pszTitle = message;
	ebt.ttiIcon = TTI_ERROR_LARGE;    // tooltip icon
	SendMessageW(
		this->windowHandle,
		EM_SHOWBALLOONTIP,
		0,
		(LPARAM)&ebt
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
	this->inputBuffer = NULL;
	this->inputLength = 0;
}

InputWrapper::~InputWrapper()
{
	DestroyWindow(this->windowHandle);
}

void InputWrapper::reset(void)
{
	setValueFromString(L"");
}

bool InputWrapper::isEmpty(void)
{
	this->updateInputBuffer();
	return this->getInputLength() == 0;
}

float FloatInput::GetValue(void)
{
	this->updateInputBuffer();
	return _wtof(
		this->getInputBuffer()
	);
}

bool FloatInput::isValid(void)
{
	float providedValue = GetValue();
	LPWSTR inputBuffer = getInputBuffer();
	LPWSTR reconvertedFloat = floatToString(providedValue);
	int comparisonResult = wcscmp(inputBuffer, reconvertedFloat);
	if (comparisonResult == 0)
	{
		return true;
	}
	else
	{
		displayError(L"podana wartość nie jest liczbą zmiennoprzecinkową, wymagany format X.YY");
		return false;
	}
}

void FloatInput::setValue(float newValue)
{
	LPCWSTR floatString = (LPCWSTR)floatToString(newValue);
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
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		params[i] = new FloatInputWithStepping(
			parent,
			elementOffsetX,
			elementOffsetY,
			elementWidth,
			elementHeight,
			-10,
			10
		);
		elementOffsetX += elementWidth + doubleMargin;
	}
}

AffineTransformationForm::~AffineTransformationForm()
{
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		delete params[i];
	}
}

AffineTransformation AffineTransformationForm::getValue(void)
{
	return AffineTransformation(
		params[0]->GetValue(),
		params[1]->GetValue(),
		params[2]->GetValue(),
		params[3]->GetValue(),
		params[4]->GetValue(),
		params[5]->GetValue()
	);
}

void AffineTransformationForm::setValue(AffineTransformation newValue)
{
	params[0]->setValue(newValue.getA());
	params[1]->setValue(newValue.getB());
	params[2]->setValue(newValue.getC());
	params[3]->setValue(newValue.getD());
	params[4]->setValue(newValue.getE());
	params[5]->setValue(newValue.getF());
}

void AffineTransformationForm::reset(void)
{
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		params[i]->reset();
	}
}

bool AffineTransformationForm::isValid(void)
{
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		if (!params[i]->isValid())
		{
			return false;
		}
	}
	return true;
}

void AffineTransformationForm::processUpDownNotification(const NMUPDOWN* upDownMessage)
{
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		params[i]->processChange(upDownMessage);
	}
}

void AffineTransformationForm::processInputChange(const HWND changedInputWindowHandle)
{
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		params[i]->processInputChange(changedInputWindowHandle);
	}
}

unsigned int NaturalInput::getValue(void)
{
	this->updateInputBuffer();
	return _wtoi(
		this->getInputBuffer()
	);
}

void NaturalInput::setValue(int newValue)
{
	LPCWSTR integerString = (LPCWSTR)integerToString(newValue);
	setValueFromString(integerString);
}

bool NaturalInput::isValid(void)
{
	int providedValue = getValue();
	LPWSTR inputBuffer = getInputBuffer();
	LPWSTR reconvertedInt = integerToString(providedValue);
	int comparisonResult = wcscmp(inputBuffer, reconvertedInt);
	if (comparisonResult == 0)
	{
		if (providedValue > 0)
		{
			return true;
		}
		else
		{
			displayError(L"wartość musi być większa od zera");
			return false;
		}
	}
	else
	{
		displayError(L"podana wartość nie jest liczbą całkowitą");
		return false;
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

bool FractalTransformationsRowForm::isValid(void)
{
	if (this->isEmpty())
	{
		return true;
	}
	else
	{
		if (this->probability->isValid())
		{
			return this->affineTransformationForm->isValid();
		}
		else
		{
			return false;
		}
	}
}

bool FractalTransformationsRowForm::isEmpty(void)
{
	return this->probability->isEmpty();
}

void FractalTransformationsRowForm::displayError(LPCWSTR message)
{
	this->probability->displayError(message);
}

void FractalTransformationsRowForm::processUpDownNotification(const NMUPDOWN* upDownMessage)
{
	this->affineTransformationForm->processUpDownNotification(upDownMessage);
}

void FractalTransformationsRowForm::processInputChange(const HWND changedInputWindowHandle)
{
	this->affineTransformationForm->processInputChange(changedInputWindowHandle);
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

bool FractalTransformationsForm::isValid(void)
{
	for (unsigned char i = 0; i < maxNumberOfTransformations; i++)
	{
		if (!this->transformationRowForms[i]->isValid())
		{
			return false;
		}
	}
	unsigned char probabilitiesSum = 0;
	for (unsigned char i = 0; i < maxNumberOfTransformations; i++)
	{
		if (!this->transformationRowForms[i]->isEmpty())
		{
			probabilitiesSum += this->transformationRowForms[i]->getValue().getProbability();
		}
	}
	if (probabilitiesSum == 100)
	{
		return true;
	}
	else
	{
		this->transformationRowForms[0]->displayError(L"Suma prawdopodobieństw musi być równa 100");
	}
}

void FractalTransformationsForm::processUpDownNotification(const NMUPDOWN* upDownMessage)
{
	for (unsigned char i = 0; i < maxNumberOfTransformations; i++)
	{
		this->transformationRowForms[i]->processUpDownNotification(upDownMessage);
	}
}

void FractalTransformationsForm::processInputChange(const HWND changedInputWindowHandle)
{
	for (unsigned char i = 0; i < maxNumberOfTransformations; i++)
	{
		this->transformationRowForms[i]->processInputChange(changedInputWindowHandle);
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
	FloatInputWithLeftLabel* params[] = {
		minX,
		maxX,
		minY,
		maxY
	};
	unsigned char numberOfParams = sizeof(params) / sizeof(FloatInputWithLeftLabel*);
	for (unsigned char i = 0; i < numberOfParams; i++)
	{
		if (!params[i]->getFloatInput()->isValid())
		{
			return false;
		}
	}
	return true;
}

void FractalClippingForm::processUpDownNotification(const NMUPDOWN* upDownMessage)
{
	this->minX->getFloatInput()->processChange(upDownMessage);
	this->maxX->getFloatInput()->processChange(upDownMessage);
	this->minY->getFloatInput()->processChange(upDownMessage);
	this->maxY->getFloatInput()->processChange(upDownMessage);
}

void FractalClippingForm::processInputChange(const HWND changedInputWindowHandle)
{
	this->minX->getFloatInput()->processInputChange(changedInputWindowHandle);
	this->maxX->getFloatInput()->processInputChange(changedInputWindowHandle);
	this->minY->getFloatInput()->processInputChange(changedInputWindowHandle);
	this->maxY->getFloatInput()->processInputChange(changedInputWindowHandle);
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
	input = new FloatInputWithStepping(
		parent,
		offsetX + labelWidth,
		offsetY,
		inputWidth,
		height,
		-64,
		64,
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

FloatInputWithStepping* FloatInputWithLeftLabel::getFloatInput(void)
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

bool FractalDefinitionForm::isValid(void)
{
	if (!this->clipping->isValid())
	{
		return false;
	}
	if (!this->transformations->isValid())
	{
		return false;
	}
	return true;
}

FractalDrawingUI::FractalDrawingUI(HWND parent, unsigned short offsetX, unsigned short offsetY)
{	
	this->parentWindow = parent;

	this->fractalDefinition = new FractalDefinitionForm(
		parent,
		offsetX,
		offsetY
	);

	//liczba punktów do zrenderowania
	unsigned short numberOfPointsOffsetY = offsetY + this->fractalDefinition->getHeight() + this->layersOffsetY;

	const WCHAR numberOfPointsToRenderLabelText[] = L"Liczba punktów do zrenderowania";
	SIZE numberOfPointsTextSize = {};
	HDC parentDC = GetDC(parent);
	GetTextExtentPoint32A(
		parentDC,
		(LPCSTR)numberOfPointsToRenderLabelText,
		wcslen(numberOfPointsToRenderLabelText),
		&numberOfPointsTextSize
	);	
	ReleaseDC(parent, parentDC);
	unsigned short numberOfPointsLabelWidth = numberOfPointsTextSize.cx + 10;
	unsigned short numberOfPointsHeight = numberOfPointsTextSize.cy + 5;
	this->numberOfPointsToRenderLabel = new LabelWrapper(
		parent,
		numberOfPointsToRenderLabelText,
		offsetX,
		numberOfPointsOffsetY,
		numberOfPointsLabelWidth,
		numberOfPointsHeight,
		LabelHorizontalAlignment::center
	);
	
	unsigned short numberOfPointsControlOffsetX = numberOfPointsLabelWidth + 5;
	this->numberOfPointsToRender = new NaturalInput(
		parent,
		numberOfPointsControlOffsetX,
		numberOfPointsOffsetY,
		this->numberOfPointsControlWidth,
		numberOfPointsHeight
	);
	//this->numberOfPointsUpDownHandle = CreateWindowExW(
	//	WS_EX_LEFT | WS_EX_LTRREADING,
	//	UPDOWN_CLASS,
	//	NULL,
	//	WS_CHILDWINDOW | WS_VISIBLE
	//	| UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
	//	0, 0,
	//	0, 0,         // Set to zero to automatically size to fit the buddy window.
	//	this->numberOfPointsToRender->,
	//	NULL,
	//	(HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
	//	NULL
	//);
	//SendMessageW(this->numberOfPointsUpDownHandle, UDM_SETRANGE, 0, MAKELPARAM(this->maxNumberOfPointsToRender, 0));	

	unsigned short currentNumberRangeOffsetY = numberOfPointsOffsetY + numberOfPointsHeight + this->layersOffsetY;

	this->currentNumberOfPointsRangeHandle = CreateWindowW(
		TRACKBAR_CLASS,
		L"Punkt renderowania",
		WS_CHILD | WS_VISIBLE,
		10,
		currentNumberRangeOffsetY,
		this->fractalDefinition->getWidth() - 20,
		this->currentNumberRangeHeight,
		parent,
		NULL,
		(HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE),
		NULL
	);

	unsigned short buttonOffsetX = offsetX + this->fractalDefinition->getTransformationsForm()->getWidth() - buttonWidth;
	unsigned short buttonOffsetY = currentNumberRangeOffsetY + this->currentNumberRangeHeight + this->layersOffsetY;
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

	height = buttonOffsetY + buttonHeight + this->layersOffsetY;	
}

FractalDrawingUI::~FractalDrawingUI()
{
	delete fractalDefinition;
	delete this->numberOfPointsToRenderLabel;
	delete this->numberOfPointsToRender;
	//DestroyWindow(this->numberOfPointsUpDownHandle);
	delete renderFractalButton;
	delete this->importValuesFromPDFButton;
	DestroyWindow(this->currentNumberOfPointsRangeHandle);
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

NaturalInput* FractalDrawingUI::getNumberOfPointsToRender(void)
{
	return this->numberOfPointsToRender;
}

void FractalDrawingUI::setMaxNumberOfPointsToRender(unsigned int maxNumberOfPointsToRender)
{
	this->numberOfPointsToRender->setValue(maxNumberOfPointsToRender);
	SendMessageW(
		this->currentNumberOfPointsRangeHandle,
		TBM_SETRANGEMAX,
		FALSE,
		maxNumberOfPointsToRender
	);
	SendMessageW(
		this->currentNumberOfPointsRangeHandle,
		TBM_SETPOS,
		TRUE,
		maxNumberOfPointsToRender
	);
}

void FractalDrawingUI::ProcessParentWindowMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_NOTIFY:
			{
				NMHDR* message = (NMHDR*)lParam;
				this->getFractalDefinitionForm()->processNotification(message);
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
				/*
				HWND mainWindow = GetWindow(hDlg, GW_OWNER);
				FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);
				if (fractalWindowData->currentFractalBitmapGeneratorV2 != NULL)
				{
					InitializeFractalBitmapThreadV2(
						fractalWindowData,
						(unsigned int)currentPosition
					);
				}
				*/
			}
			break;
		case WM_COMMAND:
			if (lParam != 0)			
			{				
				if (this->getRenderButton()->isCommandFromControl(lParam))
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
						//this->renderSubscribers.
						//HWND mainWindow = GetWindow(hDlg, GW_OWNER);
						//FractalWindowData* fractalWindowData = (FractalWindowData*)GetWindowLongW(mainWindow, GWL_USERDATA);



						//Fractal fractalFromForm = this->getFractalDefinitionForm()->getValue();
						//if (fractalWindowData->fractal != NULL)
						//{
						//	Fractal* fractalPointer = fractalWindowData->fractal;
						//	fractalWindowData->fractal = NULL;
						//	delete fractalWindowData->fractal;
						//}
						//fractalWindowData->fractal = new Fractal(fractalFromForm);

						//RECT windowRect;
						//GetClientRect(mainWindow, &windowRect);
						//unsigned short bitmapWidth = windowRect.right;
						//unsigned short bitmapHeight = windowRect.bottom;

						////parametry skalowania
						//fractalWindowData->updatedScale = 1.0f;
						//fractalWindowData->updateOffsetX = 0;
						//fractalWindowData->updateOffsetY = 0;

						//if (this->getNumberOfPointsToRender()->isValid())
						//{
						//	fractalWindowData->numberOfPointsToProcess = this->getNumberOfPointsToRender()->getValue();
						//}
						//else
						//{
						//	fractalWindowData->numberOfPointsToProcess = initialNumberOfPointsToRender;
						//}


						//InitializeFractalPointsCalculator(fractalWindowData);
						//InitializeFractalPointsThread(fractalWindowData);
						//InitializeFractalRender(
						//	fractalWindowData,
						//	bitmapWidth,
						//	bitmapHeight
						//);

						//fractalWindowData->fractalImage->offsetX = 0;
						//fractalWindowData->fractalImage->offsetY = 0;
						//fractalWindowData->fractalImage->scale = 1.0f;
						//fractalWindowData->fractalImage->width = bitmapWidth;
						//fractalWindowData->fractalImage->height = bitmapHeight;
					}
				}
				else if (this->getImportbutton()->isCommandFromControl(lParam))
				{
					/*
					Tutaj trzeba sprostować
					*/
					/*
					HWND importDialogWindowHandle = CreateDialog(
						(HINSTANCE)GetWindowLongPtr(this->parentWindow, GWLP_HINSTANCE),
						MAKEINTRESOURCE(IDD_FRAKTALE_IMPORT_FROM_PDF),
						hDlg,
						(DLGPROC)ImportFromPdfProc
					);
					ShowWindow(importDialogWindowHandle, SW_SHOW);
					*/
				}
				else
				{
					WORD notificationCode = HIWORD(wParam);
					this->getFractalDefinitionForm()->processControlCommand(
						notificationCode,
						(HWND)lParam
					);
				}
			}
			break;
	}
}

void FractalDrawingUI::SubscribeToFractalRendering(FractalUIRenderingSubsriberInterface& subscriber)
{
	this->renderSubscribers.push_back(&subscriber);
}

void FractalDefinitionForm::processNotification(const NMHDR* message)
{
	if (message->code == UDN_DELTAPOS)
	{
		NMUPDOWN* upDownMessage = (NMUPDOWN*)message;
		this->transformations->processUpDownNotification(upDownMessage);
		this->clipping->processUpDownNotification(upDownMessage);
	}
}

void FractalDefinitionForm::processControlCommand(WORD notificationCode, HWND controlWindowHandle)
{
	if (notificationCode == EN_CHANGE)
	{
		this->transformations->processInputChange(controlWindowHandle);
		this->clipping->processInputChange(controlWindowHandle);
	}
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

void FloatInputWithStepping::updateUpDownPos(short newValue)
{
	SendMessageW(
		this->upDownWindowHandle,
		UDM_SETPOS,
		0,
		newValue
	);
}

void FloatInputWithStepping::updateFloatInput(void)
{
	LRESULT result = SendMessageW(
		this->upDownWindowHandle,
		UDM_GETPOS,
		0,
		0
	);
	if (HIWORD(result) == 0)
	{
		short currentPosition = LOWORD(result);
		float newInputValue = (float)currentPosition / 100.0f;
		FloatInput::setValue(newInputValue);
	}
}

void FloatInputWithStepping::processChange(const NMUPDOWN* upDownMessage)
{
	if (upDownMessage->hdr.hwndFrom == this->upDownWindowHandle)
	{
		float newInputValue = (float)(upDownMessage->iPos + upDownMessage->iDelta) / 100.0f;
		if (newInputValue <= this->max && newInputValue >= this->min)
		{
			FloatInput::setValue(newInputValue);
		}
	}
}

void FloatInputWithStepping::processInputChange(const HWND changedInputWindowHandle)
{
	if (changedInputWindowHandle == this->getInputWindowHandle())
	{
		if (!this->isValueChangedViaSetter())
		{
			short positionValue = (short)(FloatInput::GetValue() * 100.0f);
			this->updateUpDownPos(positionValue);
			this->isValid();
		}
	}
}

bool FloatInputWithStepping::isValid(void)
{
	if (FloatInput::isValid())
	{
		float currentValue = this->GetValue();
		if (currentValue > this->max)
		{
			this->displayError(L"Podana wartość jest większa od maksymalnej");
			return false;
		}
		if (currentValue < this->min)
		{
			this->displayError(L"Podana wartość jest niższa od minimalnej");
			return false;
		}
	}
	return true;
}

void FloatInputWithStepping::setValue(float newValue)
{
	short positionValue = (short)(newValue * 100.0f);
	this->updateUpDownPos(positionValue);
	FloatInput::setValue(newValue);
}

float FloatInputWithStepping::GetValue(void)
{
	LRESULT result = SendMessageW(
		this->upDownWindowHandle,
		UDM_GETPOS,
		0,
		0
	);
	WORD hiWord = HIWORD(result);
	if (hiWord == 1)
	{
		short currentPosition = LOWORD(result);
		return (float)currentPosition / 100.0f;
	}
	else
	{
		DWORD error = GetLastError();
		return 0.0f;
	}
}
