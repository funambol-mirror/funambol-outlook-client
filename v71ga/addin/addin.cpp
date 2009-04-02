/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2003 - 2007 Funambol, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License version 3 as published by
 * the Free Software Foundation with the addition of the following permission 
 * added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 * WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE 
 * WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Affero General Public License 
 * along with this program; if not, see http://www.gnu.org/licenses or write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 * 
 * You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite 
 * 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 * 
 * The interactive user interfaces in modified source and object code versions
 * of this program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU Affero General Public License version 3.
 * 
 * In accordance with Section 7(b) of the GNU Affero General Public License
 * version 3, these Appropriate Legal Notices must retain the display of the
 * "Powered by Funambol" logo. If the display of the logo is not reasonably 
 * feasible for technical reasons, the Appropriate Legal Notices must display
 * the words "Powered by Funambol".
 */

#include "stdafx.h"
#include "base/Log.h"
#include "base/util/utils.h"
#include "spdm/DMTreeFactory.h"
#include "Client/DMTClientConfig.h"

#include "FunambolAddin.h"
#include "addin.h"

using namespace Funambol;

STARTUPINFOA         si;
PROCESS_INFORMATION  pi;
DWORD         processId;

// The hook handle for keyboards events.
HHOOK hkb;

Caddin* funAddin;

CComQIPtr <_CommandBarButton> pFunButton;       // The CommandBar button    (icon)
CComQIPtr <_CommandBarButton> pFunSync;         // The Synchronize button   (menu)
CComQIPtr <_CommandBarButton> pFunGoto;         // The Goto button          (menu)
CComQIPtr <_CommandBarButton> pFunConfig;       // The Configuration button (menu)


/////////////////////////////////////////////////////////////////////////////
// Caddin
_ATL_FUNC_INFO OnClickButtonInfo          ={CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_BOOL}};
_ATL_FUNC_INFO OnClickSynchronizationInfo ={CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_BOOL}};
_ATL_FUNC_INFO OnClickGotoInfo            ={CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_BOOL}};
_ATL_FUNC_INFO OnClickConfigurationInfo   ={CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_BOOL}};


