#include "fractale-misc.h"

Fractal getDragonFractal(void)
{
	const unsigned char dragonFractalRowsNumber = 2;
	AffineTransformationRow dragonFractalRows[dragonFractalRowsNumber] = {
				AffineTransformationRow(
					18,
					AffineTransformation(
						-0.4f,
						0.0f,
						-1.0f,
						0.0f,
						-0.4f,
						0.1f
					)
				),
				AffineTransformationRow(
					82,
					AffineTransformation(
						0.76f,
						-0.4f,
						0.0f,
						0.4f,
						0.76f,
						0.0f
					)
				)
	};
	return Fractal(
		AffineTransformationRowsGroup(
			dragonFractalRows,
			dragonFractalRowsNumber
		),
		FractalClipping(
			-1.3f,
			0.6f,
			-0.9f,
			0.45f
		)
	);
}