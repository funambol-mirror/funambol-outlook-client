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
#include "CalendarSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"
#include "DateFilter.h"

#include "winmaincpp.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"

#include <string>

using namespace std;

static wstring outlookFolder;
static CCalendarSettings* wndCalendar;
static HANDLE handleThread;

IMPLEMENT_DYNCREATE(CCalendarSettings, CDialog)

CCalendarSettings::CCalendarSettings()
	: CDialog(CCalendarSettings::IDD)
{
    handleThread = NULL;
}

CCalendarSettings::~CCalendarSettings()
{
    // clean stuff used in the select Outlook folder thread
    wndCalendar = NULL;
    if (handleThread) {
        TerminateThread(handleThread, -1);
        CloseHandle(handleThread);
        handleThread = NULL;
    }
}

void CCalendarSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CALENDAR_COMBO_SYNCTYPE,  lstSyncType);
    DDX_Control(pDX, IDC_CALENDAR_EDIT_FOLDER,     editFolder);
    DDX_Control(pDX, IDC_CALENDAR_CHECK_INCLUDE,   checkIncludeSubfolders);
    DDX_Control(pDX, IDC_CALENDAR_BUT_SELECT,      butSelectFolder);
    DDX_Control(pDX, IDC_CALENDAR_EDIT_REMOTE,     editRemote);
    DDX_Control(pDX, IDC_CALENDAR_RADIO_SIF,       radioSif);
    DDX_Control(pDX, IDC_CALENDAR_RADIO_VCARD,     radioVcard);
    DDX_Control(pDX, IDC_CALENDAR_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_CALENDAR_GROUP_FOLDER,    groupFolder);
    DDX_Control(pDX, IDC_CALENDAR_GROUP_ADVANCED,  groupAdvanced);
    DDX_Control(pDX, IDC_CALENDAR_COMBO_FILTER,    lstFilter);
    DDX_Control(pDX, IDC_CALENDAR_GROUP_FILTER,    groupFilter);
}

BEGIN_MESSAGE_MAP(CCalendarSettings, CDialog)
    ON_BN_CLICKED(IDC_CALENDAR_BUTOK,       &CCalendarSettings::OnBnClickedCalendarButok)
    ON_BN_CLICKED(IDC_CALENDAR_BUTCANCEL,   &CCalendarSettings::OnBnClickedCalendarButcancel)
    ON_BN_CLICKED(IDC_CALENDAR_BUT_SELECT,  &CCalendarSettings::OnBnClickedCalendarButSelect)
    ON_BN_CLICKED(IDC_CALENDAR_RADIO_VCARD, &CCalendarSettings::OnBnClickedCalendarRadioVcard)
    ON_BN_CLICKED(IDC_CALENDAR_RADIO_SIF,   &CCalendarSettings::OnBnClickedCalendarRadioSif)
END_MESSAGE_MAP()

#ifdef _DEBUG
void CCalendarSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CCalendarSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


// CCalendarSettings message handlers

BOOL CCalendarSettings::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_CALENDAR_DETAILS); SetWindowText(s1);
    CDialog::OnInitDialog();
    
    
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(APPOINTMENT_);
    
    editFolder.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editRemote.SetLimitText(EDIT_TEXT_MAXLENGTH);
    
    // load string resources
    s1.LoadString(IDS_SYNCTYPE1);           lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE2);           lstSyncType.AddString(s1);
    s1.LoadString(IDS_SYNCTYPE3);           lstSyncType.AddString(s1);

    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_CALENDAR_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_CURRENT);             SetDlgItemText(IDC_CALENDAR_STATIC_FOLDER, s1);
    s1.LoadString(IDS_INCLUDE_SUBFOLDERS);  SetDlgItemText(IDC_CALENDAR_CHECK_INCLUDE, s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_CALENDAR_BUT_SELECT, s1);
    s1.LoadString(IDS_REMOTE_NAME);         SetDlgItemText(IDC_CALENDAR_STATIC_REMOTE, s1);
    s1.LoadString(IDS_ADVANCED);            SetDlgItemText(IDC_CALENDAR_GROUP_ADVANCED, s1);
    s1.LoadString(IDS_CALENDAR_FOLDER);     SetDlgItemText(IDC_CALENDAR_GROUP_FOLDER, s1);
    s1.LoadString(IDS_EVENT_FILTER);        SetDlgItemText(IDC_CALENDAR_GROUP_FILTER, s1);
    s1.LoadString(IDS_SYNC_PAST_EVENTS);    SetDlgItemText(IDC_CALENDAR_STATIC_FILTER, s1);

    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_CALENDAR_BUTOK, s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_CALENDAR_BUTCANCEL, s1);

    s1.LoadString(IDS_DATE_FILTER_NONE);            lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_LAST_WEEK);       lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_LAST_2_WEEKS);    lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_LAST_MONTH);      lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_LAST_3_MONTHS);   lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_LAST_6_MONTHS);   lstFilter.AddString(s1);
    s1.LoadString(IDS_DATE_FILTER_ALL);             lstFilter.AddString(s1);

    // Set dropdown-lists initial position
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    lstFilter.SetCurSel(getDateFilterIndex(ssconf->getDateFilter().getRelativeLowerDate()));

    
    // Get folder path.
    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* olFolder = toWideChar(ssconf->getFolderPath());
    s1 = olFolder;
    delete [] olFolder;

    try {
        if(s1 == ""){
            s1 = getDefaultFolderPath(APPOINTMENT).data();
        }
    }
    catch (...){
        // an exception occured while trying to get the default folder
        EndDialog(-1);
    }

    SetDlgItemText(IDC_CALENDAR_EDIT_FOLDER, s1);

    if(ssconf->getUseSubfolders())
        checkIncludeSubfolders.SetCheck(BST_CHECKED);

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* remName = toWideChar(ssconf->getURI());
    s1 = remName;
    delete [] remName;
    SetDlgItemText(IDC_CALENDAR_EDIT_REMOTE, s1);

    s1.LoadString(IDS_DATA_FORMAT); SetDlgItemText(IDC_CALENDAR_STATIC_DATAFORMAT, s1);
    s1.LoadString(IDS_USE_SIF);     SetDlgItemText(IDC_CALENDAR_RADIO_SIF, s1);
    s1.LoadString(IDS_USE_VCAL);    SetDlgItemText(IDC_CALENDAR_RADIO_VCARD, s1);

    if( strstr(ssconf->getType(),"sif") ){
        radioSif.SetCheck(BST_CHECKED);
        currentRadioChecked = SIF_CHECKED;
    }
    else {
        radioVcard.SetCheck(BST_CHECKED);
        currentRadioChecked = VCALENDAR_CHECKED;
    }


    //
    // Hide Advanced settings for portal build
    //
    if(getConfig()->checkPortalBuild()) {
        groupAdvanced.ShowWindow(SW_HIDE);
        editRemote.ShowWindow(SW_HIDE);
        radioSif.ShowWindow(SW_HIDE);
        radioVcard.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CALENDAR_STATIC_REMOTE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CALENDAR_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);

        // Redraw buttons 'OK' and 'Cancel' where the groupAdvanced was located
        CPoint posAdvanced = getRelativePosition(&groupAdvanced, this);
        int top = posAdvanced.y + 10;   // 10 = some space

        CWnd* butOk     = GetDlgItem(IDC_CALENDAR_BUTOK);
        CWnd* butCancel = GetDlgItem(IDC_CALENDAR_BUTCANCEL);
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
        pfnSetWindowTheme (groupFilter.m_hWnd,L" ",L" ");
    };

    // Accessing Outlook, could be no more in foreground
    SetForegroundWindow();

    wndCalendar = this;
    return FALSE;
}

