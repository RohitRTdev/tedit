#include "wtopWindow.h"
#include "wMenu.h"
#include "utils.h"
#include <atomic>

using namespace std::literals;
static std::atomic<size_t> topWindInstances = 0;

LPWSTR generateClassName(size_t seed)
{
	std::wstring stringBuf = L"classwin"s + std::to_wstring(topWindInstances);

	LPWSTR stringRawBuffer = new WCHAR[stringBuf.size() + 1];
	wcscpy_s(stringRawBuffer, stringBuf.size() + 1, stringBuf.c_str());

	return stringRawBuffer;
}

wtopWindow::wtopWindow(HINSTANCE _modInst, LPCWSTR _winTitle, wMenu* menu, int _xpos, 
	int _ypos, int _nWidth, int _nHeight, HICON icon,
	HCURSOR cursor, HBRUSH backgroundBrush, HACCEL accelTable) : winClass(_modInst, generateClassName(++topWindInstances), NULL, backgroundBrush,
		cursor, CS_OWNDC | CS_PARENTDC | CS_DBLCLKS, NULL, icon), winWindow<>(winClassAttrib.lpszClassName, _winTitle, WS_OVERLAPPEDWINDOW,
	_xpos, _ypos, _nWidth, _nHeight, NULL, menu ? menu->menuHandle : nullptr, _modInst), winMenu(menu), accelHandle(accelTable)
{
	if(winMenu)
		winMenu->drawMenu(wWindHandle);
}

int wtopWindow::run(int showStyle)
{
	ShowWindow(wWindHandle, showStyle);

	MSG winMsg;
	BOOL ret;

	//Start message loop
	while ((ret = GetMessage(&winMsg, NULL, 0, 0)))
	{
		if (ret == -1) break;

		if (accelHandle && TranslateAccelerator(wWindHandle, accelHandle, &winMsg))
			continue;
		
		TranslateMessage(&winMsg);
		DispatchMessage(&winMsg);
		
	}

	if (!ret) return static_cast<int>(winMsg.wParam);
	else return -1;
}

void wtopWindow::close(int exitCode) 
{
	winExitCode = exitCode;
	SendMessage(wWindHandle, WM_DESTROY, 0, 0);
}