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

AffineTransformationForm::AffineTransformationForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	const unsigned char inputsOffset = 50;
	unsigned short elementOffsetX = 0;
	a = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
	);
	b = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
	);
	c = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
	);
	d = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
	);
	e = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
	);
	f = new FloatInput(
		parent,
		elementOffsetX += inputsOffset,
		offsetY,
		40,
		20
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

FractalTransformationsRowForm::FractalTransformationsRowForm(HWND parent, unsigned short offsetX, unsigned short offsetY)
{
	probability = new NaturalInput(
		parent,
		offsetX,
		offsetY,
		40,
		20
	);
	affineTransformationForm = new AffineTransformationForm(
		parent,
		offsetX + 50,
		offsetY
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