STDMETHODIMP Caddin::InterfaceSupportsErrorInfo(REFIID riid) {

    static const IID* arr[] =
    {
        &IID_Iaddin
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++) {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return E_FAIL;
}


STDMETHODIMP Caddin::OnConnection(IDispatch * Application, ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY * * custom) {

    m_pParentApp = Application;

    if (ConnectMode != ext_cm_Startup)
        OnStartupComplete(custom);

    return S_OK;
}



void copyIconIntoClip() {

    HICON hIco = (HICON)::LoadImage(_Module.GetResourceInstance(),
                 MAKEINTRESOURCE(IDI_ICON1), 
                 IMAGE_ICON, 0, 0, 
                 LR_LOADTRANSPARENT |  LR_LOADMAP3DCOLORS);

    ICONINFO oIconInfo;
    GetIconInfo(hIco, &oIconInfo);

    // put Icon into Clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, oIconInfo.hbmColor);
    CloseClipboard();
    DeleteObject(hIco);
}


/**
 * Called when some keyboard event occurrs.
 * Capture keyboard combinations:
 *   CTRL+S  -> Sync All
 *   CTRL+G  -> Open plugin (Go to)
 *   CTRL+T  -> Options
 */
LRESULT CALLBACK OnKeyboardEvent(int code, WPARAM wParam, LPARAM lParam) {
    //LOG.debug("code = %d, wparam = 0x%x, lparam = 0x%08x", code, (DWORD)wParam, (DWORD)lParam);

    //
    // Catch only keystroke messages.
    // http://blogs.msdn.com/michkap/archive/2006/03/23/558658.aspx
    //
    if ( ((DWORD)lParam & 0x40000000) && (HC_ACTION==code) ) {
        
        // Check if CTRL is down (high order bit = 1)
        if (GetKeyState(VK_CONTROL) & 0x10000000) {
            
            // CTRL+F7 -> button "Sync All"     (0x76 = F7)            
            if (wParam == 0x76) {  
                LOG.debug("CTRL+S captured -> SYNC ALL");
                if (funAddin) {
                    funAddin->launchSyncClientOutlook(PARAM_OUTLOOK_SYNC);
                }
            }
            //  CTRL+F8 -> button "Go To"  (0x77 = F8)
            else if (wParam == 0x77) { 
                LOG.debug("CTRL+G captured -> GO TO");
                if (funAddin) {
                    funAddin->launchSyncClientOutlook(NULL);
                }
            }
            //  CTRL+F9 -> button "Go To"  (0x78 = F9)
            else if (wParam == 0x78) { 
                LOG.debug("CTRL+T captured -> CONFIGURATION");
                if (funAddin) {
                    funAddin->launchSyncClientOutlook(PARAM_OUTLOOK_OPTIONS);
                }
            }
        }
    }

    LRESULT RetVal = CallNextHookEx(hkb, code, wParam, lParam);
    return  RetVal;
}




STDMETHODIMP Caddin::OnStartupComplete(LPSAFEARRAY* custom) {

    _ExplorerPtr            spExplorer;
    _CommandBarsPtr         spCmdBars;
    CommandBarPtr           spCmdBar;
    CommandBarControlPtr    commandBarControlPtr;
    CommandBarControlsPtr   spCmdCtrls;
    CommandBarControlPtr    spCmdCtrl;

    CComVariant vtEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);
    HRESULT hr = S_OK;
    bool createdCommandBar = false;
    bool createdMenuBar    = false;


    openLog();
    LOG.info("---------- OUTLOOK STARTUP ----------");


    //
    // Open Outlook Application
    //
    try {
        _ApplicationPtr spApp(m_pParentApp);
        hr = spApp->ActiveExplorer(&spExplorer);
        if (FAILED(hr)) {
            setErrorF(getLastErrorCode(), ERR_OPEN_EXPLORER);
            //sprintf(lastErrorMsg, ERR_OPEN_EXPLORER);
            goto error;
        }

        applicationPtr = spApp;

        hr = applicationPtr->ActiveExplorer(&spExplorer);
        if (FAILED(hr)) {
            setErrorF(getLastErrorCode(), ERR_OPEN_APPLICATION);
            //sprintf(lastErrorMsg, ERR_OPEN_APPLICATION);
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OPEN_APPLICATION);
        //sprintf(lastErrorMsg, ERR_OPEN_APPLICATION);
        goto error;
    }

    //
    // Get CommandBars
    //
    try {
        hr = spExplorer->get_CommandBars(&spCmdBars);
        if (FAILED(hr)) {
            setErrorF(getLastErrorCode(), ERR_GET_COMMANDBARS);
            //sprintf(lastErrorMsg, ERR_GET_COMMANDBARS);
            goto error;
        }
    }
    catch(_com_error &e) {
        // This happens if Outlook UI not loaded (e.g. bkground logon)
        // So we don't track errors.
        if (e.Error() == ERR_CODE_BAD_POINTER) {
            LOG.debug("Outlook UI not loaded -> exit");
            return S_FALSE;
        }
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_GET_COMMANDBARS);
        //sprintf(lastErrorMsg, ERR_GET_COMMANDBARS);
        goto error;
    }


    //
    // Check if addin is curently installed on Outlook UI.
    //
    int addinState = -1;                // addin "not found"
    if (isAddinInstalled()) {
        addinState = 0;                 // addin "installed"
    }
    else {
        addinState = 3;                 // addin "uninstalled"
    }


    //
    // -------- Correct state, based on READ-ONLY informations from HKLM --------
    // If HKLM key found, plugin is installed         
    //         -> force "installing" if state is "uninstalled"                                              
    //         -> force "installing" if old version is not compatible
    //         -> force "installing" if last time the addin produced an error
    // If HKLM key not found, plugin is not installed 
    //         -> force "uninstalling" if state is "installed"
    //
    char* oldSwv = NULL;
    char* swv = readPropertyValueFromHKLM(PLUGIN_CONTEXT, PROPERTY_SW_VERSION);
    if (swv && strcmp(swv, "")) {
        //
        // Plugin is installed.
        //
        if (addinState == 3) {
            // addin "uninstalled" -> wrong! force "installing"
            LOG.debug("Detected Outlook Sync Client installed -> state = installing");
            addinState = 1;
        }
        else {
            // addin "installed" -> check if software version is changed: could need to reinstall addin.
            oldSwv = readAddinSwv();
            if (!oldSwv || !strcmp(oldSwv, "")) {     
                // addin HKCU key not found -> plugin removed it during uninstall -> reinstall
                LOG.debug("No info found about old version installed -> state = installing");
                addinState = 1;
            }
            else if (swvNotCompatible(swv, oldSwv)) {
                // Version not compatible -> force "installing"
                LOG.debug("Detected not compatible version of Addin (was %s) -> state = installing", oldSwv);
                addinState = 1;
            }
            else if (checkErrorsLastTime()) {
                // Last time the addin produced an error -> force "installing"
                LOG.debug("Detected an error on last execution -> state = installing");
                addinState = 1;
            }
        }
    }
    else {
        //
        // Plugin is NOT installed.
        //
        if (addinState == 0) {
            // addin "installed" -> wrong! force "uninstalling"
            LOG.debug("Outlook Sync Client not detected -> state = uninstalling");
            addinState = 2;
        }
    }
    if (swv) {
        delete[] swv;    swv = NULL;
    }
    if (oldSwv) {
        delete[] oldSwv; oldSwv = NULL;
    }


    saveAddinState(ADDIN_STATE_IN_PROGRESS);


    //
    // Check current addin State:
    // --------------------------
    //  0 = installed      : nothing to do                          -> break
    // -1 = not found      :                                        -> goto state 1
    //  1 = installing     : remove old (if exist) addin            -> break
    //  2 = uninstalling   : remove addin                           -> goto state 3
    //  3 = uninstalled    : unreg dll (if last one)                -> exit
    //  ? = default        : remove addin                           -> exit
    //                       [this should not happen...]
    //
    switch (addinState) {
        case 0: 
        {
            LOG.info("Addin state: installed");
            break;
        }
        case -1:
            LOG.info("Reg key not found -> installing");
        case  1:
        {
            LOG.info("Addin state: installing");
            removeAddin();
            break;
        }
        case 2:
        {
            LOG.info("Addin state: uninstalling");
            removeAddin();
        }
        case 3:
        {
            LOG.info("Addin state: uninstalled");
            if (isLastInstance()) {
                LOG.info("Last addin detected, try to unregister myself...");
                hr = DllUnregisterServer();
                if (FAILED(hr)) {
                    setErrorF(getLastErrorCode(), ERR_UNREG_DLL);
                    //sprintf(lastErrorMsg, ERR_UNREG_DLL);
                    LOG.info(getLastErrorMsg());
                }
            }
            saveAddinState(ADDIN_STATE_UNINSTALLED);
            return hr;
        }
        default:
        {
            LOG.error("Addin state = %d (not a correct state)", addinState);
            removeAddin();
            LOG.info("Set state = uninstalled");
            saveAddinState(ADDIN_STATE_UNINSTALLED);
            return hr;
        }
    }


    LOG.info("Installing Outlook Addin...");

    try {
        //
        // Verify if CommandBar already exists
        //
        hr = spCmdBars->get_Item(CComVariant(PROGRAM_NAME), &spCmdBar);
        if (SUCCEEDED(hr)) {
            LOG.debug("CommandBar already exist -> use it.");
            commandBarControlPtr = spCmdBar->FindControl(CComVariant(msoControlButton), vtEmpty, 
                                                         CComVariant(FUN), VARIANT_FALSE, VARIANT_FALSE);

            _CommandBarButtonPtr _commandBarButtonPtr(commandBarControlPtr);
            pFunButton = _commandBarButtonPtr;
            pFunButton->Enabled = true;
        }

        // Add a new CommandBar to Outlook
        else {
            LOG.debug("CommandBar does not exist -> create it...");
            hr = AddNewCommandBar(spCmdBars);
            if (hr != S_OK) {
                setErrorF(getLastErrorCode(), ERR_ADD_NEW_COMMANDBAR);
                //sprintf(lastErrorMsg, ERR_ADD_NEW_COMMANDBAR);
                goto error;
            }
            createdCommandBar = true;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_GET_COMMANDBAR);
        //sprintf(lastErrorMsg, ERR_GET_COMMANDBAR);
        goto error;
    }


    try {
        //
        // Verify if Funambol menu exists
        //
        spCmdCtrls = spCmdBars->ActiveMenuBar->GetControls();
        ATLASSERT(spCmdCtrls);

        hr = spCmdCtrls->get_Item(CComVariant(FUNAMBOL), &spCmdCtrl);
        // Add new menu bar
        if (FAILED(hr)) {
            LOG.debug("MenuBar does not exist -> create it...");
            hr = AddNewMenubar(spCmdBars);
            if (hr != S_OK) {
                setErrorF(getLastErrorCode(), ERR_ADD_NEW_MENUBAR);
                //sprintf(lastErrorMsg, ERR_ADD_NEW_MENUBAR);
                goto error;
            }
            createdMenuBar = true;
        }
        else {
            LOG.debug("MenuBar already exist -> use it.");
            CommandBarPopupPtr pMenuItem;
            pMenuItem  = spCmdBars->ActiveMenuBar->Controls->GetItem(FUNAMBOL);
            pFunSync   = pMenuItem->Controls->GetItem(BUTTON_SYNCHRONIZE);
            pFunGoto   = pMenuItem->Controls->GetItem(BUTTON_GOTO_PLUGIN);
            pFunConfig = pMenuItem->Controls->GetItem(BUTTON_CONFIGURATION);
        }

        LOG.debug("Linking buttons...");
        hr = DispAdviseControls();
        if (hr != S_OK) {
            LOG.error("Error on setting buttons behaviour -> exit");
            // Something wrong with CommandBarButtons -> reinstall the addin next time.
            saveAddinState(ADDIN_STATE_FAILED);
            errorMsgBox();
            return S_FALSE;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_GET_MENUBAR);
        //sprintf(lastErrorMsg, ERR_GET_MENUBAR);
        goto error;
    }

    //
    // If here, Funambol Addin is installed correctly -> state = 0 ("installed")
    //
    LOG.info("set state = ok");

    setCurrentSwv();                    // Update swv: next time addin won't be reinstalled
    saveAddinState(ADDIN_STATE_OK);

    // Also increment #instances if buttons were created for 1st time.
    if (createdCommandBar && createdMenuBar) {
        LOG.info("Addin installed correctly!");
        updateAddinInstances(true);
    }

    // To catch keyboard events (shortcuts).
    funAddin = this;
    DWORD threadId = GetCurrentThreadId();
    hkb = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)OnKeyboardEvent, NULL, threadId);

    return S_OK;


