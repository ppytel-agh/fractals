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

AffineTransformationRowsGroup::AffineTransformationRowsGroup(
	AffineTransformationRow transformationRows[],
	unsigned char numberOfRows
) : numberOfRows(numberOfRows)
{
	unsigned char i = 0;
	unsigned char probabilitiesSum = 0;
	for (; i < numberOfRows; i++)
	{
		probabilitiesSum += transformationRows[i].getProbability();
	}
	if (probabilitiesSum == 100) {
		this->transformationRows = new AffineTransformationRow * [numberOfRows];
		for (i = 0; i < numberOfRows; i++)
		{
			this->transformationRows[i] = new AffineTransformationRow(
				transformationRows[i].getProbability(),
				transformationRows[i].getTransformation()
			);
		}
	}
	else {
		this->transformationRows = nullptr;
	}
}

AffineTransformationRowsGroup::AffineTransformationRowsGroup(const AffineTransformationRowsGroup& prototype)
{
	this->numberOfRows = prototype.numberOfRows;
	this->transformationRows = new AffineTransformationRow * [this->numberOfRows];
	for (unsigned char i = 0; i < this->numberOfRows; i++)
	{
		this->transformationRows[i] = new AffineTransformationRow(*prototype.transformationRows[i]);
	}
}

AffineTransformationRowsGroup::~AffineTransformationRowsGroup()
{
	if (this->transformationRows != nullptr)
	{
		for (int i = 0; i < this->numberOfRows; i++)
		{
			delete this->transformationRows[i];
		}
		delete[] this->transformationRows;
	}
}

AffineTransformationRow AffineTransformationRowsGroup::getAffineTransformation(unsigned char index)
{
	return 	*(this->transformationRows[index]);
}

unsigned char AffineTransformationRowsGroup::getNumberOfRows()
{
	return this->numberOfRows;
}

bool AffineTransformationRowsGroup::isValid(void)
{
	return this->transformationRows != nullptr;
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
	AffineTransformationRowsGroup transformationRowsGroup,
	FractalClipping clipping
) : clipping(clipping), transformationRowsGroup(transformationRowsGroup)
{
	this->numberOfProbabilities = transformationRowsGroup.getNumberOfRows() - 1;
	if (this->numberOfProbabilities > 0)
	{
		this->probabilityAssociations = new unsigned char[this->numberOfProbabilities];
		unsigned char maxProbabilityValue = 0;
		for (unsigned char i = 0; i < this->numberOfProbabilities; i++)
		{
			maxProbabilityValue += transformationRowsGroup.getAffineTransformation(i).getProbability();
			this->probabilityAssociations[i] = maxProbabilityValue;
		}
	}
	else
	{
		this->probabilityAssociations = nullptr;
	}
}

Fractal::Fractal(const Fractal& prototype)
	: clipping(prototype.clipping),
	transformationRowsGroup(prototype.transformationRowsGroup),
	numberOfProbabilities{numberOfProbabilities}
{	
	if (this->numberOfProbabilities > 0)
	{
		this->probabilityAssociations = new unsigned char[prototype.numberOfProbabilities];
		unsigned char maxProbabilityValue = 0;
		for (unsigned char i = 0; i < prototype.numberOfProbabilities; i++)
		{
			this->probabilityAssociations[i] = prototype.probabilityAssociations[i];
		}
	}
	else
	{
		this->probabilityAssociations = nullptr;
	}
}

Fractal::~Fractal()
{
	if (this->probabilityAssociations != nullptr)
	{
		delete[] probabilityAssociations;
	}
}

bool Fractal::isValid(void)
{
	return transformationRowsGroup.isValid();
}

AffineTransformation Fractal::getAffineTransformation(int randomValue)
{
	unsigned char percentageValue = randomValue % 100;
	for (unsigned char i = 0;i < this->numberOfProbabilities; i++)
	{
		if (percentageValue < this->probabilityAssociations[i])
		{
			return this->transformationRowsGroup.getAffineTransformation(i).getTransformation();
		}
	}
	return this->transformationRowsGroup.getAffineTransformation(this->numberOfProbabilities).getTransformation();
}

FractalClipping Fractal::getClipping(void)
{
	return this->clipping;
}

PixelCalculator::PixelCalculator(
	unsigned short gxMax,
	unsigned short gyMax,
	FractalClipping fractal
) :fractal(fractal)
{
	this->gxMax = gxMax;
	this->gyMax = gyMax;
	this->fractal = fractal;
	this->xDelta = fractal.getXMax() - fractal.getXMin();
	this->yDelta = fractal.getYMax() - fractal.getYMin();
}

unsigned short PixelCalculator::getPixelX(float xPrim)
{
	float result = xPrim - fractal.getXMin();
	result *= gxMax;
	result /= xDelta;
	unsigned short integerResult = static_cast<unsigned short>(result);
	return integerResult;
}

unsigned short PixelCalculator::getPixelY(float yPrim)
{
	float result = yPrim - fractal.getYMin();
	result /= yDelta;
	result = 1 - result;
	result *= gyMax;
	unsigned short integerResult = static_cast<unsigned short>(result);
	return integerResult;
}
