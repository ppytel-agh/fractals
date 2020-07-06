#include "fractal-calculations.h"

FractalPoints::FractalPoints(
	Fractal fractal,
	Point startingPoint
)
{
	this->fractal = fractal;
	this->calculatedPoints.push_back(startingPoint);
	this->isCalculatingPoints = false;
}

bool FractalPoints::calculatePoints(
	unsigned int numberOfPointsToCalculate,
	std::shared_ptr<bool> continueOperation
)
{
	if (this->isCalculatingPoints)
	{
		return false;
	}
	else
	{
		this->isCalculatingPoints = true;
		unsigned int currentSize = this->calculatedPoints.size();
		bool anyPointsToCalculate = false;
		if (numberOfPointsToCalculate > currentSize)
		{
			anyPointsToCalculate = true;
			Point currentPoint = this->calculatedPoints[currentSize - 1];
			for (
				unsigned int i = currentSize;
				i < numberOfPointsToCalculate;
				i++
				)
			{
				if (*continueOperation)
				{
					currentPoint = this->fractal.getAffineTransformation(rand()).calculatePrim(currentPoint);
					this->calculatedPoints.push_back(currentPoint);
				}
				else
				{
					break;
				}
			}
		}
		this->isCalculatingPoints = false;
		return anyPointsToCalculate;
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

bool FractalPoints::pointsAreCalculated(void)
{
	return this->isCalculatingPoints;
}

unsigned int BitmapPixelsCalculator::getNumberOfCalculatedPixels(void)
{
	return this->calculatedPixels2.size();
}

bool BitmapPixelsCalculator::getPixel(unsigned int pixelIndex, BitmapPixel& output)
{
	if (this->calculatedPixels2.find(pixelIndex) == this->calculatedPixels2.end())
	{
		return false;
	}
	else
	{
		output = this->calculatedPixels2[pixelIndex];
		return true;
	}
}

FractalPixels::FractalPixels(
	std::shared_ptr<FractalPoints> pointsCalculator,
	FractalClipping fractalClipping,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight
) : pixelCalculator(bitmapWidth, bitmapHeight, fractalClipping)
{
	this->pointsCalculator = pointsCalculator;
}

bool FractalPixels::calculatePixels(std::shared_ptr<bool> continueOperation)
{
	if (this->isCalculatingPixels)
	{
		return false;
	}
	else
	{
		this->isCalculatingPixels = true;
		bool anyPointsToProcess = false;
		do
		{
			if (*continueOperation)
			{
				unsigned int noCalculatedPoints = this->pointsCalculator->getNumberOfCalculatedPoints();
				unsigned int noCalculatedPixels = this->calculatedPixels.size();
				if (noCalculatedPoints > noCalculatedPixels)
				{
					anyPointsToProcess = true;
					unsigned int lastOutputtedPointIndex = noCalculatedPoints - 1;
					concurrency::parallel_for(
						noCalculatedPixels,
						lastOutputtedPointIndex,
						(unsigned int)1,
						[&](unsigned int pointIndex)
					{
						Point pointBuffer;
						BitmapPixel pixel = {};
						bool processPoint = true;
						if (*continueOperation)
						{
							if (this->pointsCalculator->getPoint(pointIndex, pointBuffer))
							{
								pixel.x = pixelCalculator.getPixelX(pointBuffer.GetX());
								pixel.y = pixelCalculator.getPixelY(pointBuffer.GetY());		
								this->calculatedPixels2[pointIndex] = pixel;
							}
						}
					}
					);
				}
			}
		} while (this->pointsCalculator->pointsAreCalculated());
		this->isCalculatingPixels = false;
		return anyPointsToProcess;
	}
}