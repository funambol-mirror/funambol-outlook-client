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

// ContactSettings.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ContactSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"
#include "SettingsHelper.h"

#include "vocl/WinItem.h"
#include "winmaincpp.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"
#include <string>
#include "UICustomization.h"

using namespace std;
// CContactSettings

static wstring outlookFolder;
static CContactSettings* wndContacts;
static HANDLE handleThread;

IMPLEMENT_DYNCREATE(CContactSettings, CDialog)

CContactSettings::CContactSettings()
	: CDialog(CContactSettings::IDD)
{
    handleThread = NULL;
}

CContactSettings::~CContactSettings()
{
    // clean stuff used in the select Outlook folder thread
    wndContacts = NULL;
    if (handleThread) {
        TerminateThread(handleThread, -1);
        CloseHandle(handleThread);
        handleThread = NULL;
    }
}

void CContactSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CONTACTS_COMBO_SYNCTYPE, lstSyncType);
    DDX_Control(pDX, IDC_CONTACTS_EDIT_FOLDER, editFolder);
    DDX_Control(pDX, IDC_CONTACTS_CHECK_INCLUDE, checkInclude);
    DDX_Control(pDX, IDC_CONTACTS_BUT_FOLDER, butSelectFolder);
    DDX_Control(pDX, IDC_CONTACTS_EDIT_REMOTE, editRemote);
    DDX_Control(pDX, IDC_CONTACTS_RADIO_SIF, radioSif);
    DDX_Control(pDX, IDC_CONTACTS_RADIO_VCARD, radioVcard);
    DDX_Control(pDX, IDC_CONTACTS_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_CONTACTS_GROUP_FOLDER, groupFolder);
    DDX_Control(pDX, IDC_CONTACTS_GROUP_ADVANCED, groupAdvanced);

    DDX_Control(pDX, IDC_CONTACTS_CHECK_SHARED, checkShared);
}

BEGIN_MESSAGE_MAP(CContactSettings, CDialog)
    ON_BN_CLICKED(IDC_CONTACTS_BUTOK, &CContactSettings::OnBnClickedContactsButok)
    ON_BN_CLICKED(IDC_CONTACTS_BUTCANCEL, &CContactSettings::OnBnClickedContactsButcancel)
    ON_BN_CLICKED(IDC_CONTACTS_BUT_FOLDER, &CContactSettings::OnBnClickedContactsButFolder)

    ON_BN_CLICKED(IDC_CONTACTS_RADIO_SIF, &CContactSettings::OnBnClickedContactsRadioSif)
    ON_BN_CLICKED(IDC_CONTACTS_RADIO_VCARD, &CContactSettings::OnBnClickedContactsRadioVcard)

    ON_BN_CLICKED(IDC_CONTACTS_CHECK_SHARED, &CContactSettings::OnBnClickedContactsCheckShared)
END_MESSAGE_MAP()


// CContactSettings diagnostics

#ifdef _DEBUG
void CContactSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CContactSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


// CContactSettings message handlers

