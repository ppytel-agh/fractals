#include "fractal-calculations.h"

FractalPoints::FractalPoints(AffineTransformationRowsGroup fractalTransformations, Point startingPoint) : calculatedPoints()
{
	this->fractalTransformations = fractalTransformations;
	this->calculatedPoints.push_back(startingPoint);
}

void FractalPoints::calculatePoints(unsigned int numberOfPointsToCalculate)
{
	if (numberOfPointsToCalculate > this->calculatedPoints.size())
	{

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
		output = nullptr;
		return false;
	}
}
