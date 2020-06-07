#include "Fractals.h"

Point::Point(float x, float y)
{
	this->x = x;
	this->y = y;
}

float Point::GetX(void)
{
	return x;
}

float Point::GetY(void)
{
	return y;
}

AffineTransformation::AffineTransformation(float a, float b, float c, float d, float e, float f)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
	this->e = e;
	this->f = f;
}

Point* AffineTransformation::calculatePrim(Point* originalPoint)
{
	float xPrim = this->a * originalPoint->GetX() + this->b * originalPoint->GetY() + this->c;
	float yPrim = this->d * originalPoint->GetX() + this->e * originalPoint->GetY() + this->f;
	return new Point(xPrim, yPrim);
}

AffineTransformationRow::AffineTransformationRow(unsigned char probability, AffineTransformation* transformation)
{
	this->probability = probability;
	this->transformation = transformation;
}

unsigned char AffineTransformationRow::getProbability(void)
{
	return this->probability;
}

AffineTransformation* AffineTransformationRow::getTransformation(void)
{
	return this->transformation;
}

FractalClipping::FractalClipping(float xMin, float xMax, float yMin, float yMax)
{
	this->xMin = xMin;
	this->xMax = xMax;
	this->yMin = yMin;
	this->yMax = yMax;
}

float FractalClipping::getXMin(void)
{
	return this->xMin;
}

float FractalClipping::getXMax(void)
{
	return this->xMax;
}

float FractalClipping::getYMin(void)
{
	return this->yMin;
}

float FractalClipping::getYMax(void)
{
	return this->yMax;
}

Fractal::Fractal(AffineTransformationRow** transformationRows, unsigned char numberOfRows, FractalClipping* clipping)
{
	unsigned char i = 0;
	unsigned char probabilitiesSum = 0;
	for (; i < numberOfRows; i++)
	{
		probabilitiesSum += transformationRows[i]->getProbability();
	}
	if (probabilitiesSum == 100) {
		this->transformationRows = transformationRows;
		this->numberOfRows = numberOfRows;
		this->clipping = clipping;
		this->probabilityAssociations = new unsigned char[numberOfRows - 1];

	} else {
		this->transformationRows = nullptr;
		this->numberOfRows = 0;
		this->clipping = nullptr;
		this->probabilityAssociations = nullptr;
	}
}

Fractal::~Fractal()
{
	if (probabilityAssociations != nullptr)
	{
		delete[] probabilityAssociations;
	}
}

PixelCalculator::PixelCalculator(unsigned short gxMax, unsigned short gyMax, FractalClipping* fractal)
{
	this->gxMax = gxMax;
	this->gyMax = gyMax;
	this->fractal = fractal;
	this->xDelta = fractal->getXMax() - fractal->getXMin();
	this->yDelta = fractal->getYMax() - fractal->getYMin();
}

unsigned short PixelCalculator::getPixelX(float xPrim)
{
	float result = xPrim - fractal->getXMin();
	result *= gxMax;
	result /= xDelta;
	unsigned short integerResult = static_cast<unsigned short>(result);
	return integerResult;
}

unsigned short PixelCalculator::getPixelY(float yPrim)
{
	float result = yPrim - fractal->getYMin();
	result /= yDelta;
	result = 1 - result;
	result *= gyMax;
	unsigned short integerResult = static_cast<unsigned short>(result);
	return integerResult;
}