BOOL CContactSettings::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_CONTACTS_DETAILS); SetWindowText(s1);
    CDialog::OnInitDialog();

    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(CONTACT_);

    editFolder.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editRemote.SetLimitText(EDIT_TEXT_MAXLENGTH);

    // load string resources
    s1.LoadString(IDS_SYNCTYPE1); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE2); lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE3); lstSyncType.AddString(s1);

    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_CONTACTS_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_CONTACTS_FOLDER);     SetDlgItemText(IDC_CONTACTS_GROUP_FOLDER, s1);
    s1.LoadString(IDS_CURRENT);             SetDlgItemText(IDC_CONTACTS_STATIC_FOLDER, s1);
    s1.LoadString(IDS_INCLUDE_SUBFOLDERS);  SetDlgItemText(IDC_CONTACTS_CHECK_INCLUDE, s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_CONTACTS_BUT_FOLDER, s1);
    s1.LoadString(IDS_REMOTE_NAME);         SetDlgItemText(IDC_CONTACTS_STATIC_REMOTE, s1);

    s1.LoadString(IDS_ADVANCED);            SetDlgItemText(IDC_CONTACTS_GROUP_ADVANCED, s1);
    s1.LoadString(IDS_DATA_FORMAT);         SetDlgItemText(IDC_CONTACTS_STATIC_DATAFORMAT, s1);
    s1.LoadString(IDS_USE_SIF);             SetDlgItemText(IDC_CONTACTS_RADIO_SIF, s1);
    s1.LoadString(IDS_USE_VCARD);           SetDlgItemText(IDC_CONTACTS_RADIO_VCARD, s1);
    
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_CONTACTS_BUTOK, s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_CONTACTS_BUTCANCEL, s1);

    s1.LoadString(IDS_SHARED);              SetDlgItemText(IDC_CONTACTS_CHECK_SHARED, s1);

    // load settings from Registry
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    
    // Get folder path.
    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* olFolder = toWideChar(ssconf->getFolderPath());
    s1 = olFolder;
    delete [] olFolder;
    try {
        if(s1 == ""){
            s1 = getDefaultFolderPath(CONTACT).data();
        }
    }
    catch (...){
        // an exception occured while trying to get the default folder
        EndDialog(-1);
    }

    SetDlgItemText(IDC_CONTACTS_EDIT_FOLDER, s1);
    
    if(ssconf->getUseSubfolders())
        checkInclude.SetCheck(BST_CHECKED);

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* remName = toWideChar(ssconf->getURI());
    s1 = remName;
    delete [] remName;
    SetDlgItemText(IDC_CONTACTS_EDIT_REMOTE, s1);

    if (s1.Right(wcslen(SHARED_SUFFIX)).Compare(SHARED_SUFFIX) == 0) {
        checkShared.SetCheck(BST_CHECKED);
    }

    if( strstr(ssconf->getType(),"sif") ){
        s1.LoadString(IDS_USE_SIF);
        SetDlgItemText(IDC_CONTACTS_DATA_FORMAT, s1);
        radioSif.SetCheck(BST_CHECKED);
        currentRadioChecked = SIF_CHECKED;
    }
    else {
        s1.LoadString(IDS_USE_VCARD);
        SetDlgItemText(IDC_CONTACTS_DATA_FORMAT, s1);
        radioVcard.SetCheck(BST_CHECKED);
        currentRadioChecked = VCARD_CHECKED;
    }

    // Hide the radio buttons of data format: only vCard (since 7.1.2).
    GetDlgItem(IDC_CONTACTS_RADIO_SIF)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CONTACTS_RADIO_VCARD)->ShowWindow(SW_HIDE);

    // Apply customizations
    bool shared             = UICustomization::shared;
    bool forceUseSubfolders = UICustomization::forceUseSubfolders;
    bool hideDataFormats    = UICustomization::hideDataFormats;
    bool hideAllAdvanced    = !SHOW_ADVANCED_SETTINGS;

    if (!shared) {
        GetDlgItem(IDC_CONTACTS_CHECK_SHARED)->ShowWindow(SW_HIDE);
    } else {
        editRemote.EnableWindow(false);
    }

    if (forceUseSubfolders) {
        checkInclude.SetCheck(BST_CHECKED);
        checkInclude.ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        checkInclude.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(GetDlgItem(IDC_CONTACTS_GROUP_FOLDER), 0, dy);

        moveItem(this, &groupAdvanced, 0, dy);
        moveItem(this, &editRemote,    0, dy);
        moveItem(this, &radioSif,      0, dy);
        moveItem(this, &radioVcard,    0, dy);
        moveItem(this, &checkShared,   0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_STATIC_REMOTE),     0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_STATIC_DATAFORMAT), 0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_DATA_FORMAT),       0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTOK),             0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTCANCEL),         0, dy);

        setWindowHeight(this, GetDlgItem(IDC_CONTACTS_BUTOK));
    }

    if (hideAllAdvanced) {
        groupAdvanced.ShowWindow(SW_HIDE);
        editRemote.ShowWindow(SW_HIDE);
        radioSif.ShowWindow(SW_HIDE);
        radioVcard.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CONTACTS_STATIC_REMOTE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CONTACTS_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);

        CRect rect;
        groupAdvanced.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 10);

        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTOK),     0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTCANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_CONTACTS_BUTOK));
    } else if (hideDataFormats) {
        radioSif.ShowWindow(SW_HIDE);
        radioVcard.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CONTACTS_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CONTACTS_DATA_FORMAT)->ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        GetDlgItem(IDC_CONTACTS_STATIC_DATAFORMAT)->GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(&groupAdvanced, 0, dy);

        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTOK),     0, dy);
        moveItem(this, GetDlgItem(IDC_CONTACTS_BUTCANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_CONTACTS_BUTOK));
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

    wndContacts = this;
    return FALSE;
}
void CContactSettings::OnBnClickedContactsButok()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    };
}