error:
    // Something wrong -> reinstall the addin next time.
    LOG.error(getLastErrorMsg());
    //LOG.error(lastErrorMsg);

    saveAddinState(ADDIN_STATE_FAILED);
    return S_FALSE;
}



/**
 * Prompt a default error message on desktop.
 */
void Caddin::errorMsgBox() {
    char msg[200];
    sprintf(msg, "Outlook Addin could not be loaded correctly.\nPlease restart Microsoft Outlook to fix the problem.");
    MessageBoxA(NULL, msg, "Funambol Outlook Addin", MB_SETFOREGROUND |MB_OK);
}


/**
 * Opens and initialize the log file for writing. 
 * Get path from Windows TEMP dir -> this is because we need a path different from
 * the plugin install dir, as we need it even when the plugin has been removed.
 * Also, we don't want to leave files/dirs after uninstall procedure.
 * Usually: "C:\Documents and settings\<userName>\Local settings\Temp"
 */
void Caddin::openLog() {

    BOOL resetLog = FALSE;
    size_t requiredSize;
    getenv_s(&requiredSize, NULL, 0, TEMP_ENV);

    char* logPath = new char[requiredSize];
    getenv_s(&requiredSize, logPath, requiredSize, TEMP_ENV);

    if (!logPath) {
        // if not found, use a default one (surely working...)
        logPath = stringdup(LOG_DEFAULT_PATH);
    }

    // Reset log if size too big (>1MB).
    char* logName = new char[strlen(logPath) + strlen(LOG_FILENAME) + 2];
    sprintf(logName, "%s\\%s", logPath, LOG_FILENAME);
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind = FindFirstFileA(logName, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        DWORD logSize = FindFileData.nFileSizeLow;
        FindClose(hFind);
        resetLog = (logSize > MAX_LOG_SIZE)? TRUE : FALSE;
    }

    // Initialize log.
    //Log(resetLog, logPath, LOG_FILENAME);
    LOG.setLogPath(logPath);
    LOG.setLogName(LOG_FILENAME);
    LOG.setLevel(LOG_LEVEL_DEBUG);

    if (logPath) {
        delete [] logPath; logPath = NULL;
    }
    if (logName) {
        delete [] logName; logName = NULL;
    }
}



STDMETHODIMP Caddin::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom) {

    LOG.debug("Closing Outlook...");
    HRESULT hr = 0;

    LOG.debug("Un-linking buttons / unhooking events...");
    DispUnadviseControls();
    UnhookWindowsHookEx(hkb);

    LOG.info("Outlook Closed.");
    return S_OK;
}


STDMETHODIMP Caddin::OnBeginShutdown(SAFEARRAY ** custom) {
    return S_OK;
}


void __stdcall Caddin::OnClickButton(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault) {
    LOG.debug("Click COMMANDBAR ICON");
    launchSyncClientOutlook(PARAM_OUTLOOK_SYNC);
}

void __stdcall Caddin::OnClickSynchronization(IDispatch* Ctrl,VARIANT_BOOL * CancelDefault) {
    LOG.debug("Click SYNC ALL");
    launchSyncClientOutlook(PARAM_OUTLOOK_SYNC);
}

void __stdcall Caddin::OnClickGoto(IDispatch* Ctrl,VARIANT_BOOL * CancelDefault) {
    LOG.debug("Click GOTO");
    launchSyncClientOutlook(NULL);
}

void __stdcall Caddin::OnClickConfiguration(IDispatch* Ctrl,VARIANT_BOOL * CancelDefault) {
    LOG.debug("Click OPTIONS");
    launchSyncClientOutlook(PARAM_OUTLOOK_OPTIONS);
}




/**
 * Launch the Outlook Sync Client executable with the passed parameter.
 * @param parameter   can be "sync" to start automatially the sync,
 *                    or "options" to open up the Config window.
 *                    NULL to simply launch Outlook Sync Client.
 */
