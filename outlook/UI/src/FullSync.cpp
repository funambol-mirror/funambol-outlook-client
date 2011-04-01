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
#include "FullSync.h"
#include "MainSyncFrm.h"
#include "SyncForm.h"
#include "ClientUtil.h"
#include "utils.h"
#include "UICustomization.h"

#include "winmaincpp.h"


IMPLEMENT_DYNAMIC(CFullSync, CDialog)

CFullSync::CFullSync(CWnd* pParent /*=NULL*/) : CDialog(CFullSync::IDD, pParent) {}

CFullSync::~CFullSync() {}

void CFullSync::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_CONTACTS, checkContacts);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_CALENDAR, checkCalendar);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_TASKS, checkTasks);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_NOTES, checkNotes);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_PICTURES,  checkPictures);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_VIDEOS,    checkVideos);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_FILES,     checkFiles);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO1, radio1);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO2, radio2);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO3, radio3);
    DDX_Control(pDX, IDC_FULLSYNC_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_FULLSYNC_GROUP_ITEMS, groupItems);
}


BEGIN_MESSAGE_MAP(CFullSync, CDialog)
    ON_BN_CLICKED(IDOK, &CFullSync::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CFullSync::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_CONTACTS, &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_CALENDAR, &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_TASKS,    &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_NOTES,    &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_PICTURES, &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_VIDEOS,   &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_FILES,    &CFullSync::OnBnClickedSourceCheckBox)
    ON_BN_CLICKED(IDC_FULLSYNC_RADIO3,         &CFullSync::OnBnClickedRefreshC2S)
    ON_BN_CLICKED(IDC_FULLSYNC_RADIO2,         &CFullSync::OnBnClickedRefreshS2C)
END_MESSAGE_MAP()

BOOL CFullSync::OnInitDialog() {

    CString s1;
    s1.LoadString(IDS_RECOVER); SetWindowText(s1);
    CDialog::OnInitDialog();

    s1.LoadString(IDS_FULLSYNC_SYNCTYPE1);  SetDlgItemText(IDC_FULLSYNC_RADIO1, s1);
    s1.LoadString(IDS_FULLSYNC_SYNCTYPE2);  SetDlgItemText(IDC_FULLSYNC_RADIO2, s1);
    s1.LoadString(IDS_FULLSYNC_SYNCTYPE3);  SetDlgItemText(IDC_FULLSYNC_RADIO3, s1);
    s1.LoadString(IDS_RECOVER_PERFORMS);    SetDlgItemText(IDC_FULLSYNC_STATIC_RECOVER, s1);
    s1.LoadString(IDS_ITEMS);               SetDlgItemText(IDC_FULLSYNC_GROUP_ITEMS, s1);
    s1.LoadString(IDS_DIRECTION);           SetDlgItemText(IDC_FULLSYNC_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_CONTACTS);            SetDlgItemText(IDC_FULLSYNC_CHECK_CONTACTS, s1);
    s1.LoadString(IDS_CALENDAR);            SetDlgItemText(IDC_FULLSYNC_CHECK_CALENDAR, s1);
    s1.LoadString(IDS_NOTES);               SetDlgItemText(IDC_FULLSYNC_CHECK_NOTES, s1);
    s1.LoadString(IDS_TASKS);               SetDlgItemText(IDC_FULLSYNC_CHECK_TASKS, s1);
    s1.LoadString(IDS_PICTURES);            SetDlgItemText(IDC_FULLSYNC_CHECK_PICTURES, s1);
    s1.LoadString(IDS_VIDEOS);              SetDlgItemText(IDC_FULLSYNC_CHECK_VIDEOS, s1);
    s1.LoadString(IDS_FILES);               SetDlgItemText(IDC_FULLSYNC_CHECK_FILES, s1);
    s1.LoadString(IDS_RECOVER);             SetDlgItemText(IDOK, s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDCANCEL, s1);
    
     // resize/move dynamically the source checkboxes
    adjustCheckboxes();


    if (UICustomization::defaultFullSyncFromClient) {
        // Refresh from Server is the default
        radio3.SetCheck(BST_CHECKED);
        radio3.SetFocus();
    } else {
        // Refresh from Server is the default
        radio2.SetCheck(BST_CHECKED);
        radio2.SetFocus();
    }

    // Grey out disabled sources
    if (isSourceEnabled(CONTACT_))     { checkContacts.EnableWindow(TRUE);  }
    else                               { checkContacts.EnableWindow(FALSE); }
    if (isSourceEnabled(APPOINTMENT_)) { checkCalendar.EnableWindow(TRUE);  }
    else                               { checkCalendar.EnableWindow(FALSE); }
    if (isSourceEnabled(TASK_))        { checkTasks.EnableWindow(TRUE);     }
    else                               { checkTasks.EnableWindow(FALSE);    }
    if (isSourceEnabled(NOTE_))        { checkNotes.EnableWindow(TRUE);     }
    else                               { checkNotes.EnableWindow(FALSE);    }
    checkPictures.EnableWindow(FALSE);  // C2S not available for pictures!
    checkVideos.EnableWindow(FALSE);    // C2S not available for videos!
    checkFiles.EnableWindow(FALSE);     // C2S not available for files!


    // Show/hide checkboxes
    if (!isSourceVisible(CONTACT)) {
        // checkContacts.ShowWindow(SW_HIDE);
        checkContacts.EnableWindow(FALSE);

    }
    if (!isSourceVisible(APPOINTMENT)) {
        // checkCalendar.ShowWindow(SW_HIDE);
        checkCalendar.EnableWindow(FALSE);
    }
    if (!isSourceVisible(TASK)) {
        // checkTasks.ShowWindow(SW_HIDE);
        checkTasks.EnableWindow(FALSE);
    }
    if (!isSourceVisible(NOTE)) {
        // checkNotes.ShowWindow(SW_HIDE);
        checkNotes.EnableWindow(FALSE);
    }
    //checkPictures.ShowWindow(SW_HIDE);
    //checkVideos.ShowWindow(SW_HIDE);
    //checkFiles.ShowWindow(SW_HIDE);
    
    if (!isSourceVisible(PICTURE)) {
        checkPictures.ShowWindow(SW_HIDE);
    }
    if (!isSourceVisible(VIDEO)) {
        checkVideos.ShowWindow(SW_HIDE);
    }
    if (!isSourceVisible(FILES)) {
        checkFiles.ShowWindow(SW_HIDE);
    }
    
    GetDlgItem(IDOK)->EnableWindow(FALSE);

    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupDirection.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupItems.m_hWnd,L" ",L" ");
    };

    return FALSE;
}


