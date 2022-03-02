#pragma once
#include "winWindow.h"
#include <vector>
#pragma comment(lib, "Comctl32.lib")

struct statusBarRatios
{
	std::wstring displayText;
	size_t ratio;
};

class wStatusBar : private isComCtlClass, public winWindow<isPrivateEvtHandler>
{
public:
	wStatusBar(winWindowCom* parent, size_t id, const std::vector<statusBarRatios>& ratios);
	LONG getHeight() const;
	LONG resize();
	void writeText(size_t index, const std::wstring& statText);
private:
	std::vector<statusBarRatios> parts;
	int* partsArray;
	int ratioDen;

	int getParentWidth() const;

	void calibrateStatusParts(int parentWidth);
	void setPartText() const;
};