void Caddin::launchSyncClientOutlook(const char* parameter) {

    // Note: installDir of Outlook Client is read from HKEY_LOCAL_MACHINE tree:
    //       the key is written only once during install process, for all users.
    char* dir = readPropertyValueFromHKLM(ADDIN_CONTEXT, PROPERTY_PATH);

    if (!dir || !strcmp(dir, "")) {
        // Error: application path not found
        setErrorF(getLastErrorCode(), ERR_INSTALL_DIR_KEY, PROGRAM_NAME_EXE, ADDIN_CONTEXT, PROPERTY_PATH);
        LOG.error(getLastErrorMsg());
        MessageBoxA(NULL, getLastErrorMsg(), "Funambol Outlook Addin", MB_SETFOREGROUND | MB_OK);
        //sprintf(lastErrorMsg, ERR_INSTALL_DIR_KEY, PROGRAM_NAME_EXE, ADDIN_CONTEXT, PROPERTY_PATH);
        //LOG.error(lastErrorMsg);
        //MessageBoxA(NULL, lastErrorMsg, "Funambol Outlook Addin", MB_SETFOREGROUND | MB_OK);
        return;
    }

    // program = "C:\...\OutlookPlugin.exe [param]"
    char* program = NULL;
    if (parameter) {
        program = new char[strlen(dir) + strlen(PROGRAM_NAME_EXE) + strlen(parameter) + 3];
        sprintf(program, "%s\\%s %s", dir, PROGRAM_NAME_EXE, parameter);
    }
    else {
        program = new char[strlen(dir) + strlen(PROGRAM_NAME_EXE) + 2];
        sprintf(program, "%s\\%s", dir, PROGRAM_NAME_EXE);
    }


    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    BOOL res = FALSE;

    //
    // Start the child process.
    //
    SetCurrentDirectoryA(dir);
    res = CreateProcessA(NULL,             // No module name (use command line).
                         program,
                         NULL,             // Process handle not inheritable.
                         NULL,             // Thread handle not inheritable.
                         FALSE,            // Set handle inheritance to FALSE.
                         0,                // No creation flags.
                         NULL,             // Use parent's environment block.
                         NULL,             // Use parent's starting directory.
                         &si,              // Pointer to STARTUPINFO structure.
                         &pi );            // Pointer to PROCESS_INFORMATION structure.
    
    // Save process ID!
    processId = pi.dwProcessId;

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (dir)     delete [] dir;
    if (program) delete [] program;
}




/**
 * Add a new Menu bar with 3 buttons 'Sync All' 'Go to...' and 'Options...'.
 * A menu is a CommandBarPopup object(MsoControlType::msoControlPopup = 10),
 * to which we have to add our menuitems i.e. 
 * CommandBarButton(MsoControlType::msoControlButton = 1).
 * You can also add popup menus similarly.
 *
 * @return : S_OK if no errors
 */
HRESULT Caddin::AddNewMenubar(_CommandBars* pCmdBars) {

    HRESULT hr = S_OK;
    CommandBarPtr          spNewCmdBar;
    CommandBarControlsPtr  spCtrls;
    CommandBarControlsPtr  spCmdBarCtrls;
    CommandBarControlPtr   spCtrl;
    CommandBarControlPtr   spBarCtrlGoto;
    CommandBarControlPtr   spBarCtrlConfig;
    CommandBarControlPtr   spBarCtrl;

    CComVariant vtEmpty    (DISP_E_PARAMNOTFOUND, VT_ERROR);
    CComVariant vtFalse    (VARIANT_FALSE);
    CComVariant popupType  (msoControlPopup);
    CComVariant buttonType (msoControlButton);

    //
    // Add the new menu bar
    //
    try {
        hr = pCmdBars->get_ActiveMenuBar(&spNewCmdBar);
        if (FAILED(hr)) {
            return hr;
        }
        spCtrls = spNewCmdBar->GetControls();
        ATLASSERT(spCtrls);

        // Put Funambol menu before 'Help' (which usually is the last menu)
        int menuCount = spCtrls->GetCount();
        spCtrl = spCtrls->Add(popupType, vtEmpty, vtEmpty, menuCount, vtFalse);
        ATLASSERT(spCtrl);
        CComQIPtr<CommandBarPopup> spPopup(spCtrl->GetControl());
        ATLASSERT(spPopup);

        // Set menu caption.
        LOG.debug("MenuBar -> set Menu caption...");
        spPopup->PutCaption(FUNAMBOL);
        spPopup->PutVisible(VARIANT_TRUE);

        // now add a menu item to the menubar
        // as a CommandBarButton so you can specify styles
        spCmdBarCtrls = spPopup->GetControls();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_ADD_NEW_MENUBAR);        
        // sprintf(lastErrorMsg, ERR_ADD_NEW_MENUBAR);
        goto error;
    }


    //
    // Button "Sync All"
    //
    LOG.debug("MenuBar -> add button '%ls'...", BUTTON_SYNCHRONIZE);
    try {
        spBarCtrl = spCmdBarCtrls->Add(buttonType, vtEmpty, vtEmpty, vtEmpty, vtFalse);
        ATLASSERT(spBarCtrl);
        CComQIPtr <_CommandBarButton> spButton(spBarCtrl);
        ATLASSERT(spButton);

        // set button styles
        spButton->PutCaption(BUTTON_SYNCHRONIZE);
        copyIconIntoClip();
        spButton->PasteFace();
        spButton->PutStyle(msoButtonIconAndCaption);
        spButton->PutVisible(VARIANT_TRUE);

        pFunSync = spButton;
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_ADD_BUTTON1);
        // sprintf(lastErrorMsg, ERR_ADD_BUTTON1);
        goto error;
    }

    //
    // Button "Go to..."
    //
    LOG.debug("MenuBar -> add button '%ls'...", BUTTON_GOTO_PLUGIN);
    try {
        spBarCtrlGoto = spCmdBarCtrls->Add(buttonType, vtEmpty, vtEmpty, vtEmpty, vtFalse);
        ATLASSERT(spBarCtrl);
        CComQIPtr <_CommandBarButton> spButtonGoto(spBarCtrlGoto);
        ATLASSERT(spButtonGoto);

        // set button styles
        spButtonGoto->PutCaption(BUTTON_GOTO_PLUGIN);
        spButtonGoto->PutStyle(msoButtonCaption);
        spButtonGoto->PutVisible(VARIANT_TRUE);

        pFunGoto = spButtonGoto;
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_ADD_BUTTON2);
        // sprintf(lastErrorMsg, ERR_ADD_BUTTON2);
        goto error;
    }

    //
    // Button "Options"
    //
    LOG.debug("MenuBar -> add button '%ls'...", BUTTON_CONFIGURATION);
    try {
        spBarCtrlConfig = spCmdBarCtrls->Add(buttonType, vtEmpty, vtEmpty, vtEmpty, vtFalse);
        ATLASSERT(spBarCtrl);
        CComQIPtr <_CommandBarButton> spButtonConfig(spBarCtrlConfig);
        ATLASSERT(spButtonConfig);

        // set button styles
        spButtonConfig->PutCaption(BUTTON_CONFIGURATION);
        spButtonConfig->PutStyle(msoButtonCaption);
        spButtonConfig->PutVisible(VARIANT_TRUE);

        pFunConfig = spButtonConfig;
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_ADD_BUTTON3);
        //sprintf(lastErrorMsg, ERR_ADD_BUTTON3);
        goto error;
    }

    return S_OK;

