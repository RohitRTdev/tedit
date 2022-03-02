#pragma once

#include "winWindow.h"
#include "wMenu.h"


class wtopWindow : public winClass, public winWindow<>
{
public:
	wtopWindow(HINSTANCE _modInst, LPCWSTR _winTitle, wMenu* menu = nullptr, int _xpos = CW_USEDEFAULT, 
		int _ypos = CW_USEDEFAULT, int _nWidth = CW_USEDEFAULT
		, int _nHeight = CW_USEDEFAULT, HICON icon = NULL, 
		HCURSOR cursor = NULL, HBRUSH backgroundBrush = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), HACCEL accelTable = nullptr);

	int run(int showStyle);
	void close(int exitCode);

private:
	wMenu* winMenu;	
	HACCEL accelHandle;
	friend class wMenu;
};

