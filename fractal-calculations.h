#pragma once
#include "Fractals.h"
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <cstdlib>
#include <ppl.h>
#include "gdi-wrapper.h"
#include "FractalsGDI.h"

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

class BitmapPixelsCalculator
{
protected:
	concurrency::concurrent_vector<BitmapPixel> calculatedPixels;	
public:
	unsigned int getNumberOfCalculatedPixels(void);
	bool getPixel(unsigned int pixelIndex, BitmapPixel& output);
};

class FractalPixels : public BitmapPixelsCalculator
{
private:
	std::shared_ptr<FractalPoints> pointsCalculator;
	unsigned short bitmapWidth;
	unsigned short bitmapHeight;
	PixelCalculator pixelCalculator;	
	bool isCalculatingPixels;
	unsigned int numberOfPointsToProcess;
	int* pointPixelIndexes;
public:
	FractalPixels(
		std::shared_ptr<FractalPoints> pointsCalculator,
		FractalClipping fractalClipping,
		unsigned short bitmapWidth,
		unsigned short bitmapHeight,
		unsigned int numberOfPointsToProcess
	);
	~FractalPixels();
	bool calculatePixels(std::shared_ptr<bool> continueOperation);	
	unsigned short getBitmapWidth(void);
	unsigned short getBitmapHeight(void);
	bool getPixelByPointIndex(unsigned int pointIndex, BitmapPixel& output);
};

class AbstractFractalPixels : public BitmapPixelsCalculator
{
protected:
	std::shared_ptr<FractalPoints> pointsCalculator;
	FractalPixelCalculatorGDI pixelCalculator;
public:
	AbstractFractalPixels(
		std::shared_ptr<FractalPoints> pointsCalculator,
		FractalPixelCalculatorGDI pixelCalculator
	);
	FractalPixelCalculatorGDI GetPixelCalculator(void);
};

class FractalPixelsV2 : public AbstractFractalPixels
{
private:
	bool isCalculatingPixels;
	concurrency::concurrent_vector<unsigned int>** pixelPoints;
public:
	FractalPixelsV2(
		std::shared_ptr<FractalPoints> pointsCalculator,
		FractalPixelCalculatorGDI pixelCalculator
	);
	bool calculatePixels(
		std::shared_ptr<bool> continueOperation,
		unsigned int numberOfPointsToProcess
	);
};