error:
    LOG.error(getLastErrorMsg());
    saveAddinState(ADDIN_STATE_FAILED);
    return S_FALSE;
}



/**
 * Add a new Command bar (button with sync icon).
 * Get CommandBarButton interface for each toolbar button
 * so we can specify button styles and stuff. Eeach button displays 
 * a bitmap and caption next to it.
 * To set a bitmap to a button, load a 32x32 bitmap
 * and copy it to clipboard. Call CommandBarButton's PasteFace()
 * to copy the bitmap to the button face. To use
 * Outlook's set of predefined bitmap, set button's FaceId to the
 * button whose bitmap you want to use.
 *
 * @return : S_OK if no errors, S_FALSE if errors
 */
HRESULT Caddin::AddNewCommandBar(_CommandBars* pCmdBars) {

    HRESULT hr = S_OK;
    CommandBarPtr          spNewCmdBar;
    CommandBarControlsPtr  spBarControls;
    CommandBarControlPtr   spNewBar;
    CommandBarControlPtr   spNewBarConfig;

    CComVariant  vName     (PROGRAM_NAME);
    CComVariant  vPos      (1);
    CComVariant  vtFalse   (VARIANT_FALSE);
    CComVariant  vtEmpty   (DISP_E_PARAMNOTFOUND, VT_ERROR);

    try {
        // Add a new CommandBar
        spNewCmdBar = pCmdBars->Add(vName, vPos, vtEmpty, vtFalse);
        spNewCmdBar->Protection = msoBarNoCustomize;

        // Get the CommandBar's controls
        spBarControls = spNewCmdBar->GetControls();
        CComVariant vToolBarType(msoControlButton);

        // show the toolbar?
        CComVariant vShow(VARIANT_FALSE);

        // Add first button
        LOG.debug("CommandBar -> add button...");
        spNewBar = spBarControls->Add(vToolBarType, vtEmpty, vtEmpty, vtEmpty, vShow);
        CComQIPtr <_CommandBarButton> spCmdButton(spNewBar);

        //
        // Load an ICON
        //
        LOG.debug("CommandBar -> load icon...");
        spCmdButton->BeginGroup = true;
        spCmdButton->PutStyle(msoButtonIcon);
        copyIconIntoClip();

        // set style before setting bitmap
        hr = spCmdButton->PasteFace();
        if (hr < 0) {
            spCmdButton->PutFaceId(1758);
        }

        LOG.debug("CommandBar -> set props...");
        spCmdButton->PutVisible    (VARIANT_TRUE);
        spCmdButton->PutCaption    (CAPTION);
        spCmdButton->PutEnabled    (VARIANT_TRUE);
        spCmdButton->PutTooltipText(TOOLTIP);
        spCmdButton->PutTag        (TEXT(FUN));

        pFunButton = spCmdButton;
        spNewCmdBar->PutVisible(VARIANT_TRUE);
        pFunButton->Enabled = true;
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_ADD_NEW_COMMANDBAR);
        //sprintf(lastErrorMsg, ERR_ADD_NEW_COMMANDBAR);
        goto error;
    }

    LOG.debug("CommandBar -> finished!");
    return S_OK;

error:
    LOG.error(getLastErrorMsg());
    saveAddinState(ADDIN_STATE_FAILED);
    return S_FALSE;
}



/**
 * Link CommandBarButton pointers to corresponding events.
 */
HRESULT __stdcall Caddin::DispAdviseControls(void) {

    HRESULT hr = S_OK;
    
    // Command bar icon
    if (pFunButton) {
        try {
            hr = ButtonSyncEvent::DispEventAdvise((IDispatch*)pFunButton, &DIID__CommandBarButtonEvents);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_LINK_ICON);
                //sprintf(lastErrorMsg, ERR_LINK_ICON);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_LINK_ICON);
            //sprintf(lastErrorMsg, ERR_LINK_ICON);
            goto error;
        }
    }

    // Button 'Sync All'
    if (pFunSync) {
        try {
            hr = ItemSynchronizationEvent::DispEventAdvise((IDispatch*)pFunSync, &DIID__CommandBarButtonEvents);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_LINK_BUTTON1);
                //sprintf(lastErrorMsg, ERR_LINK_BUTTON1);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_LINK_BUTTON1);
            //sprintf(lastErrorMsg, ERR_LINK_BUTTON1);
            goto error;
        }
    }

    // Button 'Go to...'
    if (pFunGoto) {
        try {
            hr = ItemGotoEvent::DispEventAdvise((IDispatch*)pFunGoto, &DIID__CommandBarButtonEvents);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_LINK_BUTTON2);
                //sprintf(lastErrorMsg, ERR_LINK_BUTTON2);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_LINK_BUTTON2);
            //sprintf(lastErrorMsg, ERR_LINK_BUTTON2);
            goto error;
        }
    }

    // Button 'Options...'
    if (pFunConfig) {
        try {
            hr = ItemConfigurationEvent::DispEventAdvise((IDispatch*)pFunConfig, &DIID__CommandBarButtonEvents);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_LINK_BUTTON3);
                //sprintf(lastErrorMsg, ERR_LINK_BUTTON3);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_LINK_BUTTON3);
            //sprintf(lastErrorMsg, ERR_LINK_BUTTON3);
            goto error;
        }
    }

    return hr;

