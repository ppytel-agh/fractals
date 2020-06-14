#include "string-processing.h"


LPWSTR ansiToUnicode(const char* cString)
{
	size_t size = strlen(cString) + 1;
	LPWSTR unicodeBuffer = new WCHAR[size];
	size_t outSize;
	mbstowcs_s(&outSize, unicodeBuffer, size, cString, size - 1);
	return unicodeBuffer;
}

LPWSTR floatToString(float value)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << value;
	std::string outputString = stream.str();
	const char* cString = outputString.c_str();
	return ansiToUnicode(outputString.c_str());
}

LPWSTR floatToStringNonPrecise(float value)
{
	std::string valueString = std::to_string(value);
	const char* cString = valueString.c_str();
	return ansiToUnicode(cString);
}

LPWSTR integerToString(int value)
{
	std::string valueString = std::to_string(value);
	const char* cString = valueString.c_str();
	return ansiToUnicode(cString);
}
