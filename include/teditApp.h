#pragma once
#include <Windows.h>
#include <shobjidl.h>
#include "winlib.h"
#include "teditEvents.h"
#include "dragNDrop.h"

struct teditEvents;
class dragNDrop;

enum class SAVESTATES
{
	FILE_SAVE,
	FILE_NOSAVE,
	FILE_CANCEL
};

enum class EDITCOMMAND
{
	CUT,
	COPY,
	DEL,
	PASTE
};

class teditApp
{
public:
	static teditApp* getAppInstance(wtopWindow& mainWindow, wMenu& menu, wStatusBar& statBar);
	static void startNewInstance();

	void registerAppEvtListener(teditEvents* evtListInst);
	void resizeWindow(int newWidth, int newHeight);
	void bindEventCallback(UINT evt, ListenerMethod<teditEvents> callback);
	
	void displayEditMenu();
	void undo();
	void selectAll();
	void editCommand(EDITCOMMAND command);

	void checkMarkMenuItem(UINT itemId);
	void clearMarkMenuItem(UINT itemId);
	
	void toggleWordWrap();
	void toggleStatusBar();

	size_t getNewLineNumber();
	size_t getCaretIndex();
	void updateLineCol(size_t line, size_t col);
	
	void hookTextBox(WNDPROC newProc);
	
	WCHAR* getText();
	void setText();
	void setText(WCHAR* &newText, size_t newSize);
	void setText(WCHAR* &&newText, size_t newSize);
	void clearText();
	
	COLORREF showFontDialog();
	void redrawTextBox();

	void startNewEditorSession();

	void showOpenDialog();
	void showAboutDialog();
	bool showSaveDialog(bool forceShow = false);
	void displayOpenFile(const wchar_t* fileName);

	void setDirtyFlag();
	void clearDirtyFlag();

	SAVESTATES showSavePrompt();

	void notifyAboutAccelerator(UINT msg, WPARAM wparam, LPARAM lparam);

	void shutdown();
	void exit(int exitCode);

private:

	teditApp(wtopWindow& mainWindow, wMenu& menu, wStatusBar& statBar);

	static size_t appInstances;
	static WCHAR* programPath;

	LONG statBarHeight = 0;
	bool wordWrap = false;
	bool sb = true;

	CHOOSEFONT userSelectedFontMetric;
	LOGFONT userFontSelected;
	HFONT fontHandle = NULL;
	bool fontDialogCalled = false;

	WCHAR* textBuf = nullptr;
	size_t textSize = 0;
	bool currentFileIsText = true;
	bool showSafetyBox = true;

	int wHeight = 0;
	int wWidth = 0;

	int argc = 0;
	LPWSTR* argv = nullptr;

	dragNDrop* dropTargetImpl = nullptr;
	wtopWindow& mmainWindow;
	wStatusBar& mstatBar;
	teditEvents* appEvtListener;
	textBox* mtext, *mtextAux, *activeTextBox;
	wMenu& mmenu;

	HANDLE file = INVALID_HANDLE_VALUE;
	LPWSTR openFileName = nullptr;
	IFileOpenDialog* fileOpenInterface;
	IFileSaveDialog* fileSaveInterface;

	bool dirty = false;
	std::wstring windowTitle;

	void setWindowTitle(const WCHAR* fileName);

	bool considerDirtyFlag();
	bool checkIfFileIsText();
	void parseCommandLine();

	bool saveToFile();
	void revertChanges();
	bool openFile(LPWSTR fileName);
	void setOpenFileName(LPWSTR fileName);
	void clearOpenFileName();
	bool openFileForOperation(LPWSTR fileName);
	bool showFileDialogInterface(LPWSTR& fileName, LPWSTR& displayName, IFileDialog* fileInterface);
	void cleanShellRemnants(LPWSTR& fileName, LPWSTR& displayName);

	void initFontMetrics();
};

