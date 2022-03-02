#include <exception>
#include <string>
#include <shobjidl.h>
#include <shellapi.h>
#include <assert.h>
#include "teditApp.h"
#include "dragNDrop.h"
#include "defs.h"
#include "utils.h"

using namespace std::literals;

size_t teditApp::appInstances = 0;
WCHAR* teditApp::programPath = nullptr;

SAVESTATES ans = SAVESTATES::FILE_SAVE;

INT_PTR Dlgproc(HWND dlgHandle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_COMMAND)
	{
		if (HIWORD(wparam) == BN_CLICKED)
		{
			switch (LOWORD(wparam))
			{
			case ID_SAVE:   
				ans = SAVESTATES::FILE_SAVE; 
				EndDialog(dlgHandle, true);
				break;
			case ID_NOSAVE: 
				ans = SAVESTATES::FILE_NOSAVE;
				EndDialog(dlgHandle, true);
				break;
			case ID_CANCEL: 
				EndDialog(dlgHandle, false);
			}
		}
	}
	else if (msg == WM_CLOSE)
	{
		EndDialog(dlgHandle, false);
	}
	return false;
}



teditApp::teditApp(wtopWindow& mainWindow, wMenu& menu, wStatusBar& statBar)
	: mmainWindow(mainWindow), mmenu(menu), mtext(new textBox(&mmainWindow, ID_TEXTBOX, HORSCROLL)),
	mtextAux(new textBox(&mmainWindow, ID_TEXTBOXAUX, 0)), activeTextBox(mtext),
	mstatBar(statBar), appEvtListener(nullptr)
{
	ShowWindow(activeTextBox->wWindHandle, SW_SHOW);
	initFontMetrics();

	(void)OleInitialize(NULL);

	(void)CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpenInterface));
	(void)CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&fileSaveInterface));

	setWindowTitle(L"<No Title>");

	static const COMDLG_FILTERSPEC teditFilterSpec[] = { { L"Plain Text(*.txt)", L"*.txt" }, {L"All files", L"*.*"} };
	fileOpenInterface->SetFileTypes(2, teditFilterSpec);
	fileSaveInterface->SetFileTypes(2, teditFilterSpec);
	fileOpenInterface->SetDefaultExtension(L"txt");
	fileSaveInterface->SetDefaultExtension(L"txt");
	parseCommandLine();

	dropTargetImpl = new dragNDrop(this);
	
	RegisterDragDrop(mmainWindow.wWindHandle, dropTargetImpl);

}

teditApp* teditApp::getAppInstance(wtopWindow& mainWindow, wMenu& menu, wStatusBar& statBar)
{
	if (!appInstances && ++appInstances)
	{
		_get_wpgmptr(&programPath);
		return new teditApp(mainWindow, menu, statBar);
	}
	else
		throw std::exception("Only 1 instance of teditApp allowed");
	
	return nullptr;
}

void teditApp::parseCommandLine()
{
	argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc > 1)
	{
		displayOpenFile(argv[1]);
	}
}

void teditApp::displayOpenFile(const wchar_t* fileName)
{
	if (!considerDirtyFlag()) return;
	DWORD size = GetFullPathName(fileName, 0, nullptr, nullptr);

	LPWSTR fileNameBuf = new WCHAR[static_cast<size_t>(size) + 1];
	LPWSTR displayName;

	if (GetFullPathName(fileName, size, fileNameBuf, &displayName))
	{
		setOpenFileName(fileNameBuf);
		setWindowTitle(displayName);

		if (!openFile(fileNameBuf))
		{
			revertChanges();
		}
	}

	delete[] fileNameBuf;
}

void teditApp::notifyAboutAccelerator(UINT msg, WPARAM wparam, LPARAM lparam)
{
	SendMessage(mmainWindow.wWindHandle, msg, wparam, lparam);
}

