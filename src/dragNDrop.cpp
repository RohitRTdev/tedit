#include "dragNDrop.h"
#include <Shlobj.h>
dragNDrop::dragNDrop(teditApp* inst) : mainAppInst(inst)
{
}


HRESULT dragNDrop::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	IEnumFORMATETC* enumerator = nullptr;
	*pdwEffect = DROPEFFECT_NONE;

	if (pDataObj->EnumFormatEtc(DATADIR_GET, &enumerator) != S_OK) return S_OK;

	FORMATETC formatItem;
	while (enumerator->Next(1, &formatItem, NULL) == S_OK)
	{
		
		if ((formatItem.tymed == TYMED_HGLOBAL && formatItem.cfFormat == CF_HDROP) || formatItem.tymed == TYMED_FILE)
		{
			if (pDataObj->QueryGetData(&formatItem) == S_OK)
			{
				STGMEDIUM medium;
				if (pDataObj->GetData(&formatItem, &medium) == S_OK)
				{
					if (medium.tymed == TYMED_HGLOBAL)
					{
						DROPFILES* fileListing = reinterpret_cast<DROPFILES*>(GlobalLock(medium.hGlobal));
						LPWSTR file = nullptr;
						UINT fileSize = DragQueryFile(reinterpret_cast<HDROP>(fileListing), 0, nullptr, 0);

						file = new WCHAR[fileSize + 2];

						DragQueryFile(reinterpret_cast<HDROP>(fileListing), 0, file, fileSize+1);
						shellFileName = file;
						delete[] file;
						
						supports = true;
					}
					else if (medium.tymed == TYMED_FILE)
					{
						shellFileName = medium.lpszFileName;
						supports = true;
					}
					
					ReleaseStgMedium(&medium);
					break;
				}
			}
		}
	}
	enumerator->Release();
	if (!supports) return S_OK;


	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT dragNDrop::DragLeave()
{
	supports = false;
	shellFileName.erase();

	return S_OK;
}

HRESULT dragNDrop::DragOver(DWORD  grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = supports ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	return S_OK;
}

HRESULT dragNDrop::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if(supports) mainAppInst->displayOpenFile(shellFileName.c_str());
	DragLeave();
	return S_OK;
}


HRESULT dragNDrop::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOINTERFACE;
}

ULONG dragNDrop::AddRef()
{
	return ++refCount;
}

ULONG dragNDrop::Release()
{
	if (!refCount) return 0;

	if (!(--refCount)) dragNDrop::~dragNDrop();
	return refCount;
}

dragNDrop::~dragNDrop()
{
	DragLeave();
}
