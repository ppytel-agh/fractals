#pragma once
#include "Fractals.h"
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
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

class BitmapPixelsCalculator
{
protected:
	concurrency::concurrent_vector<BitmapPixel> calculatedPixels;
	concurrency::concurrent_unordered_map<unsigned int, BitmapPixel> calculatedPixels2;
public:
	unsigned int getNumberOfCalculatedPixels(void);
	bool getPixel(unsigned int pixelIndex, BitmapPixel& output);
};

class FractalPixels : public BitmapPixelsCalculator
{
private:
	std::shared_ptr<FractalPoints> pointsCalculator;
	/*unsigned short bitmapWidth;
	unsigned short bitmapHeight;*/
	PixelCalculator pixelCalculator;
	
	bool isCalculatingPixels;
public:
	FractalPixels(
		std::shared_ptr<FractalPoints> pointsCalculator,
		FractalClipping fractalClipping,
		unsigned short bitmapWidth,
		unsigned short bitmapHeight
	);
	bool calculatePixels(std::shared_ptr<bool> continueOperation);	
};