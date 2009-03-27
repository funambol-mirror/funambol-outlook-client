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

// NotesSettings.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "NotesSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"

#include "winmaincpp.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"

#include <string>

using namespace std;
// CNotesSettings

static wstring outlookFolder;
static CNotesSettings* wndNotes;
static HANDLE handleThread;

IMPLEMENT_DYNCREATE(CNotesSettings, CDialog)

CNotesSettings::CNotesSettings()
	: CDialog(CNotesSettings::IDD)
{
    handleThread = NULL;
}

CNotesSettings::~CNotesSettings()
{
    // clean stuff used in the select Outlook folder thread
    wndNotes = NULL;
    if (handleThread) {
        TerminateThread(handleThread, -1);
        CloseHandle(handleThread);
        handleThread = NULL;
    }
}

void CNotesSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_NOTES_COMBO_SYNCTYPE, lstSyncType);
    DDX_Control(pDX, IDC_NOTES_EDIT_FOLDER, editFolder);
    DDX_Control(pDX, IDC_NOTES_CHECK_INCLUDE, checkInclude);
    DDX_Control(pDX, IDC_NOTES_BUT_SELECT, butSelectFolder);
    DDX_Control(pDX, IDC_NOTES_EDIT_REMOTE, editRemote);
    DDX_Control(pDX, IDC_NOTES_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_NOTES_GROUP_FOLDER, groupFolder);
    DDX_Control(pDX, IDC_NOTES_GROUP_ADVANCED, groupAdvanced);
    DDX_Control(pDX, IDC_NOTES_RADIO_SIF, radioSif);
    DDX_Control(pDX, IDC_NOTES_RADIO_VNOTE, radioVNote);
}

BEGIN_MESSAGE_MAP(CNotesSettings, CDialog)
    ON_BN_CLICKED(IDC_NOTES_OK, &CNotesSettings::OnBnClickedNotesOk)
    ON_BN_CLICKED(IDC_NOTES_CANCEL, &CNotesSettings::OnBnClickedNotesCancel)
    ON_BN_CLICKED(IDC_NOTES_BUT_SELECT, &CNotesSettings::OnBnClickedNotesButSelect)
    ON_BN_CLICKED(IDC_NOTES_RADIO_VNOTE, &CNotesSettings::OnBnClickedNotesRadioVNote)
    ON_BN_CLICKED(IDC_NOTES_RADIO_SIF, &CNotesSettings::OnBnClickedNotesRadioSif)
END_MESSAGE_MAP()

// CNotesSettings diagnostics

#ifdef _DEBUG
void CNotesSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CNotesSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


// CNotesSettings message handlers

