#include "textBox.h"
#include <CommCtrl.h>


textBox::textBox(winWindowCom* parent, size_t id, DWORD additionalStyles) 
	: winWindow<isPrivateEvtHandler>(L"EDIT", 
		NULL, WS_CHILD | WS_VSCROLL | ES_LEFT | ES_NOHIDESEL | ES_WANTRETURN | ES_AUTOVSCROLL | ES_MULTILINE | additionalStyles
		, 0, 0, 0, 0, 
		parent->wWindHandle, reinterpret_cast<HMENU>(id), parent->wInst)
{ 
}

size_t textBox::getLineNumber()
{
	return sendWindowMessage(EM_LINEFROMCHAR, -1, 0);
}

size_t textBox::getCaretIndex()
{
	return sendWindowMessage(EM_LINEINDEX, -1, 0);
}

void textBox::setCaretIndex(size_t index)
{
	sendWindowMessage(WM_LBUTTONDOWN, MK_LBUTTON, 0);
	sendWindowMessage(WM_LBUTTONUP, MK_RBUTTON, 0);
}

size_t textBox::getFirstCharIndex()
{
	return sendWindowMessage(EM_LINEINDEX, -1, 0);
}

bool textBox::getSelBox(DWORD& starting, DWORD& ending)
{
	sendWindowMessage(EM_GETSEL, &starting, &ending);

	return starting != ending;
}

void textBox::setSelBox(DWORD starting, DWORD ending)
{
	sendWindowMessage(EM_SETSEL, starting, ending);
}

WCHAR* textBox::getText(size_t& bufSize)
{
	bufSize = sendWindowMessage(WM_GETTEXTLENGTH, 0, 0);
	
	if (!bufSize) return nullptr;

	WCHAR* textBuf = new WCHAR[bufSize + 1];

	sendWindowMessage(WM_GETTEXT, bufSize + sizeof(L'\0'), textBuf);

	return textBuf;
}

HFONT textBox::getFont()
{
	return reinterpret_cast<HFONT>(sendWindowMessage(WM_GETFONT, 0, 0));
}

void textBox::setFont(HFONT fontHandle)
{
	sendWindowMessage(WM_SETFONT, fontHandle, true);
}

bool textBox::isModified()
{
	return sendWindowMessage(EM_GETMODIFY, 0, 0);
}

bool textBox::canUndo()
{
	return sendWindowMessage(EM_CANUNDO, 0, 0);
}

void textBox::undo()
{
	sendWindowMessage(EM_UNDO, 0, 0);
}

void textBox::cut()
{
	sendWindowMessage(WM_CUT, 0, 0);
}

void textBox::copy()
{
	sendWindowMessage(WM_COPY, 0, 0);
}

void textBox::del()
{
	sendWindowMessage(WM_CLEAR, 0, 0);
}

void textBox::paste()
{
	sendWindowMessage(WM_PASTE, 0, 0);
}


