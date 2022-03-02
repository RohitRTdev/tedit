#pragma once

#include <Windows.h>
#include <unordered_map>
#include <type_traits>
#include <list>

struct isPrivateEvtHandler : std::false_type
{};

struct isPublicEvtHandler : std::true_type
{};


struct defFunctor
{
public:
	LRESULT operator()();

	HWND winHandle;
	UINT winMsg;
	WPARAM wparam;
	LPARAM lparam;
};

/* Base class for all windows */
class winClass
{
public:
	winClass(HINSTANCE _hinst, LPCWSTR _winClassName, LPCWSTR _menuName, HBRUSH _clsBackground, \
		HCURSOR _cursor, DWORD _style, HICON _icosm, HICON _ico);
	~winClass();

protected:
	WNDCLASSEX winClassAttrib;

	friend class winWindowCom;
};

template <class Listener>
using ListenerMethod = LRESULT(Listener::*)(defFunctor&, WPARAM, LPARAM);

struct _evtHandler;

template <class Listener>
struct EVENTCLASSMAPPER
{
	_evtHandler* binderInst;
	Listener* inst;
	ListenerMethod<Listener> method;
};



template <class Listener>
static std::unordered_map<UINT, std::list<EVENTCLASSMAPPER<Listener>>> evtSpecificClMap;

struct _evtHandler
{
public:
	//Window agnostic method to bind a class method to an event without regard to what type of window it is(BUTTON, TopWindow .. etc)
	template<class Listener>
	BOOL bind(UINT evt, Listener& inst, ListenerMethod<Listener> method)
	{
		if (!method && !(&inst)) return false;
		EVENTCLASSMAPPER<Listener> map = { this, &inst, method };

		bool mapped = false;
		for(auto& mapper : evtSpecificClMap<Listener>[evt])
			if (mapper.binderInst == this)
			{
				mapper = map;
				mapped = true;
				break;
			}

		if (!mapped) evtSpecificClMap<Listener>[evt].push_back(map);

		evtSuperMap[evt] = &_evtHandler::evtInvoker<Listener>;
		return true;
	}

protected:
	std::unordered_map<UINT, LRESULT(_evtHandler::*)(defFunctor&, UINT, WPARAM, LPARAM)> evtSuperMap;

	template<class Listener>
	LRESULT evtInvoker(defFunctor& evtDefActionFunctor, UINT evt, WPARAM wparam, LPARAM lparam)
	{
		for (const auto& mapper : evtSpecificClMap<Listener>[evt])
			if (mapper.binderInst == this)
			{
				return ((mapper.inst)->*(mapper.method))(evtDefActionFunctor, wparam, lparam);
			}
		return 0;
	}
};



struct isComCtlClass
{
	isComCtlClass();
private:
	static bool comCtlInitComplete;
};



class winWindowCom
{
public:
	winWindowCom(LPCWSTR _wClassName, LPCWSTR _windowName,
		DWORD _wStyle, int _xPos, int _yPos, int _wWidth, int _wHeight,
		HWND _pHandle, HMENU _idSmenu, HINSTANCE _modInst, void* windInst, bool useANSI);
	~winWindowCom();

	void writeText(WCHAR* newText);

	HWND parentHandle;
	HWND wWindHandle;
	HINSTANCE wInst;

private:
	LPCWSTR windowName;
	DWORD wStyle;
	int xPos;
	int yPos;
	int wWidth;
	int wHeight;
	HMENU menuHandle;

protected:
	template <typename ty1, typename ty2>
	LRESULT sendWindowMessage(UINT msg, ty1 par1, ty2 par2) const
	{
		return SendMessage(wWindHandle, msg, (WPARAM)par1, (LPARAM)par2);
	}

	static std::unordered_map<HWND, winWindowCom*> handleToInst;
	friend LRESULT CALLBACK winMessageDispatcher(HWND winHandle, UINT winMsg, WPARAM wparam, LPARAM lparam);

};

struct procHolder
{
	WNDPROC newProc, savedProc;
};

struct subClass
{
	void hookWindowProc(WNDPROC newProc);
private:
	virtual HWND getWinHandle() = 0;
	static std::unordered_map<HWND, procHolder> windProcMapper;

	friend LRESULT subClassComProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

//Use isPrivateEvtHandler specialisation of winWindow if creating your own window procedure(All kinds of event dispatch should be done manually)
template<class evtHandlerType = isPublicEvtHandler, bool = evtHandlerType::value>
class winWindow : public winWindowCom, public std::conditional_t<evtHandlerType::value, _evtHandler, subClass>
{
public:
	winWindow(LPCWSTR _wClassName, LPCWSTR _windowName,
		DWORD _wStyle, int _xPos, int _yPos, int _wWidth, int _wHeight,
		HWND _pHandle, HMENU _idSmenu, HINSTANCE _modInst, bool useANSI = false)
		: winWindowCom(_wClassName, _windowName, _wStyle, _xPos, _yPos, _wWidth, _wHeight
			, _pHandle, _idSmenu, _modInst, this, useANSI) {}
	int winExitCode = 0;
protected:
	HWND getWinHandle() { return wWindHandle; }
	friend LRESULT CALLBACK winMessageDispatcher(HWND winHandle, UINT winMsg, WPARAM wparam, LPARAM lparam);
};




