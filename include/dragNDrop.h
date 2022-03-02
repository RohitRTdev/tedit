#pragma once
#include <oleidl.h>
#include "teditApp.h"

class teditApp;

class dragNDrop : public IDropTarget
{
public:
	dragNDrop(teditApp* inst);
	~dragNDrop();

	HRESULT QueryInterface(REFIID riid, void** ppvObject) override;
	ULONG AddRef() override;
	ULONG Release() override;

	HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;
	HRESULT DragLeave() override;
	HRESULT DragOver(DWORD  grfKeyState, POINTL pt, DWORD* pdwEffect) override;
	HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override;

private:
	teditApp* mainAppInst;
	bool supports = false;
	std::wstring shellFileName;

	ULONG refCount = 0;
};

