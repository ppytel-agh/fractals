#pragma once
#include "Fractals.h"
#include <concurrent_vector.h>
#include <cstdlib>
#include <ppl.h>

class FractalPoints
{
private:
	Fractal fractal;
	concurrency::concurrent_vector<Point> calculatedPoints;
	bool isCalculatingPoints;
public:
	FractalPoints(
		Fractal fractal,
		Point startingPoint
	);
	bool calculatePoints(
		unsigned int numberOfPointsToCalculate,
		std::shared_ptr<bool> continueOperation
	);
	unsigned int getNumberOfCalculatedPoints(void);
	bool getPoint(unsigned int index, Point& output);
	bool pointsAreCalculated(void);

};

struct BitmapPixel
{
	unsigned short x;
	unsigned short y;
};

class FractalPixels
{
private:
	std::shared_ptr<FractalPoints> pointsCalculator;
	/*unsigned short bitmapWidth;
	unsigned short bitmapHeight;*/
	PixelCalculator pixelCalculator;
	concurrency::concurrent_vector<BitmapPixel> calculatedPixels;
	bool isCalculatingPixels;
public:
	FractalPixels(
		std::shared_ptr<FractalPoints> pointsCalculator,
		FractalClipping fractalClipping,
		unsigned short bitmapWidth,
		unsigned short bitmapHeight
	);
	bool calculatePixels(std::shared_ptr<bool> continueOperation);
	unsigned int getNumberOfProcessedPoints(void);
	bool getPixel(unsigned int pointIndex, BitmapPixel& output);
};