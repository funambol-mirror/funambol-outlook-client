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
#include "resource.h"
#include "TaskSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"
#include "SettingsHelper.h"

#include "winmaincpp.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"
#include "UICustomization.h"

#include <string>

using namespace std;

// CTaskSettings

static wstring outlookFolder;
static CTaskSettings* wndTasks;
static HANDLE handleThread;

IMPLEMENT_DYNCREATE(CTaskSettings, CDialog)

CTaskSettings::CTaskSettings()
	: CDialog(CTaskSettings::IDD)
{
    handleThread = NULL;
}

CTaskSettings::~CTaskSettings()
{
    // clean stuff used in the select Outlook folder thread
    wndTasks = NULL;
    if (handleThread) {
        TerminateThread(handleThread, -1);
        CloseHandle(handleThread);
        handleThread = NULL;
    }
}

void CTaskSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TASKS_COMBO_SYNCTYPE,  lstSyncType);
    DDX_Control(pDX, IDC_TASKS_EDIT_FOLDER,     editFolder);
    DDX_Control(pDX, IDC_TASKS_CHECK_INCLUDE,   checkInclude);
    DDX_Control(pDX, IDC_TASKS_BUT_SELECT,      butSelectFolder);
    DDX_Control(pDX, IDC_TASKS_EDIT_REMOTE,     editRemote);
    DDX_Control(pDX, IDC_TASKS_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_TASKS_GROUP_FOLDER,    groupFolder);
    DDX_Control(pDX, IDC_TASKS_ADVANCED,        groupAdvanced);
    DDX_Control(pDX, IDC_TASKS_CHECK_SHARED,    checkShared);
}

BEGIN_MESSAGE_MAP(CTaskSettings, CDialog)
    ON_BN_CLICKED(IDC_TASKS_OK,           &CTaskSettings::OnBnClickedTasksOk)
    ON_BN_CLICKED(IDC_TASKS_CANCEL,       &CTaskSettings::OnBnClickedTasksCancel)
    ON_BN_CLICKED(IDC_TASKS_BUT_SELECT,   &CTaskSettings::OnBnClickedTasksButSelect)   
    ON_BN_CLICKED(IDC_TASKS_CHECK_SHARED, &CTaskSettings::OnBnClickedTasksCheckShared)
END_MESSAGE_MAP()


// CTaskSettings diagnostics

#ifdef _DEBUG
void CTaskSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CTaskSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


// CTaskSettings message handlers
BOOL CTaskSettings::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_TASKS_DETAILS); SetWindowText(s1);
    CDialog::OnInitDialog();
    

    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(TASK_);

    editFolder.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editRemote.SetLimitText(EDIT_TEXT_MAXLENGTH);

    // load string resources
    s1.LoadString(IDS_SYNCTYPE1); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE2); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE3); lstSyncType.AddString(s1);

    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_TASKS_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_TASKS_FOLDER);        SetDlgItemText(IDC_TASKS_GROUP_FOLDER, s1);
    s1.LoadString(IDS_CURRENT);             SetDlgItemText(IDC_TASKS_STATIC_FOLDER, s1);
    s1.LoadString(IDS_INCLUDE_SUBFOLDERS);  SetDlgItemText(IDC_TASKS_CHECK_INCLUDE, s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_TASKS_BUT_SELECT, s1);
    s1.LoadString(IDS_REMOTE_NAME);         SetDlgItemText(IDC_TASKS_STATIC_REMOTE, s1);
    s1.LoadString(IDS_ADVANCED);            SetDlgItemText(IDC_TASKS_ADVANCED, s1);
    s1.LoadString(IDS_DATA_FORMAT);         SetDlgItemText(IDC_TASKS_STATIC_DATAFORMAT, s1);
    s1.LoadString(IDS_USE_VCAL);            SetDlgItemText(IDC_TASKS_DATA_FORMAT, s1);
    s1.LoadString(IDS_SHARED);              SetDlgItemText(IDC_TASKS_CHECK_SHARED, s1);
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_TASKS_OK, s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_TASKS_CANCEL, s1);

    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
	
    // Get folder path.
    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* olFolder = toWideChar(ssconf->getFolderPath());
    s1 = olFolder;
    delete [] olFolder;
    try {
        if(s1 == ""){
            s1 = getDefaultFolderPath(TASK).data();
        }
    }
    catch (...){
        // an exception occured while trying to get the default folder
        EndDialog(-1);
    }
    SetDlgItemText(IDC_TASKS_EDIT_FOLDER, s1);

    if(ssconf->getUseSubfolders()) {
        checkInclude.SetCheck(BST_CHECKED);
    }

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* remName = toWideChar(ssconf->getURI());
    s1 = remName;
    delete [] remName;
    SetDlgItemText(IDC_TASKS_EDIT_REMOTE, s1);

    if (s1.Right(wcslen(SHARED_SUFFIX)).Compare(SHARED_SUFFIX) == 0) {
        checkShared.SetCheck(BST_CHECKED);
    }

    wndTasks = this;


    // Apply customizations
    bool shared             = UICustomization::shared;
    bool forceUseSubfolders = UICustomization::forceUseSubfolders;
    bool hideDataFormats    = UICustomization::hideDataFormats;
    bool hideAllAdvanced    = !SHOW_ADVANCED_SETTINGS;

    if (forceUseSubfolders) {
        checkInclude.SetCheck(BST_CHECKED);
        checkInclude.ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        checkInclude.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(GetDlgItem(IDC_TASKS_GROUP_FOLDER), 0, dy);

        moveItem(this, &groupAdvanced, 0, dy);
        moveItem(this, &editRemote,    0, dy); 
        moveItem(this, &checkShared,   0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_DATA_FORMAT),       0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_STATIC_REMOTE),     0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_STATIC_DATAFORMAT), 0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_OK),                0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_CANCEL),            0, dy);

        setWindowHeight(this, GetDlgItem(IDC_TASKS_OK));
    }

    if (hideAllAdvanced) {
        groupAdvanced.ShowWindow(SW_HIDE);
        editRemote.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_TASKS_STATIC_REMOTE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_TASKS_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_TASKS_DATA_FORMAT)->ShowWindow(SW_HIDE);

        CRect rect;
        groupAdvanced.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 10);

        moveItem(this, GetDlgItem(IDC_TASKS_OK),     0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_CANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_TASKS_OK));
    } 
    else if (hideDataFormats) {
        GetDlgItem(IDC_TASKS_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_TASKS_DATA_FORMAT)->ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        GetDlgItem(IDC_TASKS_STATIC_DATAFORMAT)->GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(&groupAdvanced, 0, dy);

        moveItem(this, GetDlgItem(IDC_TASKS_OK),     0, dy);
        moveItem(this, GetDlgItem(IDC_TASKS_CANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_TASKS_OK));
    }

    // Shared folders
    if (shared) {
        editRemote.EnableWindow(false);
    } else {
        GetDlgItem(IDC_TASKS_CHECK_SHARED)->ShowWindow(SW_HIDE);
    } 


    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupDirection.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupFolder.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupAdvanced.m_hWnd,L" ",L" ");
    };

    // Accessing Outlook, could be no more in foreground
    SetForegroundWindow();

    return FALSE;
}