void CCalendarSettings::OnBnClickedCalendarButok()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CCalendarSettings::OnBnClickedCalendarButcancel()
{
    // Never read from winreg, will save when 'OK' is clicked on SyncSettings.
    //getConfig()->read();
    CDialog::OnCancel();
}


bool CCalendarSettings::saveSettings(bool saveToDisk)
{
    CString remoteName, outlookFolder, syncType;
    CString s1;
    _bstr_t bst;
    bool useSif;
    WindowsSyncSourceConfig* ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(APPOINTMENT_);

    GetDlgItemText(IDC_CALENDAR_EDIT_REMOTE, remoteName);
    GetDlgItemText(IDC_CALENDAR_EDIT_FOLDER, outlookFolder);

    // change values
    if(remoteName == ""){
        // remote name is empty
        s1.LoadString(IDS_ERROR_SET_REMOTE_NAME);
        MessageBox(s1);
        return false;
    }

    // sync source enabled
    ssconf->setSync(getSyncTypeName(lstSyncType.GetCurSel()));

    // Date Filter
    int filterPos = lstFilter.GetCurSel();
    ssconf->getDateFilter().setRelativeLowerDate(getDateFilterValue(filterPos));


    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* olFolder = toMultibyte(outlookFolder.GetBuffer());
    if (olFolder) {
        ssconf->setFolderPath(olFolder);
        delete [] olFolder;
    }

    if(checkIncludeSubfolders.GetCheck() == BST_CHECKED){
        ssconf->setUseSubfolders(true);
    }
    else{
        ssconf->setUseSubfolders(false);
    }

    char* remName = toMultibyte(remoteName.GetBuffer());
    if (remName) {
        ssconf->setURI(remName);
        delete [] remName;
    }

    if(radioSif.GetCheck() == BST_CHECKED){
        useSif = true;
    }
    else{
        useSif = false;
    }

    // Data formats
    if(useSif){
        char* version = toMultibyte(SIF_VERSION);
        ssconf->setType("text/x-s4j-sife");
        ssconf->setVersion(version);
        ssconf->setEncoding("b64");
        delete [] version;
    }
    else{
        char* version = toMultibyte(VCALENDAR_VERSION);
        ssconf->setType("text/x-vcalendar"); 
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

int pickFolderCalendar(){

    CString s1;
    try {
        outlookFolder = pickOutlookFolder(APPOINTMENT);
    }
    catch (...) {
        printLog("Exception thrown by pickOutlookFolder", LOG_DEBUG);
        outlookFolder = L"";
    }

    if (wndCalendar) {      // dialog could have been closed...
        if (outlookFolder != L""){
            s1 = outlookFolder.c_str();
            wndCalendar->SetDlgItemText(IDC_CALENDAR_EDIT_FOLDER, s1);
        }
        wndCalendar->SetForegroundWindow();
        wndCalendar->EndModalState();
    }
    return 0;
}

void CCalendarSettings::OnBnClickedCalendarButSelect(){
    outlookFolder = L"";
    BeginModalState();
    handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pickFolderCalendar, NULL, 0, NULL);
}



void CCalendarSettings::OnBnClickedCalendarRadioSif() {
    if (currentRadioChecked != SIF_CHECKED) {
        SetDlgItemText(IDC_CALENDAR_EDIT_REMOTE, SIFE_DEFAULT_NAME);
        currentRadioChecked = SIF_CHECKED;
    }
}

void CCalendarSettings::OnBnClickedCalendarRadioVcard() {
    if (currentRadioChecked != VCALENDAR_CHECKED) {
        SetDlgItemText(IDC_CALENDAR_EDIT_REMOTE, VCALENDAR_DEFAULT_NAME);
        currentRadioChecked = VCALENDAR_CHECKED;
    }
}
