#include "Fractals.h"

Point::Point()
{
	this->x = 0;
	this->y = 0;
}

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

Point AffineTransformation::calculatePrim(Point originalPoint)
{
	float xPrim = this->a * originalPoint.GetX() + this->b * originalPoint.GetY() + this->c;
	float yPrim = this->d * originalPoint.GetX() + this->e * originalPoint.GetY() + this->f;
	return Point(xPrim, yPrim);
}

AffineTransformationRow::AffineTransformationRow(
	unsigned char probability,
	AffineTransformation transformation
) : transformation(transformation)
{
	this->probability = probability;
}

unsigned char AffineTransformationRow::getProbability(void)
{
	return this->probability;
}

AffineTransformation AffineTransformationRow::getTransformation(void)
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

Fractal::Fractal(
	AffineTransformationRow transformationRows[],
	unsigned char numberOfRows,
	FractalClipping clipping
) : clipping(clipping), numberOfRows{numberOfRows}
{
	unsigned char i = 0;
	unsigned char probabilitiesSum = 0;
	for (; i < numberOfRows; i++)
	{
		probabilitiesSum += transformationRows[i].getProbability();
	}
	if (probabilitiesSum == 100) {
		this->transformationRows = new AffineTransformationRow*[numberOfRows];
		for (; i < numberOfRows; i++)
		{
			this->transformationRows[i] = new AffineTransformationRow(transformationRows[i]);
		}
		this->numberOfProbabilities = numberOfRows - 1;
		this->probabilityAssociations = new unsigned char[this->numberOfProbabilities];
		unsigned char maxProbabilityValue = 0;
		for (i = 0; i < this->numberOfProbabilities; i++)
		{
			maxProbabilityValue += this->transformationRows[i]->getProbability();
			this->probabilityAssociations[i] = maxProbabilityValue;
		}
	} else {
		this->transformationRows = nullptr;
		this->probabilityAssociations = nullptr;
	}
}

Fractal::~Fractal()
{
	if (this->transformationRows != nullptr)
	{
		for (int i = 0; i < this->numberOfRows; i++)
		{
			delete this->transformationRows[i];
		}
		delete[] this->transformationRows;
	}
	if (probabilityAssociations != nullptr)
	{
		delete[] probabilityAssociations;
	}
}

bool Fractal::isValid(void)
{
	return this->transformationRows != nullptr;
}

AffineTransformation Fractal::getAffineTransformation(int randomValue)
{
	unsigned char percentageValue = randomValue % 100;
	int i = 0;
	for (;i < this->numberOfProbabilities; i++)
	{
		if (percentageValue < this->probabilityAssociations[i])
		{
			return this->transformationRows[i]->getTransformation();
		}
	}
	return this->transformationRows[this->numberOfProbabilities]->getTransformation();
}

FractalClipping Fractal::getClipping(void)
{
	return this->clipping;
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