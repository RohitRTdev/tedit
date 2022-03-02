#include "wStatusBar.h"
#include <commctrl.h>
#include <stdexcept>

wStatusBar::wStatusBar(winWindowCom* parent, size_t id, const std::vector<statusBarRatios>& ratios) 
	: winWindow<isPrivateEvtHandler>(STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, 
		parent->wWindHandle, reinterpret_cast<HMENU>(id), parent->wInst), parts(ratios), ratioDen(0)
{
	partsArray = new int[ratios.size()];

	int width = getParentWidth();

	for (const auto& ratio : ratios)
	{
		ratioDen += static_cast<int>(ratio.ratio);
	}
	
	calibrateStatusParts(width);
	setPartText();
}

void wStatusBar::calibrateStatusParts(int parentWidth)
{
	int partIndex = 0;
	int accumulatedEdge = 0;
	for (const auto& ratio : parts)
	{
		if (partIndex == parts.size() - 1)
		{
			partsArray[partIndex++] = -1;
			break;
		}
		partsArray[partIndex] = accumulatedEdge + static_cast<int>(static_cast<double>(ratio.ratio) / ratioDen * parentWidth);
		accumulatedEdge += partsArray[partIndex++];
	}

	sendWindowMessage(SB_SETPARTS, partIndex, partsArray);
}

int wStatusBar::getParentWidth() const
{
	RECT rc;
	GetClientRect(parentHandle, &rc);
	return rc.right - rc.left;
}

LONG wStatusBar::getHeight() const
{
	RECT boundingRc;
	sendWindowMessage(SB_GETRECT, 0, &boundingRc);

	return boundingRc.bottom - boundingRc.top;
}

void wStatusBar::setPartText() const
{
	int partIndex = 0;
	for (const auto& ratio : parts)
	{
		sendWindowMessage(SB_SETTEXT, partIndex, ratio.displayText.data());
		partIndex++;
	}
}

LONG wStatusBar::resize()
{
	sendWindowMessage(WM_SIZE, 0, 0);
	int width = getParentWidth();

	calibrateStatusParts(width);
	setPartText();

	return getHeight();
}

void wStatusBar::writeText(size_t index, const std::wstring& statText)
{
	if (index >= parts.size()) throw std::out_of_range("Index greater than number of parts in status bar");

	parts[index].displayText = statText;
	resize();
}