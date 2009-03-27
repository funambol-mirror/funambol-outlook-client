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

// put #include "Yourdocfile" before #include "Yourviewfile" 

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "OutlookPluginDoc.h"
#include "OutlookPluginMainDoc.h"

#include "ConfigFrm.h"
#include "LeftView.h"
#include "SyncForm.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"
#include "CustomLabel.h"

#include "winmaincpp.h"
#include "utils.h"  
#include "comutil.h"
#include "afxwin.h"

// set DEBUG_LOAD_RESOURCE to 1 to explicit load a resource dll
#define LOAD_RESOURCE 1
#define RESOURCE_LIBRARY_FILENAME TEXT("language.dll")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//
// The Outlook Sync Client licence text, to be displayed in About dialog.
//
static WCHAR licence[] = TEXT("This program is provided AS IS, without warranty licensed under AGPLV3. The\n") \
                         TEXT("Program is free software; you can redistribute it and/or modify it under the\n") \
                         TEXT("terms of the GNU Affero General Public License version 3 as published by the\n") \
                         TEXT("Free Software Foundation including the additional permission set forth source\n") \
                         TEXT("code file header.\n") \
                         TEXT("\n") \
                         TEXT("The interactive user interfaces in modified source and object code versions of\n") \
                         TEXT("this program must display Appropriate Legal Notices, as required under Section 5\n") \
                         TEXT("of the GNU Affero General Public License version 3.\n") \
                         TEXT("\n") \
                         TEXT("In accordance with Section 7(b) of the GNU Affero General Public License\n") \
                         TEXT("version 3, these Appropriate Legal Notices must retain the display of the\n") \
                         TEXT("\"Powered by Funambol\" logo. If the display of the logo is not reasonably\n") \
                         TEXT("feasible for technical reasons, the Appropriate Legal Notices must display the\n") \
                         TEXT("words \"Powered by Funambol\". Funambol is a trademark of Funambol, Inc.\n");


BEGIN_MESSAGE_MAP(COutlookPluginApp, CWinApp)
	//{{AFX_MSG_MAP(COutlookPluginApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_VIEW_GUIDE, OnViewGuide)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()


COutlookPluginApp::COutlookPluginApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


COutlookPluginApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
protected:
    CFont fontSmall;
    CFont fontBold;
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    //}}AFX_VIRTUAL

    // Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    // No message handlers
    //}}AFX_MSG
    CBrush brush;
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL OnSetCursor(CWnd* pWnd,  UINT nHitTest,  UINT message );
public:
    CCustomLabel linkSite;
public:
    afx_msg void OnStnClickedAboutLink();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_ABOUT_LINK, linkSite);
}

BOOL CAboutDlg::OnInitDialog(){

    CString s1("");
    CString s2("");
    s1.LoadString(IDS_ABOUT_TITLE); SetWindowText(s1); s1 = "";
    CDialog::OnInitDialog();

    // Set fonts
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT)); 
    GetFont()->GetLogFont(&lf); 
    lf.lfWeight = FW_BOLD;
    VERIFY(fontBold.CreateFontIndirect(&lf));

    lf.lfWeight = FW_NORMAL;
    lf.lfHeight = -9;
    VERIFY(fontSmall.CreateFontIndirect(&lf));


    // Program name + version
    s1 += WPROGRAM_NAME;
    s1 += TEXT("\nVersion ");
    s1 += getConfig()->readCurrentSwv();
    SetDlgItemText(IDC_ABOUT_MAIN, s1);
    GetDlgItem(IDC_ABOUT_MAIN)->SetFont(&fontBold);

    // Copyright
    s1.LoadString(IDS_ABOUT_COPYRIGHT); 
    SetDlgItemText(IDC_ABOUT_COPYRIGHT, s1);

    // Link site
    linkSite.init();
    s1.LoadString(IDS_FUNAMBOL_LINK); 
    SetDlgItemText(IDC_ABOUT_LINK, s1);

    // Licence text
    s1 = licence;
    SetDlgItemText(IDC_ABOUT_LICENCE, s1);
    GetDlgItem(IDC_ABOUT_LICENCE)->SetFont(&fontSmall);


    brush.CreateSolidBrush(RGB(255,255,255));

    return TRUE;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    ON_WM_CTLCOLOR()
    ON_WM_SETCURSOR()
    //}}AFX_MSG_MAP
    ON_STN_CLICKED(IDC_ABOUT_LINK, &CAboutDlg::OnStnClickedAboutLink)