error:
    LOG.error(getLastErrorMsg());
    saveAddinState(ADDIN_STATE_FAILED);
    return S_FALSE;
}





/**
 * Un-link CommandBarButton pointers to corresponding events.
 */
HRESULT __stdcall Caddin::DispUnadviseControls(void) {

    HRESULT hr = S_OK;

    // Command bar icon
    if (pFunButton) {
        try {
            hr = ButtonSyncEvent::DispEventUnadvise((IDispatch*)pFunButton);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_UNLINK_ICON);
                //sprintf(lastErrorMsg, ERR_UNLINK_ICON);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_UNLINK_ICON);
            //sprintf(lastErrorMsg, ERR_UNLINK_ICON);
            goto error;
        }
    }

     // Button 'Sync All'
    if (pFunSync) {
        try {
            hr = ItemSynchronizationEvent::DispEventUnadvise((IDispatch*)pFunSync);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON1);
                //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON1);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON1);
            //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON1);
            goto error;
        }
    }

    // Button 'Go to...'
    if (pFunGoto) {
        try {
            hr = ItemGotoEvent::DispEventUnadvise((IDispatch*)pFunGoto);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON2);
                //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON2);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON2);
            //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON2);
            goto error;
        }
    }

    // Button 'Options...'
    if (pFunConfig) {
        try {
            hr = ItemConfigurationEvent::DispEventUnadvise((IDispatch*)pFunConfig);
            if (FAILED(hr)) {
                setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON3);
                //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON3);
                goto error;
            }
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_UNLINK_BUTTON3);
            //sprintf(lastErrorMsg, ERR_UNLINK_BUTTON3);
            goto error;
        }
    }

    return hr;

error:
    LOG.error(getLastErrorMsg());
    saveAddinState(ADDIN_STATE_FAILED);
    return S_FALSE;
}




/**
 * Removes the Funambol Outlook Addin from Outlook.
 * 1. Removes Funambol CommandBar (if exists)
 * 2. Removes Funambol menu named "Funambol" (if exists)
 */
HRESULT Caddin::removeAddin() {

    _ExplorerPtr           spExplorer;
    _CommandBarsPtr        spCmdBars;
    CommandBarControlPtr   commandBarControlPtr;
    CommandBarPtr          spCmdBar;
    CommandBarPopupPtr     pMenuItem;
    CommandBarControlsPtr  spCmdCtrls;
    CommandBarControlPtr   spCmdCtrl;

    VARIANT variant;
    variant.vt = VT_BSTR;
    HRESULT hr = S_OK;
    bool deletedCommandBar = false;
    bool deletedMenuBar    = false;

    LOG.info("Removing existing Outlook Addin...");

    try {
        // Get Outlook Command Bars
        applicationPtr->ActiveExplorer(&spExplorer);
        hr = spExplorer->get_CommandBars(&spCmdBars);
        if(FAILED(hr)) {
            LOG.error(ERR_GET_COMMANDBARS);
            return hr;
        }

        //
        // Get and remove Funambol Command Bar
        //
        LOG.debug("Removing Funambol CommandBar...");
        variant.bstrVal = SysAllocString(CAPTION);
        hr = spCmdBars->get_Item(variant, &spCmdBar);
        //VariantClear(&variant);
        if (SUCCEEDED(hr)) {
            hr = spCmdBar->Delete();
            if (SUCCEEDED(hr)) {
                LOG.debug("deleted.");
                deletedCommandBar = true;
            }
        }
        else {
            LOG.debug("not found.");
        }

        // get CommandBar that is Outlook's main menu
        hr = spCmdBars->get_ActiveMenuBar(&spCmdBar);
        if (FAILED(hr)) {
            LOG.error("Active Menu not found.");
            return hr;
        }
        spCmdCtrls = spCmdBar->GetControls();
        ATLASSERT(spCmdCtrls);

        //
        // Get and remove Funambol menu
        //
        LOG.debug("removing Funambol Menu...");
        hr = spCmdCtrls->get_Item(CComVariant(FUNAMBOL), &spCmdCtrl);
        if (SUCCEEDED(hr)) {
            pMenuItem = spCmdBars->ActiveMenuBar->Controls->GetItem(FUNAMBOL);
            hr = pMenuItem->Delete(vtMissing);
            if (SUCCEEDED(hr)) {
                LOG.debug("deleted.");
                deletedMenuBar = true;
            }
        }
        else {
            LOG.debug("not found.");
        }


        //
        // Try to remove old Funambol Command Bar (before v.7.1.4 its name was "Funambol Outlook Plug-in")
        // Better would be to check the oldSwv, and see if < 7.1.4. But to be sure, we can do it anyway.
        //
        LOG.debug("Removing any old Funambol CommandBar...");
        variant.bstrVal = SysAllocString(L"Funambol Outlook Plug-in");
        hr = spCmdBars->get_Item(variant, &spCmdBar);
        if (SUCCEEDED(hr)) {
            hr = spCmdBar->Delete();
            if (SUCCEEDED(hr)) {
                LOG.debug("deleted.");
                deletedCommandBar = true;
            }
        }
        else {
            LOG.debug("not found.");
        }

        //
        // Also get and remove Sync4j bar/menu that could be left :)
        //
        LOG.debug("removing any old Sync4j Bar/Menu...");
        variant.bstrVal = SysAllocString(CAPTION_S4J);
        hr = spCmdBars->get_Item(variant, &spCmdBar);
        VariantClear(&variant);
        if (SUCCEEDED(hr)) {
            hr = spCmdBar->Delete();
            LOG.debug("bar deleted.");
        }
        else {
            LOG.debug("bar not found.");
        }

        hr = spCmdCtrls->get_Item(CComVariant(AMP_SYNC4J), &spCmdCtrl);
        if (SUCCEEDED(hr)) {
            pMenuItem = spCmdBars->ActiveMenuBar->Controls->GetItem(AMP_SYNC4J);
            pMenuItem->Delete(vtMissing);
            LOG.debug("menu deleted.");
        }
        else {
            LOG.debug("menu not found.");
        }
    }

    catch(_com_error &e) {
        manageComErrors(e);
        LOG.error(ERR_REMOVING_ADDIN);
        return S_FALSE;
    }


    // Also decrement #instances of Addin.
    if (deletedCommandBar && deletedMenuBar) {
        LOG.info("Outlook Addin removed.");
        updateAddinInstances(false);
    }

    return S_OK;
}


