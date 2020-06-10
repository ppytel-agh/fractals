#pragma once

class Point
{
private:
	float x;
	float y;
public:
	Point();
	Point(
		float x,
		float y
	);
	float GetX(void);
	float GetY(void);
};

class AffineTransformation
{
private:
	float a;
	float b;
	float c;
	float d;
	float e;
	float f;
public:
	AffineTransformation(
		float a,
		float b,
		float c,
		float d,
		float e,
		float f
	);
	Point calculatePrim(Point originalPoint);
};

class AffineTransformationRow
{
private:
	unsigned char probability;
	AffineTransformation transformation;
public:
	AffineTransformationRow(
		unsigned char probability,
		AffineTransformation transformation
	);
	unsigned char getProbability(void);
	AffineTransformation getTransformation(void);
};

class AffineTransformationRowsGroup
{
private:
	AffineTransformationRow** transformationRows;
	unsigned char numberOfRows;
public:
	AffineTransformationRowsGroup(
		AffineTransformationRow transformationRows[],
		unsigned char numberOfRows
	);
	AffineTransformationRowsGroup(const AffineTransformationRowsGroup& prototype);
	~AffineTransformationRowsGroup();
	AffineTransformationRow getAffineTransformation(unsigned char index);
	unsigned char getNumberOfRows();
	bool isValid(void);
};

class FractalClipping
{
private:
	float xMin;
	float xMax;
	float yMin;
	float yMax;
public:
	FractalClipping(
		float xMin,
		float xMax,
		float yMin,
		float yMax
	);
	float getXMin(void);
	float getXMax(void);
	float getYMin(void);
	float getYMax(void);
};

class Fractal
{
private:
	AffineTransformationRowsGroup transformationRowsGroup;
	FractalClipping clipping;
	//internal
	unsigned char numberOfProbabilities;
	unsigned char* probabilityAssociations;
public:
	Fractal(
		AffineTransformationRowsGroup transformationRowsGroup,
		FractalClipping clipping
	);
	Fractal(const Fractal& prototype);
	~Fractal();
	bool isValid(void);
	AffineTransformation getAffineTransformation(int randomValue);
	FractalClipping getClipping(void);
};

class PixelCalculator
{
private:
	unsigned short gxMax;
	unsigned short gyMax;
	FractalClipping fractal;
	//internal
	float xDelta;
	float yDelta;
public:
	PixelCalculator(
		unsigned short gxMax,
		unsigned short gyMax,
		FractalClipping fractal
	);
	unsigned short getPixelX(float xPrim);
	unsigned short getPixelY(float yPrim);
};