END_MESSAGE_MAP()




HBRUSH CAboutDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor){
    pDC->SetBkColor(RGB(255,255,255));

    if(pWnd->GetRuntimeClass() == RUNTIME_CLASS(CCustomLabel) ){
        pDC->SetTextColor( ((CCustomLabel*)pWnd)->clrLinkText );
    };
    return (HBRUSH)(brush.GetSafeHandle());;
}


BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message ){
    if(pWnd->GetRuntimeClass() == RUNTIME_CLASS(CCustomLabel)){
        ::SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return TRUE;
    }
    else
        CDialog::OnSetCursor(pWnd, nHitTest, message);

    return TRUE;
}


// App command to run the dialog
void COutlookPluginApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

// App command to run the dialog
void COutlookPluginApp::OnViewGuide() {

    SHELLEXECUTEINFO lpExecInfo;
    memset(&lpExecInfo, 0, sizeof(SHELLEXECUTEINFO));
    lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    CString guide;
    guide.LoadString(IDS_GUIDE_LINK); 
    lpExecInfo.lpFile = guide;
    lpExecInfo.nShow = SW_SHOWNORMAL;
    lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    lpExecInfo.lpVerb = _T("open");
    ShellExecuteEx(&lpExecInfo);
    ZeroMemory(&lpExecInfo, sizeof(SHELLEXECUTEINFO));
}

/////////////////////////////////////////////////////////////////////////////
// COutlookPluginApp message handlers

