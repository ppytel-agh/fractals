#include "fraktale-misc.h"
#include <corecrt_wstring.h>
#include <corecrt_wstdlib.h>
#include <corecrt_malloc.h>

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

bool parseFractalFromPDF(wchar_t* textBuffer, Fractal** output)
{
	//policz liczbę linii tekstu
	wchar_t newline[] = L"\r\n";
	unsigned char newlineLength = wcslen(newline);
	wchar_t* substringPointer = textBuffer;
	int numberOfLines = 1;
	while (substringPointer = wcsstr(substringPointer, newline))
	{
		substringPointer += newlineLength;
		numberOfLines++;
	}
	//utwórz tablicę z linijkami
	unsigned char* lineLengths = new unsigned char[numberOfLines];
	wchar_t** linesArray = new wchar_t * [numberOfLines];
	substringPointer = textBuffer;
	for (unsigned char i = 0; i < (numberOfLines - 1); i++)
	{
		wchar_t* lineStart = substringPointer;
		substringPointer = wcsstr(substringPointer, newline);
		unsigned char lineLength = (unsigned char)(substringPointer - lineStart);
		linesArray[i] = new wchar_t[lineLength + 1];
		memcpy((void*)linesArray[i], (void*)lineStart, sizeof(wchar_t) * lineLength);
		linesArray[i][lineLength] = L'\0';
		substringPointer += newlineLength;
		lineLengths[i] = lineLength;
	}
	//ostatnia linijka
	unsigned char lastLineLength = wcslen(substringPointer);
	linesArray[numberOfLines - 1] = new wchar_t[lastLineLength + 1];
	memcpy((void*)linesArray[numberOfLines - 1], (void*)substringPointer, sizeof(wchar_t) * (lastLineLength + 1));
	//iteracja linijek
	for (unsigned char i = 0; i < numberOfLines; i++)
	{
		wchar_t* line = linesArray[i];
		char x = 'd';
	}
	//walidacja nagłówków
	if (numberOfLines > 2)
	{
		int firstLineComparisonResult = wcscmp(linesArray[0], L"prawdopodobieństwo");
		int secondLineComparisonResult = wcscmp(linesArray[1], L"Współczynniki odwzorowania");
		if ((firstLineComparisonResult == 0) && (secondLineComparisonResult == 0))
		{
			//waliacja wierszy
			const unsigned char numberOfLinesPerRow = 7;
			unsigned char expectedNumberOfRows = (numberOfLines - 2) / numberOfLinesPerRow;
			AffineTransformationRow* transformationRowsArray = (AffineTransformationRow*)malloc(sizeof(AffineTransformationRow) * expectedNumberOfRows);
			for (unsigned char i = 0; i < expectedNumberOfRows; i++)
			{
				unsigned char probabilityRow = 2 + (i * 7);
				unsigned char probLineLength = wcslen(linesArray[probabilityRow]);
				unsigned char probability = 0;
				float paramValues[6] = {};
				if (linesArray[probabilityRow][probLineLength - 1] == L'%')
				{
					probability = (unsigned char)_wtoi(linesArray[probabilityRow]);
				}
				else
				{
					return false;
				}
				for (unsigned char j = 97; j <= 102; j++)
				{
					wchar_t letter = (wchar_t)(char)(j);
					unsigned char lineIndex = 2 + (i * 7) + (j - 96);
					unsigned char lineLength = wcslen(linesArray[probabilityRow]);
					if (lineLength >= 2)
					{
						if (linesArray[lineIndex][0] == letter && linesArray[lineIndex][1] == L'=')
						{
							paramValues[j - 97] = _wtof((wchar_t*)&linesArray[lineIndex][2]);
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				AffineTransformation transformation(
					paramValues[0],
					paramValues[1],
					paramValues[2],
					paramValues[3],
					paramValues[4],
					paramValues[5]
				);
				transformationRowsArray[i] = AffineTransformationRow(
					probability,
					transformation
				);
			}
			AffineTransformationRowsGroup rowsGroup(
				transformationRowsArray,
				expectedNumberOfRows
			);
			free(transformationRowsArray);
			const wchar_t* lastLineParts[] = {
			   L"zakres wyświetlanych punktów na ekranie: xmin=",
			   L"; xmax=",
			   L"; ymin=",
			   L"; ymax=",
			};
			const unsigned char noParts = sizeof(lastLineParts) / sizeof(wchar_t*);
			wchar_t* lastLine = linesArray[numberOfLines - 1];
			wchar_t* substring = lastLine;
			wchar_t* nextSubstring = NULL;
			float clippingParts[noParts] = {};
			for (unsigned char i = 0; i < (noParts - 1); i++)
			{
				substring = wcsstr(substring, lastLineParts[i]);
				if (substring != NULL)
				{
					nextSubstring = wcsstr(substring, lastLineParts[i + 1]);
					if (nextSubstring != NULL)
					{
						unsigned char lastLinePartLen = wcslen(lastLineParts[i]);
						unsigned char clippingPartLength = (unsigned char)(nextSubstring - substring) - lastLinePartLen;
						wchar_t* clippingPartSubstring = substring + lastLinePartLen;
						wchar_t* clippingPartString = new wchar_t[clippingPartLength + 1];
						memcpy(clippingPartString, clippingPartSubstring, sizeof(wchar_t) * clippingPartLength);
						clippingPartString[clippingPartLength] = L'\0';
						//zastąp przecinek kropką
						wchar_t* delimiterPointer = wcsrchr(clippingPartString, L',');
						*delimiterPointer = L'.';
						float conversionResult = _wtof(clippingPartString);
						clippingParts[i] = conversionResult;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}

			}
			//unsigned char lastLineLength = wcslen(lastLine);
			unsigned char lastPartIndex = noParts - 1;
			unsigned char lastPartLength = wcslen(lastLineParts[lastPartIndex]);
			wchar_t* lastPartSubstring = nextSubstring + lastPartLength;
			wchar_t* lastPartString = new wchar_t[lastPartLength + 1];
			memcpy(lastPartString, lastPartSubstring, sizeof(wchar_t) * lastPartLength);
			lastPartString[lastPartLength] = L'\0';
			wchar_t* delimiterPointer = wcsrchr(lastPartString, L',');
			*delimiterPointer = L'.';
			clippingParts[lastPartIndex] = (float)_wtof(lastPartString);
			FractalClipping clipping(
				clippingParts[0],
				clippingParts[1],
				clippingParts[2],
				clippingParts[3]
			);
			*output = new Fractal(
				rowsGroup,
				clipping
			);
			return true;
		}
	}
	return false;
}