void teditApp::displayEditMenu()
{
	if (activeTextBox->canUndo())
		mmenu.enableItem(ID_UNDO);
	else
		mmenu.disableItem(ID_UNDO);
	
	DWORD starting, ending;
	if (activeTextBox->getSelBox(starting, ending))
	{
		mmenu.enableItem(ID_CUT);
		mmenu.enableItem(ID_COPY);
		mmenu.enableItem(ID_DELETE);
	}
	else
	{
		mmenu.disableItem(ID_CUT);
		mmenu.disableItem(ID_COPY);
		mmenu.disableItem(ID_DELETE);
	}

	if (IsClipboardFormatAvailable(CF_TEXT))
		mmenu.enableItem(ID_PASTE);
	else
		mmenu.disableItem(ID_PASTE);
		
}

void teditApp::editCommand(EDITCOMMAND command)
{
	switch (command)
	{
	case EDITCOMMAND::CUT:
		activeTextBox->cut();
		break;
	case EDITCOMMAND::COPY:
		activeTextBox->copy();
		break;
	case EDITCOMMAND::DEL:
		activeTextBox->del();
		break;
	case EDITCOMMAND::PASTE:
		activeTextBox->paste();
	}
}

void teditApp::undo()
{
	activeTextBox->undo();
}

SAVESTATES teditApp::showSavePrompt()
{
	MessageBeep(MB_OK);
	if (!DialogBoxParam(mmainWindow.wInst, MAKEINTRESOURCE(ID_SAVEDIALOG), mmainWindow.wWindHandle, Dlgproc, NULL))
		return SAVESTATES::FILE_CANCEL;
	else
		return ans;
}

bool teditApp::considerDirtyFlag()
{
	if (dirty)
	{
		SAVESTATES res = showSavePrompt();
		if (res == SAVESTATES::FILE_CANCEL) return false;

		if (res == SAVESTATES::FILE_SAVE)
		{
			if (!showSaveDialog()) return false;
		}
	}

	return true;
}

void teditApp::shutdown()
{
	if (!considerDirtyFlag()) return;

	mmainWindow.close(0);
	exit(0);
}

void teditApp::initFontMetrics()
{
	userSelectedFontMetric.lStructSize = sizeof(CHOOSEFONT);
	userSelectedFontMetric.hwndOwner = mmainWindow.wWindHandle;
	userSelectedFontMetric.lpLogFont = &userFontSelected;
	userSelectedFontMetric.Flags = CF_EFFECTS | CF_SCALABLEONLY | CF_TTONLY;
	userSelectedFontMetric.rgbColors = RGB(0, 0, 0);
}

COLORREF teditApp::showFontDialog()
{
	CHOOSEFONT newFont = userSelectedFontMetric;

	if (fontDialogCalled)
		newFont.Flags |= CF_INITTOLOGFONTSTRUCT;
	else
		fontDialogCalled = true;

	if (!ChooseFont(&newFont)) return userSelectedFontMetric.rgbColors;

	userSelectedFontMetric = newFont;
	if (fontHandle) DeleteObject(fontHandle);

	fontHandle = CreateFontIndirect(userSelectedFontMetric.lpLogFont);

	DeleteObject(activeTextBox->getFont());

	mtext->setFont(fontHandle); 
	mtextAux->setFont(fontHandle);

	return userSelectedFontMetric.rgbColors;
}


void teditApp::cleanShellRemnants(LPWSTR& fileName, LPWSTR& displayName)
{
	CoTaskMemFree(fileName);
	CoTaskMemFree(displayName);
}

void teditApp::startNewEditorSession()
{
	if (!considerDirtyFlag()) return;

	clearText();
	setWindowTitle(L"<No Title>");
	currentFileIsText = true;
	showSafetyBox = true;
	clearOpenFileName();
	mstatBar.writeText(2, L"\tUTF-8");

	if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
	file = INVALID_HANDLE_VALUE;
}

void teditApp::showOpenDialog()
{
	if (!considerDirtyFlag()) return;

	LPWSTR fileName, displayName;
	if (!showFileDialogInterface(fileName, displayName, fileOpenInterface)) return;
	if (!openFile(fileName))
	{
		goto cleanup;
	}

	setOpenFileName(fileName);
	setWindowTitle(displayName);

cleanup:
	cleanShellRemnants(fileName, displayName);
}
void teditApp::setOpenFileName(LPWSTR fileName)
{
	size_t size = wcslen(fileName);

	clearOpenFileName();
	openFileName = new WCHAR[size + 1];

	wcscpy_s(openFileName, size + 1, fileName);
}

