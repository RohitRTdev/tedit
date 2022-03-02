#pragma once
#define MAXCLASSNAMESIZE 20

#include <Windows.h>
#include <string>


LPWSTR generateClassName(size_t seed);

void preprocessText(void* textBuf, size_t textSize, bool& isText);
void postprocessText(void* textBuf, size_t textSize);
void convertToUTF16(char* utf8Text, size_t numChars, LPWSTR* utf16Text, size_t& textSize);
void convertToUTF8(LPWSTR utf16Text, size_t numChars, char** utf8Text, size_t& textSize);

void displayError(const std::wstring& header, const std::wstring& body, DWORD additionalStyles = 0);
void displayErrorWithoutExit(const std::wstring& header, const std::wstring& body);