void CContactSettings::OnBnClickedContactsButcancel()
{
    // Never read from winreg, will save when 'OK' is clicked on SyncSettings.
    //getConfig()->read();
    CDialog::OnCancel();
}

bool CContactSettings::saveSettings(bool saveToDisk)
{
    CString remoteName, outlookFolder, syncType;
    CString s1;
    _bstr_t bst;
    bool useSif;
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(CONTACT_);

    GetDlgItemText(IDC_CONTACTS_EDIT_REMOTE, remoteName);
    GetDlgItemText(IDC_CONTACTS_EDIT_FOLDER, outlookFolder);

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

    if(checkInclude.GetCheck() == BST_CHECKED || UICustomization::forceUseSubfolders)
        ssconf->setUseSubfolders(true);
    else
        ssconf->setUseSubfolders(false);

    char* remName = toMultibyte(remoteName.GetBuffer());
    if (remName) {
        ssconf->setURI(remName);
        delete [] remName;
    }

    if(radioSif.GetCheck() == BST_CHECKED)
        useSif = true;
    else
        useSif = false;

    // Data formats
    if(useSif){
        char* version = toMultibyte(SIF_VERSION);
        ssconf->setType("text/x-s4j-sifc");
        ssconf->setVersion(version);
        ssconf->setEncoding("b64");
        delete [] version;
    }
    else{
        char* version = toMultibyte(VCARD_VERSION);
        ssconf->setType("text/x-vcard"); 
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

int pickFolderContacts(){

    CString s1;
    try {
        outlookFolder = pickOutlookFolder(CONTACT);
    }
    catch (...) {
        printLog("Exception thrown by pickOutlookFolder", LOG_DEBUG);
        outlookFolder = L"";
    }

    if (wndContacts) {      // dialog could have been closed...
        if (outlookFolder != L""){
            s1 = outlookFolder.data();
            wndContacts->SetDlgItemText(IDC_CONTACTS_EDIT_FOLDER, s1);
        }
        wndContacts->SetForegroundWindow();
        wndContacts->EndModalState();
    }
    return 0;
}

void CContactSettings::OnBnClickedContactsButFolder(){
    outlookFolder = L"";
    BeginModalState();
    handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pickFolderContacts, NULL, 0, NULL);
}

void CContactSettings::OnBnClickedContactsRadioSif() {
    if (currentRadioChecked != SIF_CHECKED) {
        SetDlgItemText(IDC_CONTACTS_EDIT_REMOTE, SIFC_DEFAULT_NAME);
        currentRadioChecked = SIF_CHECKED;
    }
}

void CContactSettings::OnBnClickedContactsRadioVcard() {
    if (currentRadioChecked != VCARD_CHECKED) {
        SetDlgItemText(IDC_CONTACTS_EDIT_REMOTE, VCARD_DEFAULT_NAME);
        currentRadioChecked = VCARD_CHECKED;
    }
}

void CContactSettings::OnBnClickedContactsCheckShared() {
    long editId = IDC_CONTACTS_EDIT_REMOTE;

    CString currentValue;
    GetDlgItemText(editId, currentValue);
    CString warningMessage;
    warningMessage.LoadString(IDS_UNCHECK_SHARED);

    CString newValue = processSharedCheckboxClick(CONTACTS_REMOTE_NAME,
         checkShared.GetCheck() != 0, currentValue, warningMessage);

    SetDlgItemText(editId, newValue);
}
