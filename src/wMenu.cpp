#include "wMenu.h"
#include <Windows.h>


wMenu::wMenu() : parent(0), menuHandle(0), popupMenu(false)
{
	
}

wMenu::wMenu(HMENU _menuHandle) : menuHandle(_menuHandle), parent(0), popupMenu(false)
{

}

wMenu::wMenu(bool popup) : popupMenu(popup), parent(0)
{
	menuHandle = popup ? CreatePopupMenu() : CreateMenu();
	if (!menuHandle) throw std::exception("Menu creation failed!");
}

BOOL wMenu::insertItem(wMenu::menuType type, UINT id, const std::wstring& menuItemName, const wMenu& subMenu)
{

	if (type == MENU_SEP && !popupMenu) throw std::exception("MENU_SEP should be used only in popup menus");

	MENUITEMINFO menuInfo = {};
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = type == MENU_SUB ? MIIM_SUBMENU | MIIM_STRING: MIIM_TYPE | MIIM_ID;
	menuInfo.fType = type == MENU_SEP ? MFT_SEPARATOR : MFT_STRING;
	menuInfo.wID = id;
	menuInfo.hSubMenu = subMenu.menuHandle;
	menuInfo.dwTypeData = const_cast<LPWSTR>(menuItemName.c_str());
	menuInfo.cch = static_cast<UINT>(menuItemName.size());

	BOOL opStat = InsertMenuItem(menuHandle, id, FALSE, &menuInfo);
	if (parent) drawMenu(parent);

	return opStat;
}

BOOL wMenu::checkItemMark(UINT id)
{
	MENUITEMINFO menuInfo = {};
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
	menuInfo.fState = MFS_CHECKED;

	return SetMenuItemInfo(menuHandle, id, false, &menuInfo);
}

BOOL wMenu::clearItemMark(UINT id)
{
	MENUITEMINFO menuInfo = {};
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
	menuInfo.fState = MFS_UNCHECKED;

	return SetMenuItemInfo(menuHandle, id, false, &menuInfo);
}

void wMenu::disableItem(UINT id)
{
	MENUITEMINFO menuInfo = {};
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
	menuInfo.fState = MFS_GRAYED;

	SetMenuItemInfo(menuHandle, id, false, &menuInfo);
}

void wMenu::enableItem(UINT id)
{
	MENUITEMINFO menuInfo = {};
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
	menuInfo.fState = MFS_ENABLED;

	SetMenuItemInfo(menuHandle, id, false, &menuInfo);
}

BOOL wMenu::drawMenu(HWND parentHandle)
{
	parent = parentHandle;
	return DrawMenuBar(parentHandle);
}


wMenu::~wMenu()
{
	if(!parent && menuHandle)
		DestroyMenu	(menuHandle);
}