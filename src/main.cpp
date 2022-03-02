#include <Windows.h>
#include <WinUser.h>
#include "winlib.h"
#include "defs.h"
#include "utils.h"
#include "teditApp.h"
#include "teditEvents.h"

HWND mainWindowHandle = NULL;

//Entry point
int WINAPI wWinMain(HINSTANCE modInst, HINSTANCE prevInst, LPWSTR pCmdLine, int cmdShow)
{

	SystemParametersInfo(SPI_SETFONTSMOOTHING,
		TRUE,
		0,
		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE,
		0,
		(PVOID)FE_FONTSMOOTHINGCLEARTYPE,
		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

	SystemParametersInfo(SPI_SETFONTSMOOTHINGCONTRAST,
		0,
		(PVOID)2200,
		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int appXPos = CW_USEDEFAULT, appYPos = CW_USEDEFAULT;
	int appWidth = CW_USEDEFAULT, appHeight = CW_USEDEFAULT;
	//int appWidth = static_cast<int>(screenWidth * APPTOSCREEN_RATIO), appHeight = static_cast<int>(appWidth * ASPECTRATIO);
	
	//Load assets and resources
	HICON teditIcon = static_cast<HICON>(LoadImage(modInst, MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
	HCURSOR teditCursor = static_cast<HCURSOR>(LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	HBRUSH teditBGB = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	HMENU teditMenuHandle = LoadMenu(modInst, MAKEINTRESOURCE(ID_MENU));
	HACCEL teditAccel = LoadAccelerators(modInst, MAKEINTRESOURCE(ID_ACCEL));


	if (!teditIcon || !teditCursor || !teditBGB || !teditMenuHandle)
		displayError(L"Asset Initialisation failed!", L"Failed to load required asset!");

	wMenu teditMenu(teditMenuHandle);

	//Register window class and create window
	wtopWindow mainFrame = wtopWindow(modInst, L"tedit", &teditMenu, appXPos, appYPos, appWidth, appHeight, teditIcon, teditCursor, teditBGB, teditAccel);
	mainWindowHandle = mainFrame.wWindHandle;

	//Create the status bar
	wStatusBar teditStatBar = wStatusBar(&mainFrame, ID_STATUSBAR, { {L"", 2}, {L"", 1}, {L"\tUTF-8", 1}});


	//Register all the event handlers and create app instance
	teditApp* appInst = teditApp::getAppInstance(mainFrame, teditMenu, teditStatBar);
	teditEvents evtListener(appInst);

	return mainFrame.run(cmdShow);
}