#pragma once
#include "winWindow.h"

//TextBox class handles events privately, which means winWindow::bind method will not be defined in textBox class 
class textBox : public winWindow<isPrivateEvtHandler>
{
public:
	textBox(winWindowCom* parent, size_t id, DWORD additionalStyles);
	
	size_t getLineNumber();
	size_t getCaretIndex();
	void setCaretIndex(size_t index);
	size_t getFirstCharIndex();
	
	bool getSelBox(DWORD& starting, DWORD& ending);
	void setSelBox(DWORD starting, DWORD ending);
	bool canUndo();
	void undo();
	void cut();
	void copy();
	void del();
	void paste();
	
	WCHAR* getText(size_t& bufSize);
	bool isModified();

	HFONT getFont();
	void setFont(HFONT fontHandle);
};

