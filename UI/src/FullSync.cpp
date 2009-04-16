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

// FullSync.cpp : implementation file
//

#include "stdafx.h"
#include "FullSync.h"
#include "MainSyncFrm.h"
#include "SyncForm.h"
#include "ClientUtil.h"

#include "winmaincpp.h"


// CFullSync dialog

IMPLEMENT_DYNAMIC(CFullSync, CDialog)

CFullSync::CFullSync(CWnd* pParent /*=NULL*/)
	: CDialog(CFullSync::IDD, pParent)
{

}

CFullSync::~CFullSync()
{
}

void CFullSync::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_CONTACTS, checkContacts);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_CALENDAR, checkCalendar);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_TASKS, checkTasks);
    DDX_Control(pDX, IDC_FULLSYNC_CHECK_NOTES, checkNotes);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO1, radio1);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO2, radio2);
    DDX_Control(pDX, IDC_FULLSYNC_RADIO3, radio3);
    DDX_Control(pDX, IDC_FULLSYNC_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_FULLSYNC_GROUP_ITEMS, groupItems);
}


BEGIN_MESSAGE_MAP(CFullSync, CDialog)
    ON_BN_CLICKED(IDOK, &CFullSync::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CFullSync::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_CONTACTS, &CFullSync::OnBnClickedFullsyncCheckContacts)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_CALENDAR, &CFullSync::OnBnClickedFullsyncCheckCalendar)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_TASKS, &CFullSync::OnBnClickedFullsyncCheckTasks)
    ON_BN_CLICKED(IDC_FULLSYNC_CHECK_NOTES, &CFullSync::OnBnClickedFullsyncCheckNotes)
END_MESSAGE_MAP()

BOOL CFullSync::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_RECOVER); SetWindowText(s1);
    CDialog::OnInitDialog();

    s1.LoadString(IDS_FULLSYNC_SYNCTYPE1); SetDlgItemText(IDC_FULLSYNC_RADIO1, s1);
    s1.LoadString(IDS_FULLSYNC_SYNCTYPE2); SetDlgItemText(IDC_FULLSYNC_RADIO2, s1);
    s1.LoadString(IDS_FULLSYNC_SYNCTYPE3); SetDlgItemText(IDC_FULLSYNC_RADIO3, s1);
    s1.LoadString(IDS_RECOVER_PERFORMS); SetDlgItemText(IDC_FULLSYNC_STATIC_RECOVER, s1);
    s1.LoadString(IDS_ITEMS); SetDlgItemText(IDC_FULLSYNC_GROUP_ITEMS, s1);
    s1.LoadString(IDS_DIRECTION); SetDlgItemText(IDC_FULLSYNC_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_CONTACTS); SetDlgItemText(IDC_FULLSYNC_CHECK_CONTACTS, s1);
    s1.LoadString(IDS_CALENDAR); SetDlgItemText(IDC_FULLSYNC_CHECK_CALENDAR, s1);
    s1.LoadString(IDS_NOTES); SetDlgItemText(IDC_FULLSYNC_CHECK_NOTES, s1);
    s1.LoadString(IDS_TASKS); SetDlgItemText(IDC_FULLSYNC_CHECK_TASKS, s1);

    s1.LoadString(IDS_RECOVER); SetDlgItemText(IDOK, s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDCANCEL, s1);
    
    // settings
    checkContacts.EnableWindow(TRUE);
    checkCalendar.EnableWindow(TRUE);
    //if (getConfig()->checkPortalBuild()) {
    //    checkNotes.EnableWindow(FALSE);
    //}
    //else {
        checkNotes.EnableWindow(TRUE);
    //}
    
    // Refresh from Client is the default
    radio3.SetCheck(BST_CHECKED);
    radio3.SetFocus();

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

        // Prompt a warning message...
        unsigned int flags = MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
        int selected = MessageBox(WMSG_BOX_REFRESH_FROM_SERVER, WPROGRAM_NAME, flags);
        if (selected == IDNO) {
            return;
        }
        pos = 1;
    }

    // "refresh-from-client"
    else if (radio3.GetCheck() == BST_CHECKED) {
        pos = 2;
    }


    getConfig()->read();

    // enable the checked sources, disable the unchecked ones
    if(checkContacts.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(CONTACT_)->setSync(getFullSyncTypeName(pos));
    }
    else {
        getConfig()->getSyncSourceConfig(CONTACT_)->setIsEnabled(false);
    }

    if(checkCalendar.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setSync(getFullSyncTypeName(pos));
    }
    else {
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
    }

    if(checkTasks.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(TASK_)->setSync(getFullSyncTypeName(pos));
    }
    else {
        getConfig()->getSyncSourceConfig(TASK_)->setIsEnabled(false);
    }

    if(checkNotes.GetCheck() == BST_CHECKED) {
        getConfig()->getSyncSourceConfig(NOTE_)->setSync(getFullSyncTypeName(pos));
    }
    else {
        getConfig()->getSyncSourceConfig(NOTE_)->setIsEnabled(false);
    }

    // TODO: add check for pictures
    getConfig()->getSyncSourceConfig(PICTURE_)->setIsEnabled(false);


    getConfig()->setFullSync(true);

    ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->refreshSources();
    ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();

    OnOK();
}

void CFullSync::OnBnClickedCancel(){
    OnCancel();
}

void CFullSync::OnBnClickedFullsyncCheckContacts()
{
    if( (checkContacts.GetCheck() == BST_UNCHECKED) && (checkCalendar.GetCheck() == BST_UNCHECKED) &&
        (checkNotes.GetCheck() == BST_UNCHECKED) && (checkTasks.GetCheck() == BST_UNCHECKED) ){
           
        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    else{
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
}

void CFullSync::OnBnClickedFullsyncCheckCalendar()
{
    if( (checkContacts.GetCheck() == BST_UNCHECKED) && (checkCalendar.GetCheck() == BST_UNCHECKED) &&
        (checkNotes.GetCheck() == BST_UNCHECKED) && (checkTasks.GetCheck() == BST_UNCHECKED) ){

        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    else{
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
}

void CFullSync::OnBnClickedFullsyncCheckTasks()
{
    if( (checkContacts.GetCheck() == BST_UNCHECKED) && (checkCalendar.GetCheck() == BST_UNCHECKED) &&
        (checkNotes.GetCheck() == BST_UNCHECKED) && (checkTasks.GetCheck() == BST_UNCHECKED) ){
            
            GetDlgItem(IDOK)->EnableWindow(FALSE);
    }
    else{
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
}

void CFullSync::OnBnClickedFullsyncCheckNotes()
{
    if( (checkContacts.GetCheck() == BST_UNCHECKED) && (checkCalendar.GetCheck() == BST_UNCHECKED) &&
        (checkNotes.GetCheck() == BST_UNCHECKED) && (checkTasks.GetCheck() == BST_UNCHECKED) ){

        GetDlgItem(IDOK)->EnableWindow(FALSE);
    }        
    else{
        GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
}