BOOL COutlookPluginApp::InitInstance()
{
    
	AfxEnableControlContainer();

    #if LOAD_RESOURCE

    //if( PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale())) != LANG_ENGLISH ){            
        hInst = LoadLibrary(RESOURCE_LIBRARY_FILENAME);         
        if (hInst != NULL)
            AfxSetResourceHandle(hInst);  
    //}

    #endif

    hLib = LoadLibrary(_T("uxtheme.dll"));

    //TODO: set the time to use the current user locale
    /* 
    Some language characters are not displayed properly and depending of user format time 
    the AM/PM could not be displayed, needs investigation
    */

    //setlocale(LC_TIME, "");


	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    BOOL isScheduled  = FALSE;
    BOOL startSyncNow = FALSE;
    BOOL openConfig   = FALSE;

    CString cmdLine = GetCommandLine();
    if (cmdLine.Find(L" schedule") > 0)
        isScheduled = TRUE;
    else if (cmdLine.Find(L" sync") > 0)
        startSyncNow = TRUE;
    else if (cmdLine.Find(L" options") > 0)
        openConfig = TRUE;

    if(! isScheduled){
        // is NOT schedule, check if the UI is already opened
        if(HwndFunctions::findFunambolWindow()){
            // UI is opened, put it in foreground and close this instance
            HWND wnd = HwndFunctions::getWindowHandle();
            ShowWindow(wnd, SW_RESTORE);
            SetForegroundWindow(wnd);
            return FALSE;
        }
        else{
            // UI not opened
            if(! checkSyncInProgress()) {
                initializeClient(false);
            }
            else{
                // A sync is already in progress (another scheduled).
                CString msg;
                msg.LoadString(IDS_TEXT_SYNC_IN_PROGRESS);
                int flags = flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
                MessageBox(NULL, msg, WPROGRAM_NAME, flags);
            }
        }

        // Register a unique class name for UI.
        if (registerFunClass() == FALSE) {
            //LOG.error(ERR_CLASS_REG_FAILED, PLUGIN_UI_CLASSNAME);
            return FALSE;
        }

        // Show UI
        docSettings = new CSingleDocTemplate(
            IDR_MAINFRAME,
            RUNTIME_CLASS(COutlookPluginDoc),
            RUNTIME_CLASS(CConfigFrame),       
            RUNTIME_CLASS(CLeftView));

        docMain = new CSingleDocTemplate(
            IDR_MAINFRAME,
            RUNTIME_CLASS(COutlookPluginMainDoc),
            RUNTIME_CLASS(CMainSyncFrame),       // main SDI frame window
            RUNTIME_CLASS(CSyncForm));

        AddDocTemplate(docMain); 

        // Parse command line for standard shell commands, DDE, file open
        CCommandLineInfo cmdInfo;
        //ParseCommandLine(cmdInfo); // not used

        // Dispatch commands specified on the command line
        if (!ProcessShellCommand(cmdInfo)) {
        //	return FALSE;
        }

        m_pMainWnd->ShowWindow(SW_SHOW);
        m_pMainWnd->UpdateWindow();

        // Start immediately the sync.
        if (startSyncNow) {
            CMainSyncFrame* pFrame = (CMainSyncFrame*)AfxGetMainWnd();
            pFrame->Invalidate();
            pFrame->StartSync();
        }
        
        // Open automatically the options dialog.
        else if (openConfig) {
            CMainSyncFrame* pFrame = (CMainSyncFrame*)AfxGetMainWnd();
            pFrame->showSettingsWindow();
        }
    }
    else{
        // scheduled sync
        if(checkSyncInProgress()){
            return FALSE;  // close this instance
        }
        else{
            // sync is NOT in progress
            initializeClient(true);

            //
            // Start the sync thread.
            //
            try {
                hScheduleSyncThread = ::CreateThread(NULL, 0, syncThread, 0, 0, &dwScheduleThreadId);
                if (hScheduleSyncThread == NULL) {
                    DWORD errorCode = GetLastError();
                    printLog("Thread error: syncThread", LOG_ERROR);
                    return FALSE;
                }
            }
            catch(...){
            }

            // Wait until the sync thread has finished...
            WaitForSingleObject(hScheduleSyncThread, INFINITE);

            closeClient();
            return FALSE; // after the sync, close the instance
        }
    }
    
   return TRUE;
}


int COutlookPluginApp::ExitInstance(){
    FreeLibrary(hInst);
    FreeLibrary(hLib);
    return CWinApp::ExitInstance();
}
/*
long COutlookPluginApp::OnIdle(int count)
{
    ASSERT(_heapchk() == _HEAPOK);
    return CWinApp::OnIdle(count);
}
*/



/**
 * Register the unique class name: PLUGIN_UI_CLASSNAME
 * A unique classname is important to use FindWindow() function.
 */
BOOL COutlookPluginApp::registerFunClass() {

    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults

    wndcls.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc   = ::DefWindowProc; 
    wndcls.hInstance     = AfxGetInstanceHandle();
    wndcls.hIcon         = LoadIcon(IDR_MAINFRAME);         // ICON is set here.
    wndcls.lpszClassName = PLUGIN_UI_CLASSNAME;             // Class name.

    wndcls.hbrBackground = (HBRUSH) (BLACK_BRUSH);

    // Register the new class and exit if it fails
    if(!AfxRegisterClass(&wndcls)) {
        return FALSE;
    }
    return TRUE;
}



void CAboutDlg::OnStnClickedAboutLink()
{
    SHELLEXECUTEINFO lpExecInfo;
    memset(&lpExecInfo, 0, sizeof(SHELLEXECUTEINFO));
    lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    CString site("http://"), s1; s1.LoadString(IDS_FUNAMBOL_LINK);
    site += s1;
    lpExecInfo.lpFile = site;
    lpExecInfo.nShow = SW_SHOWNORMAL;
    lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    lpExecInfo.lpVerb = _T("open");
    ShellExecuteEx(&lpExecInfo);
    ZeroMemory(&lpExecInfo, sizeof(SHELLEXECUTEINFO));
}