BOOL CNotesSettings::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_NOTES_DETAILS); SetWindowText(s1);
    CDialog::OnInitDialog();
    
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(NOTE_);

    editFolder.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editRemote.SetLimitText(EDIT_TEXT_MAXLENGTH);
    
    // load string resources
    s1.LoadString(IDS_SYNCTYPE1); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE2); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE3); lstSyncType.AddString(s1);
    
    s1.LoadString(IDS_SYNC_DIRECTION); SetDlgItemText(IDC_NOTES_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_NOTES_FOLDER); SetDlgItemText(IDC_NOTES_GROUP_FOLDER, s1);
    s1.LoadString(IDS_CURRENT); SetDlgItemText(IDC_NOTES_STATIC_FOLDER, s1);
    s1.LoadString(IDS_INCLUDE_SUBFOLDERS); SetDlgItemText(IDC_NOTES_CHECK_INCLUDE, s1);
    s1.LoadString(IDS_SELECT_FOLDER); SetDlgItemText(IDC_NOTES_BUT_SELECT, s1);
    s1.LoadString(IDS_REMOTE_NAME); SetDlgItemText(IDC_NOTES_STATIC_REMOTE, s1);
    s1.LoadString(IDS_DATA_FORMAT); SetDlgItemText(IDC_NOTES_STATIC_DATAFORMAT, s1);
    s1.LoadString(IDS_ADVANCED); SetDlgItemText(IDC_NOTES_GROUP_ADVANCED, s1);

    s1.LoadString(IDS_OK); SetDlgItemText(IDC_NOTES_OK, s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDC_NOTES_CANCEL, s1);

    // load settings from Registry
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    
    // Get folder path.
    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* olFolder = toWideChar(ssconf->getFolderPath());
    s1 = olFolder;
    delete [] olFolder;
    
    try {
        if(s1 == ""){
            s1 = getDefaultFolderPath(NOTE).data();
        }
    }
    catch (...){
        // an exception occured while trying to get the default folder
    	EndDialog(-1);
    }
    
    SetDlgItemText(IDC_NOTES_EDIT_FOLDER, s1);

    if(ssconf->getUseSubfolders())
        checkInclude.SetCheck(BST_CHECKED);

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* remName = toWideChar(ssconf->getURI());
    s1 = remName;
    delete [] remName;
    SetDlgItemText(IDC_NOTES_EDIT_REMOTE, s1);

    wndNotes = this;


    s1.LoadString(IDS_USE_SIF);    SetDlgItemText(IDC_NOTES_RADIO_SIF,   s1);
    s1.LoadString(IDS_USE_VNOTE);  SetDlgItemText(IDC_NOTES_RADIO_VNOTE, s1);
    if( strstr(ssconf->getType(),"sif") ){
        radioSif.SetCheck(BST_CHECKED);
        currentRadioChecked = SIF_CHECKED;
    }
    else {
        radioVNote.SetCheck(BST_CHECKED);
        currentRadioChecked = VNOTE_CHECKED;
    }
    
    //
    // Hide Advanced settings for portal build
    //
    if(getConfig()->checkPortalBuild()) {
        groupAdvanced.ShowWindow(SW_HIDE);
        editRemote.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_NOTES_STATIC_REMOTE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_NOTES_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        radioSif.ShowWindow(SW_HIDE);
        radioVNote.ShowWindow(SW_HIDE);

        // Redraw buttons 'OK' and 'Cancel' where the groupAdvanced was located
        CPoint posAdvanced = getRelativePosition(&groupAdvanced, this);
        int top = posAdvanced.y + 10;   // 10 = some space

        CWnd* butOk     = GetDlgItem(IDC_NOTES_OK);
        CWnd* butCancel = GetDlgItem(IDC_NOTES_CANCEL);
        CRect rectDialog, rectOk;
        GetClientRect(&rectDialog);
        butOk->GetClientRect(&rectOk);

        CPoint posOk     = getRelativePosition(butOk,     this);
        CPoint posCancel = getRelativePosition(butCancel, this);
        butOk->SetWindowPos(&CWnd::wndTop, posOk.x, top, NULL, NULL, SWP_SHOWWINDOW | SWP_NOSIZE);
        butCancel->SetWindowPos(&CWnd::wndTop, posCancel.x, top, NULL, NULL, SWP_SHOWWINDOW | SWP_NOSIZE);

        // Resize window, now it's smaller
        int newHeight = top + rectOk.Height() + 50;     // 50 = some space
        this->SetWindowPos(&CWnd::wndTop, NULL, NULL, rectDialog.Width(), newHeight, SWP_SHOWWINDOW | SWP_NOMOVE);
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


void CNotesSettings::OnBnClickedNotesOk()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CNotesSettings::OnBnClickedNotesCancel()
{
    // Never read from winreg, will save when 'OK' is clicked on SyncSettings.
    //getConfig()->read();
    CDialog::OnCancel();
}

bool CNotesSettings::saveSettings(bool saveToDisk)
{
    CString remoteName, outlookFolder, syncType;
    CString s1;
    _bstr_t bst;
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(NOTE_);

    GetDlgItemText(IDC_NOTES_EDIT_REMOTE, remoteName);
    GetDlgItemText(IDC_NOTES_EDIT_FOLDER, outlookFolder);

    // change values
    if(remoteName == ""){
        s1.LoadString(IDS_ERROR_SET_REMOTE_NAME);
        MessageBox(s1);
        return false;
    };
    // sync source enabled
    ssconf->setSync(getSyncTypeName(lstSyncType.GetCurSel()));

    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* olFolder = toMultibyte(outlookFolder.GetBuffer());
    if (olFolder) {
        ssconf->setFolderPath(olFolder);
        delete [] olFolder;
    }

    if(checkInclude.GetCheck() == BST_CHECKED)
        ssconf->setUseSubfolders(true);
    else
        ssconf->setUseSubfolders(false);

    char* remName = toMultibyte(remoteName.GetBuffer());
    if (remName) {
        ssconf->setURI(remName);
        delete [] remName;
    }


    // Data formats
    if(radioSif.GetCheck() == BST_CHECKED){
        char* version = toMultibyte(SIF_VERSION);
        ssconf->setType("text/x-s4j-sifn");
        ssconf->setVersion(version);
        ssconf->setEncoding("b64");
        delete [] version;
    }
    else{
        char* version = toMultibyte(VNOTE_VERSION);
        ssconf->setType("text/x-vnote"); 
        ssconf->setVersion(version);
        // When encryption is used, encoding is always 'base64'.
        if ( !strcmp(ssconf->getEncryption(), "") ) {
            ssconf->setEncoding("bin");
        }
        else {
            ssconf->setEncoding("b64");
        }
        delete [] version;
    }

    // Never save to winreg, will save when 'OK' is clicked on SyncSettings.
    //if(saveToDisk)
    //    ((OutlookConfig*)getConfig())->save();

    return true;
}

int pickFolderNotes(){

    CString s1;
    try {
        outlookFolder = pickOutlookFolder(NOTE);
    }
    catch (...) {
        printLog("Exception thrown by pickOutlookFolder", LOG_DEBUG);
        outlookFolder = L"";
    }

    if (wndNotes) {     // dialog could have been closed...
        if (outlookFolder != L""){
            s1 = outlookFolder.data();
            wndNotes->SetDlgItemText(IDC_NOTES_EDIT_FOLDER, s1);
        }
        wndNotes->SetForegroundWindow();
        wndNotes->EndModalState();
    }
    return 0;
}

void CNotesSettings::OnBnClickedNotesButSelect()
{
    outlookFolder = L"";
    BeginModalState();
    handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pickFolderNotes, NULL, 0, NULL);
}

void CNotesSettings::OnBnClickedNotesRadioSif() {
    if (currentRadioChecked != SIF_CHECKED) {
        SetDlgItemText(IDC_NOTES_EDIT_REMOTE, SIFN_DEFAULT_NAME);
        currentRadioChecked = SIF_CHECKED;
    }
}

void CNotesSettings::OnBnClickedNotesRadioVNote() {
    if (currentRadioChecked != VNOTE_CHECKED) {
        SetDlgItemText(IDC_NOTES_EDIT_REMOTE, VNOTE_DEFAULT_NAME);
        currentRadioChecked = VNOTE_CHECKED;
    }
}
