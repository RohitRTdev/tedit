#include "winWindow.h"
#include <exception>
#include <stdexcept>
#include <commctrl.h>

bool isComCtlClass::comCtlInitComplete = false;
std::unordered_map<HWND, winWindowCom*> winWindowCom::handleToInst;
std::unordered_map<HWND, procHolder> subClass::windProcMapper;

LRESULT CALLBACK winMessageDispatcher(HWND winHandle, UINT winMsg, WPARAM wparam, LPARAM lparam)
{
	if (winMsg == WM_CREATE)
	{
		winWindowCom::handleToInst[winHandle] = static_cast<winWindowCom*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
		return 0;
	}

	winWindowCom* wInst = nullptr;

	try
	{
		wInst = winWindowCom::handleToInst.at(winHandle);
	}
	catch (std::out_of_range& e)
	{
		static_cast<void>(e);
		return DefWindowProc(winHandle, winMsg, wparam, lparam);
	}
	
	if (winMsg == WM_DESTROY)
	{
		PostQuitMessage(static_cast<winWindow<>*>(wInst)->winExitCode);
		winWindowCom::handleToInst.erase(winHandle);
		return 0;
	}

	defFunctor evtDefAction{ winHandle, winMsg, wparam, lparam };

	// Event dispatch mechanism dispatches the event to the instance of the class which the message originated from
	// If event handler was registered using winWindow::bind for the corresponding window instance, then that event handler will be called
	if (static_cast<winWindow<>*>(wInst)->evtSuperMap.contains(winMsg))
	{
		return (static_cast<winWindow<>*>(wInst)->*(static_cast<winWindow<>*>(wInst)->evtSuperMap[winMsg]))(evtDefAction, winMsg, wparam, lparam);
	}

	return evtDefAction();
}

winClass::winClass(HINSTANCE _hinst, LPCWSTR _winClassName, LPCWSTR _menuName, HBRUSH _clsBackground,
	HCURSOR _cursor, DWORD _style, HICON _icosm, HICON _ico)
{
	
	winClassAttrib.cbSize = sizeof(WNDCLASSEX);
	winClassAttrib.lpfnWndProc = winMessageDispatcher;
	winClassAttrib.lpszClassName = _winClassName;
	winClassAttrib.lpszMenuName = _menuName;
	winClassAttrib.cbClsExtra = winClassAttrib.cbWndExtra = 0;
	winClassAttrib.hInstance = _hinst;
	winClassAttrib.hCursor = _cursor;
	winClassAttrib.style = _style;
	winClassAttrib.hIconSm = _icosm;
	winClassAttrib.hIcon = _ico;
	winClassAttrib.hbrBackground = _clsBackground;


	if (!RegisterClassEx(&winClassAttrib)) throw std::exception("Failed to register window class");
}

winClass::~winClass()
{
	UnregisterClass(winClassAttrib.lpszClassName, winClassAttrib.hInstance);
	delete[] winClassAttrib.lpszClassName;
}


winWindowCom::winWindowCom(LPCWSTR _wClassName, LPCWSTR _windowName, DWORD _wStyle,
	int _xPos, int _yPos, int _wWidth, int _wHeight, HWND _pHandle, HMENU _idSmenu, HINSTANCE _modInst, void* windInst, bool useANSI)
	: windowName(_windowName), wStyle(_wStyle), xPos(_xPos), yPos(_yPos), wWidth(_wWidth), wHeight(_wHeight),
	parentHandle(_pHandle), menuHandle(_idSmenu), wInst(_modInst)
{
	wWindHandle = CreateWindow(_wClassName,
					windowName, wStyle,
					xPos, yPos, wWidth, wHeight,
					parentHandle,
					menuHandle,
					wInst,
					windInst);

	if (!wWindHandle) throw std::exception("Window Creation Failed!");
}

void winWindowCom::writeText(WCHAR* newText)
{
	sendWindowMessage(WM_SETTEXT, 0, newText);
}

LRESULT defFunctor::operator()()
{
	return DefWindowProc(winHandle, winMsg, wparam, lparam);
}

winWindowCom::~winWindowCom()
{
	DestroyWindow(wWindHandle);
}

LRESULT subClassComProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT val = CallWindowProc(subClass::windProcMapper[hwnd].savedProc, hwnd, msg, wparam, lparam);
	CallWindowProc(subClass::windProcMapper[hwnd].newProc, hwnd, msg, wparam, lparam);

	return val;
}


void subClass::hookWindowProc(WNDPROC newProc)
{
	HWND wHandle = getWinHandle();
	WNDPROC savedProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(wHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(subClassComProc)));
	
	subClass::windProcMapper[wHandle] = { newProc, savedProc };
}

isComCtlClass::isComCtlClass()
{
	if (!comCtlInitComplete)
	{
		INITCOMMONCONTROLSEX comCtlSpec = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES };
		InitCommonControlsEx(&comCtlSpec);
		comCtlInitComplete = true;
	}

}