void teditApp::clearOpenFileName()
{
	if (openFileName) delete[] openFileName;
	openFileName = nullptr;
}

bool teditApp::openFileForOperation(LPWSTR fileName)
{
	if (file != INVALID_HANDLE_VALUE) CloseHandle(file);

	if ((file = CreateFile(fileName, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		displayErrorWithoutExit(L"File open error", L"Requested file could not be opened");
		return false;
	}

	return true;
}
bool teditApp::openFile(LPWSTR fileName)
{
	if (!openFileForOperation(fileName)) return false;

	DWORD bytesRead = 0;
	LARGE_INTEGER sizeWrapper;
	LPWSTR textBuf16 = nullptr;
	GetFileSizeEx(file, &sizeWrapper);

	void* textBuf = new char[sizeWrapper.QuadPart + 1];

	if (!ReadFile(file, textBuf, static_cast<DWORD>(sizeWrapper.QuadPart), &bytesRead, NULL))
	{
		displayErrorWithoutExit(L"Operation failed", L"Read operation on the file failed");
		goto cleanup;
	}

	static_cast<char*>(textBuf)[sizeWrapper.QuadPart] = '\0';
	preprocessText(textBuf, sizeWrapper.QuadPart, currentFileIsText);
	if (!currentFileIsText)
		mstatBar.writeText(2, L"\tBinary");
	else
		mstatBar.writeText(2, L"\tUTF-8");

	size_t textBuf16Size;
	convertToUTF16(static_cast<char*>(textBuf), sizeWrapper.QuadPart + 1, &textBuf16, textBuf16Size);

	setText(std::move(textBuf16), textBuf16Size);
	delete[] textBuf;

	showSafetyBox = true;
	return true;

cleanup:	
	CloseHandle(file);
	file = INVALID_HANDLE_VALUE;
	return false;
}

bool teditApp::showFileDialogInterface(LPWSTR& fileName, LPWSTR& displayName, IFileDialog* fileInterface)
{
	if (fileInterface->Show(mmainWindow.wWindHandle) != S_OK) return false;

	IShellItem* shellPtr = nullptr;
	fileInterface->GetResult(&shellPtr);

	if (shellPtr->GetDisplayName(SIGDN_FILESYSPATH, &fileName) != S_OK)
	{
		displayErrorWithoutExit(L"File operation", L"Open operation failed");
		return false;
	}

	shellPtr->GetDisplayName(SIGDN_NORMALDISPLAY, &displayName);
	shellPtr->Release();

	return true;
}

void teditApp::showAboutDialog()
{
	MessageBox(mmainWindow.wWindHandle, L"T(Text) edit(editor) is a simple text editor you can use for day-day text editing tasks. \
Most of it's design guidelines are borrowed from notepad(So if you notice a lot of similarities to notepad, then yes, it's done on purpose).\
\n\nIn case of any unusual behaviour with the app, report at :- rohit20011002@gmail.com", L"tedit", MB_OK);
}

bool teditApp::showSaveDialog(bool forceShow)
{
	bool previouslyOpened = false;
	LPWSTR prevFileName = nullptr;
	if (openFileName)
	{
		size_t fileSize = wcslen(openFileName);
		prevFileName = new WCHAR[fileSize + 1];
		wcscpy_s(prevFileName, fileSize + 1, openFileName);
	}
		
	if (forceShow || !openFileName)
	{
		LPWSTR fileName, displayName;
		if (!showFileDialogInterface(fileName, displayName, fileSaveInterface)) return false;
		
		if (!openFileForOperation(fileName)) return false;

		setOpenFileName(fileName);
		setWindowTitle(displayName);
		cleanShellRemnants(fileName, displayName);

		setDirtyFlag();

		if (prevFileName && wcscmp(openFileName, prevFileName)) showSafetyBox = true;
	}
	else
	{
		previouslyOpened = true;
	}

	if (prevFileName) delete[] prevFileName;

	//If save failed(due to no write permission or some unexpected errors) then we simply start a new editor session instead of reverting to last open file
	if (!saveToFile())
	{
		if(!previouslyOpened)
			revertChanges();
		return false;
	}

	return true;
}

void teditApp::revertChanges()
{
	clearDirtyFlag();
	startNewEditorSession();
}


bool teditApp::saveToFile()
{
	if (!dirty) return true;
	assert(file != INVALID_HANDLE_VALUE);

	if (!checkIfFileIsText()) return false;
	
	char* text8 = nullptr;
	size_t text8Size = 0;
	DWORD bytesWritten = 0;
	bool opStat = false;
	 
	//Reopen file for overwriting
	CloseHandle(file);
	if ((file = CreateFile(openFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		displayErrorWithoutExit(L"File operation", L"User doesn't have file write permission");
		goto cleanup;
	}

	mstatBar.writeText(0, L"\tSaving text...");
	getText();
	
	if (textSize)
	{
		postprocessText(textBuf, textSize);
		convertToUTF8(textBuf, textSize + 1, &text8, text8Size);
	}
	
	if (text8Size && !WriteFile(file, text8, static_cast<DWORD>(text8Size - 1), &bytesWritten, NULL)) //We don't want to write the NULL character
	{
		displayErrorWithoutExit(L"File operation", L"File write operation failed");
		goto cleanup;
	}
	
	if(text8Size)
		delete[] text8;
	clearDirtyFlag();
	mstatBar.writeText(0, L"");
	opStat = true;

cleanup:
	//Reopen file with read capability
	CloseHandle(file);
	if ((file = CreateFile(openFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		displayError(L"File operation", L"An unexpected error has occured", MB_ICONSTOP);
	return opStat;

}

bool teditApp::checkIfFileIsText()
{
	if (!showSafetyBox) return true;
	if (!currentFileIsText)
	{
		MessageBeep(MB_OK);
		int res = MessageBox(mmainWindow.wWindHandle, L"Modifying a non text file with a text editor is unsafe.\
 Consider using a binary editor.Do you want to proceed anyway?", L"tedit", MB_YESNO);

		if (res == IDYES) showSafetyBox = false;
		return res == IDYES;
	}

	return true;
}

void teditApp::setWindowTitle(const WCHAR* fileName)
{
	clearDirtyFlag();
	windowTitle = std::wstring(fileName) + L" - tedit"s;
	mmainWindow.writeText(const_cast<WCHAR*>(windowTitle.c_str()));
}

void teditApp::setDirtyFlag()
{
	WCHAR* dum;
	if (!(dum = getText()) && file == INVALID_HANDLE_VALUE)
	{
		clearDirtyFlag();
		return;
	}

	if (dirty) return;
	windowTitle.insert(0, L"*");

	mmainWindow.writeText(const_cast<WCHAR*>(windowTitle.c_str()));

	dirty = true;
}

void teditApp::clearDirtyFlag()
{
	if (!dirty) return;
	windowTitle.erase(0, 1);

	mmainWindow.writeText(const_cast<WCHAR*>(windowTitle.c_str()));

	dirty = false;
}

void teditApp::toggleStatusBar()
{
	if (sb = !sb) 
		ShowWindow(mstatBar.wWindHandle, SW_SHOW);
	else 
		ShowWindow(mstatBar.wWindHandle, SW_HIDE);

	resizeWindow(wWidth, wHeight);
}

void teditApp::selectAll()
{
	activeTextBox->setSelBox(0, -1);
}

size_t teditApp::getNewLineNumber()
{
	return activeTextBox->getLineNumber() + 1;
}

size_t teditApp::getCaretIndex()
{
	DWORD startPos, endPos;
	activeTextBox->getSelBox(startPos, endPos);

	return endPos - activeTextBox->getFirstCharIndex() + 1;
}

void teditApp::redrawTextBox()
{
	getText();
	setText(); 
	resizeWindow(wWidth, wHeight);
	activeTextBox->setCaretIndex(0);
}

WCHAR* teditApp::getText()
{
	if (textBuf && !activeTextBox->isModified()) return textBuf;

	if(textBuf)
		delete[] textBuf;
	
	textBuf = activeTextBox->getText(textSize);
	return textBuf;
}

void teditApp::setText()
{
	if (!textBuf) clearText();
	activeTextBox->writeText(textBuf);
}

void teditApp::setText(WCHAR* &newText, size_t newSize)
{
	textSize = newSize;
	
	if (textBuf) delete[] textBuf;
	textBuf = new WCHAR[newSize + 1];
	wcscpy_s(textBuf, newSize + 1, newText);

	activeTextBox->writeText(textBuf);
}

void teditApp::setText(WCHAR* &&newText, size_t newSize)
{
	textSize = newSize;
	if (textBuf) delete[] textBuf;

	textBuf = newText;
	activeTextBox->writeText(textBuf);
}


void teditApp::clearText()
{
	if (textBuf) delete[] textBuf;
	
	textBuf = nullptr;
	activeTextBox->writeText(const_cast<WCHAR*>(L""));
}

void teditApp::hookTextBox(WNDPROC newProc)
{
	mtext->hookWindowProc(newProc);
	mtextAux->hookWindowProc(newProc);
}

void teditApp::updateLineCol(size_t line, size_t col)
{
	std::wstring statText = L"\tLn "s + std::to_wstring(line) + L",Col " + std::to_wstring(col);
	mstatBar.writeText(1, statText);
}

void teditApp::registerAppEvtListener(teditEvents* evtListInst)
{
	appEvtListener = evtListInst;
}

void teditApp::exit(int exitCode)
{
	mmainWindow.close(exitCode);
}

void teditApp::bindEventCallback(UINT evt, ListenerMethod<teditEvents> callback)
{
	mmainWindow.bind<teditEvents>(evt, *appEvtListener, callback);
}

void teditApp::resizeWindow(int newWidth, int newHeight)
{
	statBarHeight = sb ? mstatBar.resize() : 0;
	int estHeight = newHeight > statBarHeight ? newHeight - statBarHeight : 0;

	MoveWindow(activeTextBox->wWindHandle, 0, 0, newWidth, estHeight, true);
	wWidth = newWidth; wHeight = newHeight;
}

void teditApp::checkMarkMenuItem(UINT itemId)
{
	mmenu.checkItemMark(itemId);
}

void teditApp::clearMarkMenuItem(UINT itemId)
{
	mmenu.clearItemMark(itemId);
}

void teditApp::startNewInstance()
{
	ShellExecute(NULL, L"open", programPath, NULL, NULL, SW_SHOWDEFAULT);
}


void teditApp::toggleWordWrap()
{
	getText();
	ShowWindow(activeTextBox->wWindHandle, SW_HIDE);

	if (wordWrap = !wordWrap) activeTextBox = mtextAux;
	else activeTextBox = mtext;

	ShowWindow(activeTextBox->wWindHandle, SW_SHOW);

	setText();
	resizeWindow(wWidth, wHeight);
	activeTextBox->setCaretIndex(0);
}

void preprocessText(void* textBuf, size_t textSize, bool& isText)
{
	isText = true;
	for (size_t i = 0; i < textSize; i++)
	{
		if (static_cast<char*>(textBuf)[i] == 0)
		{
			isText = false;
			static_cast<char*>(textBuf)[i] = 1;
		}
	}
}

void postprocessText(void* textBuf, size_t textSize)
{
	for (size_t i = 0; i < textSize; i++)
	{
		if (static_cast<char*>(textBuf)[i] == 1)
			static_cast<char*>(textBuf)[i] = 0;
	}
}

void convertToUTF16(char* utf8Text, size_t numChars, LPWSTR* utf16Text, size_t& textSize)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, utf8Text, static_cast<int>(numChars), *utf16Text, 0);

	*utf16Text = new WCHAR[size];
	MultiByteToWideChar(CP_UTF8, 0, utf8Text, static_cast<int>(numChars), *utf16Text, size);
	textSize = size;
}

void convertToUTF8(LPWSTR utf16Text, size_t numChars, char** utf8Text, size_t& textSize)
{
	size_t size = WideCharToMultiByte(CP_UTF8, 0, utf16Text, static_cast<int>(numChars), *utf8Text, 0, 0, 0);

	*utf8Text = new char[size + 1];
	WideCharToMultiByte(CP_UTF8, 0, utf16Text, static_cast<int>(numChars), *utf8Text, static_cast<int>(size), 0, 0);
	textSize = size;
}