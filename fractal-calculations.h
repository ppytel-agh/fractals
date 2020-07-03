#pragma once
#include "Fractals.h"
#include <concurrent_vector.h>
#include <cstdlib>

class FractalPoints
{
private:
	Fractal fractal;
	concurrency::concurrent_vector<Point> calculatedPoints;
public:
	FractalPoints(
		Fractal fractal,
		Point startingPoint
	);
	void calculatePoints(unsigned int numberOfPointsToCalculate);
	unsigned int getNumberOfCalculatedPoints(void);
	bool getPoint(unsigned int index, Point& output);
};