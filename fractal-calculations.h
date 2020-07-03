#pragma once
#include "Fractals.h"
#include <concurrent_vector.h>

class FractalPoints
{
private:
	AffineTransformationRowsGroup fractalTransformations;
	concurrency::concurrent_vector<Point> calculatedPoints;
public:
	FractalPoints(
		AffineTransformationRowsGroup fractalTransformations,
		Point startingPoint
	);
	void calculatePoints(unsigned int numberOfPointsToCalculate);
	unsigned int getNumberOfCalculatedPoints(void);
	bool getPoint(unsigned int index, Point& output);
};