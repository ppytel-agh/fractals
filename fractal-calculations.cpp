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
	return this->calculatedPixels.size();
}

bool BitmapPixelsCalculator::getPixel(unsigned int pixelIndex, BitmapPixel& output)
{
	if (pixelIndex < this->getNumberOfCalculatedPixels())
	{
		output = this->calculatedPixels[pixelIndex];
		return true;
	}
	else
	{
		return false;
	}
}

FractalPixels::FractalPixels(
	std::shared_ptr<FractalPoints> pointsCalculator,
	FractalClipping fractalClipping,
	unsigned short bitmapWidth,
	unsigned short bitmapHeight,
	unsigned int numberOfPointsToProcess
) : pixelCalculator(bitmapWidth, bitmapHeight, fractalClipping)
{
	this->pointsCalculator = pointsCalculator;
	this->numberOfPointsToProcess = numberOfPointsToProcess;
	this->pointPixelIndexes = new int[this->numberOfPointsToProcess];
	memset(
		this->pointPixelIndexes,
		255,
		sizeof(int) * numberOfPointsToProcess
	);
	this->bitmapWidth = bitmapWidth;
	this->bitmapHeight = bitmapHeight;
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
		concurrency::concurrent_vector<BitmapPixel>::iterator firstPixelIndex = this->calculatedPixels.begin();
		unsigned int numberOfProcessedPoints = 0;
		while(numberOfProcessedPoints < this->numberOfPointsToProcess)
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
								concurrency::concurrent_vector<BitmapPixel>::iterator pushedPixelIterator = this->calculatedPixels.push_back(pixel);
								int pixelIndex = std::distance(firstPixelIndex, pushedPixelIterator) - 1;
								this->pointPixelIndexes[pointIndex] = pixelIndex;
								numberOfProcessedPoints++;
							}
						}
					}
					);
				}
			}
		}
		this->isCalculatingPixels = false;
		return anyPointsToProcess;
	}
}

unsigned short FractalPixels::getBitmapWidth(void)
{
	return this->bitmapWidth;
}

unsigned short FractalPixels::getBitmapHeight(void)
{
	return this->bitmapHeight;
}

bool FractalPixels::getPixelByPointIndex(unsigned int pointIndex, BitmapPixel& output)
{
	if (pointIndex < this->numberOfPointsToProcess)
	{
		if (this->pointPixelIndexes[pointIndex] != -1)
		{
			output = this->calculatedPixels[this->pointPixelIndexes[pointIndex]];
			return true;
		}
	}
	return false;
}
