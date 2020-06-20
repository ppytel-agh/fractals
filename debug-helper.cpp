#include "debug-helper.h"
#include <WinUser.h>

void debugLastError(void)
{
	DWORD lastError = GetLastError();
	WCHAR errorMessageFormat[] = L"ostatni błąd - %d\n";
	LPWSTR errorString = new WCHAR[sizeof(errorMessageFormat) + 8];
	wsprintfW(errorString, errorMessageFormat, lastError);
	OutputDebugStringW(errorString);
}