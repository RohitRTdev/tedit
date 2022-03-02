#pragma once
#include "winWindow.h"
#include "utils.h"

class wMenu
{
public:
	enum menuType
	{
		MENU_SEP,
		MENU_STR,
		MENU_SUB
	};

	wMenu(HMENU _menuHandle);
	wMenu(bool popMenu);
	wMenu();
	~wMenu();

	BOOL insertItem(menuType type, UINT id = 0, const std::wstring& menuItemName = std::wstring(), const wMenu& subMenu = wMenu());
	
	BOOL checkItemMark(UINT id);
	BOOL clearItemMark(UINT id);
	
	void disableItem(UINT id);
	void enableItem(UINT id);
private:
	HMENU menuHandle, shortcutMenu = 0;
	HWND parent;
	BOOL drawMenu(HWND parentHandle);
	BOOL popupMenu;

	friend class wtopWindow;
};

