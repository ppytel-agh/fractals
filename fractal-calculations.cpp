#include "fractal-calculations.h"

FractalPoints::FractalPoints(
	Fractal fractal,
	Point startingPoint
) 
{
	this->fractal = fractal;
	this->calculatedPoints.push_back(startingPoint);
}

void FractalPoints::calculatePoints(unsigned int numberOfPointsToCalculate)
{
	unsigned int currentSize = this->calculatedPoints.size();
	if (numberOfPointsToCalculate > currentSize)
	{
		Point currentPoint = this->calculatedPoints[currentSize - 1];
		for (unsigned int i = currentSize; i < numberOfPointsToCalculate; i++)
		{
			currentPoint = this->fractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
			this->calculatedPoints.push_back(currentPoint);
		}
	}
}

unsigned int FractalPoints::getNumberOfCalculatedPoints(void)
{
	return this->calculatedPoints.size();
}

bool FractalPoints::getPoint(unsigned int index, Point& output)
{
	if (index < this->calculatedPoints.size())
	{
		output = this->calculatedPoints[index];
		return true;
	}
	else
	{
		return false;
	}
}
