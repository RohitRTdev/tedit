#include "teditEvents.h"
#include "defs.h"

static teditEvents* globalEvtListenerInst = nullptr;

static LRESULT textBoxProcHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        globalEvtListenerInst->notifyPotentialAcceleratorKey(msg, wparam, lparam);
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: 
    case WM_RBUTTONUP:
        globalEvtListenerInst->onTextUpdate();
    }        
    return 0;
}

teditEvents::teditEvents(teditApp* appInst) : app(appInst), userColourChoice(RGB(0,0,0)), wordWrap(false), sbHidden(false)
{
    globalEvtListenerInst = this;
    app->registerAppEvtListener(this);
    
    app->bindEventCallback(WM_SIZE,          &teditEvents::onResize);
    app->bindEventCallback(WM_COMMAND,       &teditEvents::onCommand);
    app->bindEventCallback(WM_CTLCOLOREDIT,  &teditEvents::onTextBoxDrawUpdate);
    app->bindEventCallback(WM_CLOSE,         &teditEvents::onClose);
    app->bindEventCallback(WM_INITMENUPOPUP, &teditEvents::onMenuPopup);

    app->updateLineCol(lineCount, col);

    //Simply binding to keydown event on main window won't work as keydown messages are sent to and processed by textBox
    //Thus we need to subclass the textBox window
    app->hookTextBox(textBoxProcHook);    
}

void teditEvents::notifyPotentialAcceleratorKey(UINT msg, WPARAM wparam, LPARAM lparam)
{
    app->notifyAboutAccelerator(msg, wparam, lparam);
}

LRESULT teditEvents::onMenuPopup(defFunctor& defAction, WPARAM wparam, LPARAM lparam)
{
    if (!HIWORD(lparam) && LOWORD(lparam) == 1) //Opened edit menu
    {
        app->displayEditMenu();
        return 0;
    }

    return defAction();
}

LRESULT teditEvents::onClose(defFunctor& defAction, WPARAM wparam, LPARAM lparam)
{
    app->shutdown();
    return 0;
}

LRESULT teditEvents::onTextBoxDrawUpdate(defFunctor& defAction, WPARAM wparam, LPARAM lparam)
{
    SetTextColor(reinterpret_cast<HDC>(wparam), userColourChoice);

    return GetBkColor(reinterpret_cast<HDC>(wparam));
}

LRESULT teditEvents::onTextUpdate()
{
    lineCount = app->getNewLineNumber();
    col = app->getCaretIndex();

    app->updateLineCol(lineCount, col);
    return 0;
}


LRESULT teditEvents::onResize(defFunctor& defAction, WPARAM wparam, LPARAM lparam)
{
    app->resizeWindow(LOWORD(lparam), HIWORD(lparam));
    return 0;
}

LRESULT teditEvents::onCommand(defFunctor& defAction, WPARAM wparam, LPARAM lparam)
{
    switch (HIWORD(wparam))
    {
    case EN_UPDATE:
        app->setDirtyFlag();
        return 0;
    }
    if (!lparam) // Menu commands and accelerator keys
    {
        switch (LOWORD(wparam))
        {
        case ID_EXIT: 
            app->exit(0); 
            return 0;
        case ID_NEWWINDOW: 
            teditApp::startNewInstance();
            return 0;
        case ID_WORDWRAP:
            wordWrap ? app->clearMarkMenuItem(ID_WORDWRAP) : app->checkMarkMenuItem(ID_WORDWRAP);
            wordWrap = !wordWrap;
            app->toggleWordWrap();
            return 0;
        case ID_FONT: 
            userColourChoice = app->showFontDialog();
            app->redrawTextBox();
            onTextUpdate();
            return 0;
        case ID_OPEN:
            app->showOpenDialog();
            return 0;
        case ID_SAVE:
            app->showSaveDialog();
            return 0;
        case ID_SAVEAS:
            app->showSaveDialog(true);
            return 0;
        case ID_NEW:
            app->startNewEditorSession();
            return 0;
        case ID_UNDO:
            app->undo();
            return 0;
        case ID_SELECTALL:
            app->selectAll();
            return 0;
        case ID_STATUSBARMENU:
            sbHidden ? app->checkMarkMenuItem(ID_STATUSBAR) : app->clearMarkMenuItem(ID_STATUSBAR);
            sbHidden = !sbHidden;
            app->toggleStatusBar();
            return 0;
        case ID_CUT:
            app->editCommand(EDITCOMMAND::CUT);
            return 0;
        case ID_COPY:
            app->editCommand(EDITCOMMAND::COPY);
            return 0;
        case ID_DELETE:
            app->editCommand(EDITCOMMAND::DEL);
            return 0;
        case ID_PASTE:
            app->editCommand(EDITCOMMAND::PASTE);
            return 0;
        case ID_ABOUT:
            app->showAboutDialog();
            return 0;
        }
    }


    return defAction();
}