void CFullSync::OnBnClickedOk() {

    int pos=0;

    // "slow-sync": disabled
    if (radio1.GetCheck() == BST_CHECKED) {
        pos = 0;
    }
    // "refresh-from-server"
    else if (radio2.GetCheck() == BST_CHECKED) {

        if (UICustomization::confirmOnRefreshFromServer) {
            // Prompt a warning message...
            unsigned int flags = MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
            int selected = MessageBox(WMSG_BOX_REFRESH_FROM_SERVER, WPROGRAM_NAME, flags);
            if (selected == IDNO) {
                return;
            }
        }
        pos = 1;
    }

    // "refresh-from-client"
    else if (radio3.GetCheck() == BST_CHECKED) {
        if (UICustomization::confirmOnRefreshFromClient) {
            // Prompt a warning message...
            unsigned int flags = MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
            int selected = MessageBox(WMSG_BOX_REFRESH_FROM_CLIENT, WPROGRAM_NAME, flags);
            if (selected == IDNO) {
                return;
            }
        }
        pos = 2;
    }


    getConfig()->read();
    const char* fullSyncMode = getFullSyncTypeName(pos);
    
    ((CMainSyncFrame*)AfxGetMainWnd())->backupSyncModeSettings();
    
    // enable the checked sources, disable the unchecked ones
    if(checkContacts.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(CONTACT_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(CONTACT_)->setIsEnabled(false);
    }

    if(checkCalendar.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
    }

    if(checkTasks.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(TASK_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(TASK_)->setIsEnabled(false);
    }

    if(checkNotes.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(NOTE_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(NOTE_)->setIsEnabled(false);
    }

    if(checkPictures.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(PICTURE_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(PICTURE_)->setIsEnabled(false);
    }

    if(checkVideos.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(VIDEO_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(VIDEO_)->setIsEnabled(false);
    }

    if(checkFiles.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(FILES_)->setSync(fullSyncMode);
    }
    else {
        getConfig()->getSyncSourceConfig(FILES_)->setIsEnabled(false);
    }

    //getConfig()->setFullSync(true);

    ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->refreshSources();
    ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();

    OnOK();
}

void CFullSync::OnBnClickedCancel() {
    OnCancel();
}

void CFullSync::OnBnClickedSourceCheckBox() {

    if (isAtLeastOneSourceChecked()) {
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    } 
    else{
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
}

void CFullSync::OnBnClickedRefreshC2S() {

    checkPictures.EnableWindow(FALSE);
    checkPictures.SetCheck(BST_UNCHECKED);

    checkVideos.EnableWindow(FALSE);
    checkVideos.SetCheck(BST_UNCHECKED);

    checkFiles.EnableWindow(FALSE);
    checkFiles.SetCheck(BST_UNCHECKED);

    OnBnClickedSourceCheckBox();
}

void CFullSync::OnBnClickedRefreshS2C() {

    if (isSourceEnabled(PICTURE_)) { 
        checkPictures.EnableWindow(TRUE);
    }
    if (isSourceEnabled(VIDEO_)) { 
        checkVideos.EnableWindow(TRUE);
    }
    if (isSourceEnabled(FILES_)) { 
        checkFiles.EnableWindow(TRUE);
    }
}


bool CFullSync::isAtLeastOneSourceChecked() {

    if ( (checkContacts.GetCheck() == BST_CHECKED) || 
         (checkCalendar.GetCheck() == BST_CHECKED) ||
         (checkTasks.GetCheck()    == BST_CHECKED) || 
         (checkNotes.GetCheck()    == BST_CHECKED) ||
         (checkPictures.GetCheck() == BST_CHECKED) ||
         (checkVideos.GetCheck()   == BST_CHECKED) ||
         (checkFiles.GetCheck()    == BST_CHECKED) ) {
        return true;
    }
    else {
        return false;
    }
}


void CFullSync::adjustCheckboxes() {

    int numSources = countSourceVisible();
    if (isSourceEnabled(PICTURE_)) {
        numSources--;
    }
    if (isSourceEnabled(VIDEO_)) {
        numSources--;
    }
    if (isSourceEnabled(FILES_)) {
        numSources--;
    }
    // currently we consider at least 4 sources
    if (numSources < 4) {
        numSources = 4;
    }

    // Get the groupbox 'Items'
    CWnd* group = GetDlgItem(IDC_FULLSYNC_GROUP_ITEMS);
    CRect rectGroup;
    group->GetClientRect(&rectGroup);
    CPoint posGroup = getRelativePosition(group, this);
    
    CRect contactsRect;
    checkContacts.GetClientRect(&contactsRect);
    CPoint posContacts = getRelativePosition(&checkContacts, this);

    int offset1 = posGroup.x;                           // between left border <-> groupbox       
    int offset2 = posContacts.x - offset1;              // between groupbox <-> first checkbox
    int someSpace = 2;                                  // between checkboxes, to avoid overlapping!
    int totalWidth = rectGroup.Width();                 // The groupbox total width
    int width = (totalWidth - offset2) / numSources;    // The width of each checkbox

    int x  = posContacts.x;
    int y  = posContacts.y;
    int cx = width - someSpace;
    int cy = contactsRect.Height();
    
    //
    // TODO: once the sources are dynamically loaded, just cycle on
    //       the sources visible and SetWindowsPos on each one, like the last 2.
    // TODO: add minCx = 60 px
    //
    checkContacts.SetWindowPos(&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;
    checkCalendar.SetWindowPos(&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;
    checkTasks.SetWindowPos   (&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;

    int maxCx = offset1 + (totalWidth - x) - someSpace;
    cx = min(cx, maxCx);
    checkNotes.SetWindowPos   (&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;

    maxCx = offset1 + (totalWidth - x) - someSpace;
    cx = min(cx, maxCx);
    checkPictures.SetWindowPos(&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;

    maxCx = offset1 + (totalWidth - x) - someSpace;
    cx = min(cx, maxCx);
    checkVideos.SetWindowPos(&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
    x = x + width;

    maxCx = offset1 + (totalWidth - x) - someSpace;
    cx = min(cx, maxCx);
    checkFiles.SetWindowPos(&CWnd::wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
}

