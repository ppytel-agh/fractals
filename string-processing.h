#pragma once

#include <sstream>
#include <iomanip>
#include <string>
#include <Windows.h>

LPWSTR ansiToUnicode(const char* cString);
LPWSTR floatToString(float value);
LPWSTR integerToString(int value);