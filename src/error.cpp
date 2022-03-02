#include "utils.h"
#include <Windows.h>

extern HWND mainWindowHandle;

void displayError(const std::wstring& header, const std::wstring& body, DWORD additionalStyles)
{
	MessageBox(mainWindowHandle, body.c_str(), header.c_str(), MB_OK | additionalStyles);
	exit(1);
}

void displayErrorWithoutExit(const std::wstring& header, const std::wstring& body)
{
	MessageBox(mainWindowHandle, body.c_str(), header.c_str(), MB_OK);
}