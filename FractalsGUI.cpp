#include "FractalsGUI.h"

FloatInput::FloatInput(
	HWND parent,
	unsigned short offsetX,
	unsigned short offsetY,
	unsigned char width,
	unsigned char height
)
{
	this->hWnd = CreateWindowExW(
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

FloatInput::~FloatInput()
{
	DestroyWindow(this->hWnd);
}

float FloatInput::GetValue(void)
{
	TCHAR buffer[16];
	Edit_GetLine(
		this->hWnd,
		NULL,
		buffer,
		16
	);
	return (float)atof((const char*)buffer);
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

void AffineTransformationForm::GetValue(AffineTransformation& output)
{
	output = AffineTransformation(
		a->GetValue(),
		b->GetValue(),
		c->GetValue(),
		d->GetValue(),
		e->GetValue(),
		f->GetValue()
	);
}