/**
 * Returns true if the addin is currently installed.
 * Checks the commandBar 'PROGRAM_NAME' to know if plugin is installed.
 */
bool Caddin::isAddinInstalled() {

    _ExplorerPtr    spExplorer;
    _CommandBarsPtr spCmdBars;
    CommandBarPtr   spCmdBar;
    HRESULT hr = S_OK;

    try {
        // Get Outlook Command Bars
        applicationPtr->ActiveExplorer(&spExplorer);
        hr = spExplorer->get_CommandBars(&spCmdBars);
        if(FAILED(hr)) {
            LOG.error(ERR_GET_COMMANDBARS);
            return false;
        }

        // Get CommandBars
        hr = spExplorer->get_CommandBars(&spCmdBars);
        if (FAILED(hr)) {
            return false;
        }

        // Get Funambol CommandBar
        hr = spCmdBars->get_Item(CComVariant(PROGRAM_NAME), &spCmdBar);
        if (SUCCEEDED(hr)) {
            // found!
            return true;
        }
    }
    catch(_com_error &) {
        return false;
    }

    return false;
}



/**
 * Returns true if an error occurred during last execution of addin.
 * We save a 'State' value under HKCU key, to ensure everything was good.
 */
bool Caddin::checkErrorsLastTime() {

    bool ret = true;
    char* state = NULL;
    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    // Get state from reg key
    dmt = DMTreeFactory::getDMTree(ADDIN_CONTEXT);
    if (!dmt)   goto finally;
    node = dmt->readManagementNode(ADDIN_CONTEXT);
    if (!node)  goto finally;
    state = node->readPropertyValue(PROPERTY_STATE);
    if (!state) goto finally;


    // No errors only if state = "ok" or "installing"
    if (!strcmp(state, ADDIN_STATE_OK) || !strcmp(state, ADDIN_STATE_INSTALLING)) {
        ret = false;
    }
    else {
        ret = true;
    }

finally:
    if (state) delete [] state;
    if (node)  delete node;

    return ret;
}



/**
 * Saves addin state in win registry (HKCU, key = STATE_CONTEXT)
 */
int Caddin::saveAddinState(char* state) {

    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    if (!state) {
        return 1;
    }

    // Save value
    LOG.debug("Saving addin state = %s", state);
    dmt = DMTreeFactory::getDMTree(ADDIN_CONTEXT);
    if (!dmt) return 1;
    node = dmt->readManagementNode(ADDIN_CONTEXT);
    if (!node) return 1;
    node->setPropertyValue(PROPERTY_STATE, state); 
    delete node;

    return 0;
}





/**
 * Actions to execute when a COM pointer exception occurs.
 */
void Caddin::manageComErrors(_com_error &e) {

    //sprintf(lastErrorMsg, ERR_COM_POINTER, e.Error(), e.ErrorMessage());
    //lastErrorCode = (int)e.Error(); 
    setErrorF((int)e.Error(), ERR_COM_POINTER, e.Error(), e.ErrorMessage());
    LOG.error(getLastErrorMsg());
}




/**
 * Reads Addin property 'swv' from HKCU keys.
 * Returns a (new allocated) buffer with value read.
 */
char* Caddin::readAddinSwv() {

    int value = -1;
    char* swv = NULL;
    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    // Get state from reg key
    dmt = DMTreeFactory::getDMTree(ADDIN_CONTEXT);
    if (!dmt)   goto finally;
    node = dmt->readManagementNode(ADDIN_CONTEXT);
    if (!node)  goto finally;
    swv = node->readPropertyValue(PROPERTY_SW_VERSION);
    delete node;

finally:
    return swv;
}


/**
 * Saves current software version to HKCU addin keys.
 * Used to check sw upgrades at each start-up.
 */
int Caddin::setCurrentSwv() {

    int ret = 0;
    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    // Read current swv from HKLM plugin keys.
    char* swv = readPropertyValueFromHKLM(PLUGIN_CONTEXT, PROPERTY_SW_VERSION);
    if (swv && strcmp(swv, "")) {

        // Save value to HKCU addin keys.
        dmt = DMTreeFactory::getDMTree(ADDIN_CONTEXT);
        if (!dmt) {
            ret = -1;
            goto finally;
        }
        node = dmt->readManagementNode(ADDIN_CONTEXT);
        if (!node) {
            ret = -1;
            goto finally;
        }
        node->setPropertyValue(PROPERTY_SW_VERSION, swv);
        delete node;
        ret = 0; 
    }
    else {
        ret = 1;
    }

finally:
    if (swv) {
        delete[] swv;
    }
    return ret;
}



/**
 * Checks if the current version of addin is compatible with the old version installed.
 * If not compatible, the addin must be reinstalled to avoid errors.
 * @return : 'true'  if versions are not compatible
 *           'false' if versions are compatible
 */
bool Caddin::swvNotCompatible(const char* currentVersion, const char* oldVersion) {

    if ( !oldVersion     || !strcmp(oldVersion, "") ||
         !currentVersion || !strcmp(currentVersion, "") ) {
        // Old version not found or something wrong -> not compatible (reinstall)
        return true;
    }

    if (!strcmp(currentVersion, oldVersion)) {
        // Same version -> compatible (no need to reinstall)
        return false;
    }

    int major=0,    minor=0,    build=0;
    int oldmajor=0, oldminor=0, oldbuild=0;

    sscanf(currentVersion, "%d.%d.%d", &major,    &minor,    &build);
    sscanf(oldVersion,     "%d.%d.%d", &oldmajor, &oldminor, &oldbuild);

    int currentBuildNumber = (major*10000)    + (minor*100)    + build;
    int oldBuildNumber     = (oldmajor*10000) + (oldminor*100) + oldbuild;

    if (currentBuildNumber < oldBuildNumber) {
        // Old version is more recent -> not compatible (reinstall)
        return true;
    }

    if (oldBuildNumber < LAST_COMPATIBLE_VERSION) {
        // Old version is not compatible with this one (reinstall)
        return true;
    }

    // Old version is compatible (no need to reinstall)
    return false;
}



/**
 * Returns the value of the given property, from HKEY_LOCAL_MACHINE tree (read only).
 * The value is returned as a new char array and must be freed by the user
 *
 * @param context - the context path under HKLM
 * @param prop    - the property name
 */
