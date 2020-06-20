#include "debug-helper.h"

void debugLastError(void)
{
	DWORD lastError = GetLastError();
	WCHAR errorMessageFormat[] = L"ostatni błąd - %d\n";
	LPWSTR errorString = new WCHAR[sizeof(errorMessageFormat) + 8];
	wsprintfW(errorString, errorMessageFormat, lastError);
	OutputDebugStringW(errorString);
}

void debugRectangle(const RECT* debuggedRect)
{
	const WCHAR debugStringFormat[] = L"T - %d, B - %d, L - %d, R - %d";
	LPWSTR debugString = new WCHAR[sizeof(debugStringFormat) + 32];
	wsprintfW(debugString, debugStringFormat, debuggedRect->top, debuggedRect->bottom, debuggedRect->left, debuggedRect->right);
	OutputDebugStringW(debugString);
}
