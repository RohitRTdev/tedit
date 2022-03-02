#pragma once
#include "winlib.h"
#include "teditApp.h"

//forward references to calm down dependency errors in some translation units
class teditApp;

struct teditEvents
{
	teditEvents(teditApp* appInst);

	LRESULT onResize(defFunctor& defAction, WPARAM wparam, LPARAM lparam);
	LRESULT onCommand(defFunctor& defAction, WPARAM wparam, LPARAM lparam);
	LRESULT onKeyDown(defFunctor& defAction, WPARAM wparam, LPARAM);
	LRESULT onTextBoxDrawUpdate(defFunctor& defAction, WPARAM wparam, LPARAM lparam);
	LRESULT onClose(defFunctor& defAction, WPARAM wparam, LPARAM lparam);
	LRESULT onMenuPopup(defFunctor& defAction, WPARAM wparam, LPARAM lparam);

	void notifyPotentialAcceleratorKey(UINT msg, WPARAM wparam, LPARAM lparam);
private:
	teditApp* app;
	bool wordWrap;
	bool sbHidden;
	size_t lineCount = 1, col = 1, index = 0;

	COLORREF userColourChoice;

	LRESULT onTextUpdate();
	friend LRESULT textBoxProcHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