char* Caddin::readPropertyValueFromHKLM(const char* context, const char* prop) {
    
    DWORD res = 0;  	
    long  err = 0;
    ULONG dim = 0;
    HKEY  key = NULL;
    char* ret = NULL;

    char fullContext[DIM_MANAGEMENT_PATH] = "Software/";
    strcat(fullContext, context);
    toWindows(fullContext);

    RegCreateKeyExA(
            HKEY_LOCAL_MACHINE,
            fullContext,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,                           // Read only: could be from a limited rights user.
            NULL,
            &key,
            &res
            );

    if (key == 0) {
        //lastErrorCode = ERR_INVALID_CONTEXT;
        //sprintf(lastErrorMsg, "Invalid context path: HKEY_LOCAL_MACHINE\\%s", fullContext);
        setErrorF(ERR_INVALID_CONTEXT, "Invalid context path: HKEY_LOCAL_MACHINE\\%s", fullContext);
        goto finally;
    }

    // Get value length
    err = RegQueryValueExA(
            key,
            prop,
            NULL,
            NULL,  // we currently support only strings
            NULL,
            &dim
            );

    if (err == ERROR_SUCCESS) {
		if (dim > 0) {
            char* buf = new char[dim + 1];

			err = RegQueryValueExA(
					key,
					prop,
					NULL,
					NULL,  // we currently support only strings
					(UCHAR*)buf,
					&dim 
                    );
            if (err == ERROR_SUCCESS) {
                ret = stringdup(buf);
            }
            delete [] buf;
		}
    }

    if (!ret) {
        // return an empty string if key not found...
        ret = stringdup("");
    }

finally:

    if (key != 0) {
        RegCloseKey(key);
    }
    return ret;
}



/**
 * Convert the path in Windows format, changing the slashes in back-slashes.
 * @param str - the string to convert
 */
static void toWindows(char* str) {

    int i=0;
    while (str[i]) {
        if (str[i] == '/') {
            str[i] = '\\';
        }
        i++;
    }
}


/**
 * *********** TODO **************
 * Returns true if this is the last instace installed of Outlook addin.
 *
 * Number of addin installed for all users is stored under
 * "HKU\.Default" root key, is incremented each time a new
 * addin creates the buttons/bars.
 * Addin dll MUST NOT be unregistered until every user has removed
 * buttons/bars from Outlook UI...
 */
bool Caddin::isLastInstance() {

//    HKEY key;
//    DWORD res = 0;
//    long  err = 0;
//    ULONG dim = 100;
//
//    DWORD numInstances;
//    const char* propertyName = PROPERTY_NUM_INSTANCES;
//
//    //
//    // Get access to key: "HKEY_USERS\.Default\Software\Microsoft\Office\Outlook\Addins\FunambolAddin.Addin"
//    //
//    char fullContext[DIM_MANAGEMENT_PATH] = ".Default/Software/";
//    strcat(fullContext, ADDIN_CONTEXT);
//    toWindows(fullContext);
//
//    RegCreateKeyExA(
//        HKEY_USERS,
//        fullContext,
//        0,
//        NULL,
//        REG_OPTION_NON_VOLATILE,
//        KEY_ALL_ACCESS,
//        NULL,
//        &key,
//        &res
//        );
//
//    if (key == 0) {
//        lastErrorCode = ERR_INVALID_CONTEXT;
//        sprintf(lastErrorMsg, "Invalid context path: HKEY_USERS\\%s", fullContext);
//        goto finally;
//    }
//
//    //
//    // Get value
//    //
//    err = RegQueryValueExA(
//            key,
//            propertyName,
//            NULL,
//            NULL,
//            (LPBYTE)&numInstances,
//            &dim
//            );
//
//
//    LOG.debug("#instances = %d", numInstances);
//
//finally:
//    if (key) {
//        RegCloseKey(key);
//    }
//
//    if (!numInstances) {
//        return true;
//    }
//    else {
//        return false;
//    }

    return false;
}


/**
 ******************* TODO ********************
 * Method called when Addin buttons are created for the first time for
 * this user / when Addin buttons are deleted. The global 
 * number of instances will be incremented / decremented by 1.
 * Addin dll MUST NOT be unregistered until every user has removed
 * buttons/bars from Outlook UI...
 */
void Caddin::updateAddinInstances(bool increment) {

//    HKEY key;
//    DWORD res = 0;
//    long  err = 0;
//    ULONG dim = 100;
//
//    DWORD numInstances;
//    const char* propertyName = PROPERTY_NUM_INSTANCES;
//    LPBYTE lpData = NULL;
//
//    //
//    // Get access to key: "HKEY_USERS\.Default\Software\Microsoft\Office\Outlook\Addins\FunambolAddin.Addin"
//    //
//    char fullContext[DIM_MANAGEMENT_PATH] = ".Default/Software/";
//    strcat(fullContext, ADDIN_CONTEXT);
//    toWindows(fullContext);
//
//    RegCreateKeyExA(
//        HKEY_USERS,
//        fullContext,
//        0,
//        NULL,
//        REG_OPTION_NON_VOLATILE,
//        KEY_ALL_ACCESS,
//        NULL,
//        &key,
//        &res
//        );
//
//    if (key == 0) {
//        lastErrorCode = ERR_INVALID_CONTEXT;
//        sprintf(lastErrorMsg, "Invalid context path: HKEY_USERS\\%s", fullContext);
//        goto finally;
//    }
//
//    //
//    // Get value
//    //
//    err = RegQueryValueExA(
//            key,
//            propertyName,
//            NULL,
//            NULL,
//            (LPBYTE)&numInstances,
//            &dim
//            );
//
//
//    // ----------------------------
//    if (increment) {
//        LOG.debug("#instances = %d -> %d", numInstances, numInstances+1);
//        numInstances ++;
//    }
//    else {
//        LOG.debug("#instances = %d -> %d", numInstances, numInstances-1);
//        numInstances --;
//    }
//    // ----------------------------
//
//
//    //
//    // Set new value
//    // 
//    RegSetValueExA(
//        key,
//        propertyName,
//        NULL,
//        REG_DWORD,
//        (LPBYTE)&numInstances,
//        sizeof(DWORD)
//        );
//
//finally:
//    if (key) {
//        RegCloseKey(key);
//    }

}