void CTaskSettings::OnBnClickedTasksOk()
{
    // OK Button
    if (saveSettings(false)){
        CDialog::OnOK();
    }
    
}

void CTaskSettings::OnBnClickedTasksCancel()
{
    // Never read from winreg, will save when 'OK' is clicked on SyncSettings.
    // getConfig()->read();
    CDialog::OnCancel();
}


bool CTaskSettings::saveSettings(bool saveToDisk)
{
    CString remoteName, outlookFolder, syncType;
    CString s1;
    _bstr_t bst;
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(TASK_);

    GetDlgItemText(IDC_TASKS_EDIT_REMOTE, remoteName);
    GetDlgItemText(IDC_TASKS_EDIT_FOLDER, outlookFolder);

    // change values
    if(remoteName == ""){
        s1.LoadString(IDS_ERROR_SET_REMOTE_NAME);
        wsafeMessageBox(s1);
        return false;
    };

    if (UICustomization::showWarningOnChangeFromOneWay) {
        int currentSyncType = getSyncTypeIndex(ssconf->getSync());
        int newSyncType = lstSyncType.GetCurSel();
        if (checkOneWayToTwoWay(currentSyncType, newSyncType)) {
           return false;
        }
    }

    // sync source enabled
    ssconf->setSync(getSyncTypeName(lstSyncType.GetCurSel()));
    
    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* olFolder = toMultibyte(outlookFolder.GetBuffer());
    if (olFolder) {
        // If folder has changed, clear anchors
        if (UICustomization::clearAnchorsOnFolderChange) {
            const char * original = ssconf->getFolderPath();
            if (strcmp(original, olFolder) != 0) {
                ssconf->setLast(0);
                ssconf->setEndTimestamp(0);
            }
        }
        
        ssconf->setFolderPath(olFolder);
        delete [] olFolder;
    }

    if(checkInclude.GetCheck() == BST_CHECKED)
        ssconf->setUseSubfolders(true);
    else
        ssconf->setUseSubfolders(false);

    StringBuffer remName;
    remName.convert(remoteName.GetBuffer());
    if (!remName.null()) {
        ssconf->setURI(remName.c_str());
    } 

    // Never save to winreg, will save when 'OK' is clicked on SyncSettings.
    //if(saveToDisk)
    //    ((OutlookConfig*)getConfig())->save();

    return true;
}

int pickFolderTasks(){

    CString s1;
    try {
        outlookFolder = pickOutlookFolder(TASK);
    }
    catch (...) {
        printLog("Exception thrown by pickOutlookFolder", LOG_DEBUG);
        outlookFolder = L"";
    }

    if (wndTasks) {      // dialog could have been closed...
        if (outlookFolder != L""){
            s1 = outlookFolder.data();
            wndTasks->SetDlgItemText(IDC_TASKS_EDIT_FOLDER, s1);
        }
        wndTasks->SetForegroundWindow();
        wndTasks->EndModalState();
    }
    return 0;
}

void CTaskSettings::OnBnClickedTasksButSelect()
{
    outlookFolder = L"";
    BeginModalState();
    handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pickFolderTasks, NULL, 0, NULL);
}



void CTaskSettings::OnBnClickedTasksCheckShared() {
    long editId = IDC_TASKS_EDIT_REMOTE;

    CString currentValue;
    GetDlgItemText(editId, currentValue);
    CString warningMessage;
    warningMessage.LoadString(IDS_UNCHECK_SHARED);

    CString newValue = processSharedCheckboxClick(TASKS_REMOTE_NAME,
         checkShared.GetCheck() != 0, currentValue, warningMessage);

    SetDlgItemText(editId, newValue);
}

