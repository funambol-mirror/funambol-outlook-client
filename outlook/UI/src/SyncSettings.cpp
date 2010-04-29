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

// SyncSettings.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SyncSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"

#include "LeftView.h"
#include "ContactSettings.h"
#include "CalendarSettings.h"
#include "NotesSettings.h"
#include "TaskSettings.h"
#include "PicturesSettings.h"

#include "customization.h"
#include "winmaincpp.h"
#include "utils.h"
#include "OutlookPlugin.h"


// Values for the scheduler
static StringBuffer minutesString = SCHEDULED_MINUTES_VALUES;
static StringBuffer hoursString   = SCHEDULED_HOURS_VALUES;
static ArrayList minutesA, hoursA;
static int defaultPosition = 0;
static bool initialized = false;

bool hasDefault(StringBuffer* value) {  
    if (value->find("fault") != StringBuffer::npos) {
        return true;
    }
    return false;
}

StringBuffer removeDefaultString(StringBuffer& value) {
    StringBuffer ret = value;
    size_t pos = value.find("(");
    if (pos != StringBuffer::npos) {
        ret = value.substr(0, pos);
    }
    return ret;
}

void populateArrays() {
    
    ArrayList tmp;
    StringBuffer* el; 
    StringBuffer value, completeValue;
    int res = 0, minSize = 0, hourSize = 0, totalSize = 0;

    // safe check to remove element that are number <5 and >59
    if (!minutesString.empty()) {
    minutesString.split(tmp, ",");    
    for (el = (StringBuffer *)tmp.front(); el; el = (StringBuffer *)tmp.next() ) {
        completeValue = el->c_str();
        value = removeDefaultString(completeValue);
        res = atoi(value.c_str());
        if (res >= 5 && res <= 59) {
            minutesA.add(completeValue);
        }
    }
    }

    tmp.clear();
    if (!hoursString.empty()) {
    hoursString.split(tmp, ",");
    for (el = (StringBuffer *)tmp.front(); el; el = (StringBuffer *)tmp.next() ) {
        completeValue = el->c_str();
        value = removeDefaultString(completeValue);
        res = atoi(value.c_str());
        if (res >= 1 && res <= 24) {
            hoursA.add(completeValue);
        }
    }
    }
    minSize = minutesA.size();
    hourSize = hoursA.size();
    totalSize = minSize + hourSize;

    for (int i = 0; i < totalSize; i++) {
        if (i < minSize) {
            if (hasDefault((StringBuffer*)minutesA.get(i))) {
                StringBuffer v = removeDefaultString(*((StringBuffer*)minutesA.get(i)));
                minutesA.removeElementAt(i);
                minutesA.add(i, v);
                if (defaultPosition == 0) {
                    defaultPosition = i;
                }
            }
        } else {
            if (hasDefault((StringBuffer*)hoursA.get(i-minSize))) {
                StringBuffer v = removeDefaultString(*((StringBuffer*)hoursA.get(i-minSize)));
                hoursA.removeElementAt(i-minSize);                
                hoursA.add(i-minSize, v);
                if (defaultPosition == 0) {
                    defaultPosition = i;
                }
            }
    }
}

}

int getSchedulerPosition(int minutes) {

    int minSize = minutesA.size();
    int hourSize = hoursA.size();
    int totalSize = minSize + hourSize;
    StringBuffer* s = NULL;
    int res = 0, position = 0; 

    for (int i = 0; i < totalSize; i++) {
        if (i < minSize) {
            s = (StringBuffer*)minutesA.get(i);
            res = atoi(s->c_str());
            if (minutes <= res) {
                position = i;
                break;
            }            
        } else {
            s = (StringBuffer*)hoursA.get(i-minSize);
            res = atoi(s->c_str());
            res = res * 60;
            if (minutes <= res) {
                position = i;
                break;
            }    
        }
}
    return position;

}

int getSchedulerMinutes(int position) {
    
    int res = 0;
    StringBuffer* s = NULL;
    if (position < minutesA.size()) {
        s = (StringBuffer*)minutesA.get(position);
        res = atoi(s->c_str());
    } else {
        s = (StringBuffer*)hoursA.get(position - minutesA.size());
        res = atoi(s->c_str());
        res = res * 60;
    }
    return res;
}

// CSyncSettings
IMPLEMENT_DYNCREATE(CSyncSettings, CFormView)

CSyncSettings::CSyncSettings()
	: CFormView(CSyncSettings::IDD)
{
}

CSyncSettings::~CSyncSettings()
{
}

void CSyncSettings::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SYNC_CHECK_CONTACTS, checkContacts);
    DDX_Control(pDX, IDC_SYNC_CHECK_CALENDAR, checkCalendar);
    DDX_Control(pDX, IDC_SYNC_CHECK_TASKS,    checkTasks);
    DDX_Control(pDX, IDC_SYNC_CHECK_NOTES,    checkNotes);
    DDX_Control(pDX, IDC_SYNC_CHECK_PICTURES, checkPictures);

    DDX_Control(pDX, IDC_SYNC_BUT_CONTACTS, butContacts);
    DDX_Control(pDX, IDC_SYNC_BUT_CALENDAR, butCalendar);
    DDX_Control(pDX, IDC_SYNC_BUT_TASKS,    butTasks);
    DDX_Control(pDX, IDC_SYNC_BUT_NOTES,    butNotes);
    DDX_Control(pDX, IDC_SYNC_BUT_PICTURES, butPictures);

    DDX_Control(pDX, IDC_SCHEDULER_CHECK_ENABLED, checkEnabled);
    DDX_Control(pDX, IDC_SCHEDULER_COMBO_VALUE,   comboSchedulerValue);
    DDX_Control(pDX, IDC_SYNC_CHECK_ENCRYPTION,   checkEncryption);
    DDX_Control(pDX, IDC_SYNC_GROUP_ITEMS,        groupItems);
    DDX_Control(pDX, IDC_SCHEDULER_GROUP,         groupScheduler);
    DDX_Control(pDX, IDC_SYNC_GROUP_SECURITY,     groupSecurity);
}

BEGIN_MESSAGE_MAP(CSyncSettings, CFormView)
    ON_MESSAGE( WM_INITDIALOG, OnInitForm ) 
    ON_BN_CLICKED(IDC_SYNC_CHECK_CONTACTS,  &CSyncSettings::OnBnClickedSyncCheckContacts)
    ON_BN_CLICKED(IDC_SYNC_CHECK_CALENDAR,  &CSyncSettings::OnBnClickedSyncCheckCalendar)
    ON_BN_CLICKED(IDC_SYNC_CHECK_TASKS,     &CSyncSettings::OnBnClickedSyncCheckTasks)
    ON_BN_CLICKED(IDC_SYNC_CHECK_NOTES,     &CSyncSettings::OnBnClickedSyncCheckNotes)
    ON_BN_CLICKED(IDC_SYNC_CHECK_PICTURES,  &CSyncSettings::OnBnClickedSyncCheckPictures)
    ON_BN_CLICKED(IDC_SYNC_OK,              &CSyncSettings::OnBnClickedSyncOk)
    ON_BN_CLICKED(IDC_SYNC_CANCEL,          &CSyncSettings::OnBnClickedSyncCancel)
    ON_BN_CLICKED(IDC_SYNC_BUT_CONTACTS,    &CSyncSettings::OnBnClickedSyncButContacts)
    ON_BN_CLICKED(IDC_SYNC_BUT_CALENDAR,    &CSyncSettings::OnBnClickedSyncButCalendar)
    ON_BN_CLICKED(IDC_SYNC_BUT_TASKS,       &CSyncSettings::OnBnClickedSyncButTasks)
    ON_BN_CLICKED(IDC_SYNC_BUT_NOTES,       &CSyncSettings::OnBnClickedSyncButNotes)
    ON_BN_CLICKED(IDC_SYNC_BUT_PICTURES,    &CSyncSettings::OnBnClickedSyncButPictures)
    ON_WM_NCPAINT()
    ON_BN_CLICKED(IDC_SCHEDULER_CHECK_ENABLED,  &CSyncSettings::OnBnClickedSchedulerCheckEnabled)
    ON_CBN_SELCHANGE(IDC_SCHEDULER_COMBO_VALUE, &CSyncSettings::OnCbnSelchangeSchedulerComboValue)
END_MESSAGE_MAP()


// CSyncSettings diagnostics

#ifdef _DEBUG
void CSyncSettings::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSyncSettings::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG



void CSyncSettings::disableSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2) {
    
    button1.EnableWindow(FALSE);
    button2.EnableWindow(FALSE);
    *synctype = false;
    GetDlgItem(sep1)->EnableWindow(FALSE);
    if (sep2 > 0) {
        GetDlgItem(sep2)->EnableWindow(FALSE);
    }

}
void CSyncSettings::hideSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2) {
    
        button1.ShowWindow(SW_HIDE);
        button2.ShowWindow(SW_HIDE);
        *synctype = false;
        GetDlgItem(sep1)->ShowWindow(SW_HIDE);
        if (sep2 > 0) {
            GetDlgItem(sep2)->ShowWindow(SW_HIDE);
        }
}


// CSyncSettings message handlers
LRESULT CSyncSettings::OnInitForm(WPARAM, LPARAM){
    CFormView::OnInitialUpdate();
    CString s1;
    int minutes = 0;
    
    s1.LoadString(IDS_ITEMS);       SetDlgItemText(IDC_SYNC_GROUP_ITEMS,    s1);
    s1.LoadString(IDS_SCHEDULER);   SetDlgItemText(IDC_SCHEDULER_GROUP,     s1);
    s1.LoadString(IDS_CONTACTS);    SetDlgItemText(IDC_SYNC_CHECK_CONTACTS, s1);
    s1.LoadString(IDS_CALENDAR);    SetDlgItemText(IDC_SYNC_CHECK_CALENDAR, s1);
    s1.LoadString(IDS_TASKS);       SetDlgItemText(IDC_SYNC_CHECK_TASKS,    s1);
    s1.LoadString(IDS_NOTES);       SetDlgItemText(IDC_SYNC_CHECK_NOTES,    s1);
    s1.LoadString(IDS_PICTURES);    SetDlgItemText(IDC_SYNC_CHECK_PICTURES, s1);
    
    s1.LoadString(IDS_DETAILS);
    SetDlgItemText(IDC_SYNC_BUT_CONTACTS, s1);
    SetDlgItemText(IDC_SYNC_BUT_CALENDAR, s1);
    SetDlgItemText(IDC_SYNC_BUT_TASKS,    s1);
    SetDlgItemText(IDC_SYNC_BUT_NOTES,    s1);
    SetDlgItemText(IDC_SYNC_BUT_PICTURES, s1);

    s1.LoadString(IDS_OK);     SetDlgItemText(IDC_SYNC_OK,     s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDC_SYNC_CANCEL, s1);

    // Scheduler: add strings to the comboBox
    if (initialized == false) {
        populateArrays();
        initialized = true;
    }
    s1.LoadString(IDS_SYNC_SYNCHRONIZE_EVERY); 
    SetDlgItemText(IDC_SCHEDULER_CHECK_ENABLED, s1);
    CString sched;
    s1.LoadString(IDS_MINUTES);
    
    StringBuffer val;
    for (int i = 0; i < minutesA.size(); i++) {
        val = ((StringBuffer*)(minutesA.get(i)))->c_str();
        sched = val.c_str();     sched += " ";     sched += s1;
        comboSchedulerValue.AddString(sched);
    }
    s1.LoadString(IDS_HOUR);
    for (int i = 0; i < hoursA.size(); i++) {
        val = ((StringBuffer*)(hoursA.get(i)))->c_str();
        if (val == "1") {
            s1.LoadString(IDS_HOUR);
        } else {
    s1.LoadString(IDS_HOURS);
        }
        sched = val.c_str();     sched += " ";     sched += s1;
        comboSchedulerValue.AddString(sched);
    }

    s1.LoadString(IDS_SECURITY);                 SetDlgItemText(IDC_SYNC_GROUP_SECURITY,  s1);
    s1.LoadString(IDS_SYNC_ENABLE_ENCRYPTION);   SetDlgItemText(IDC_SYNC_CHECK_ENCRYPTION, s1);


    //
    // enable/disable controls, depending of what sources are set to none
    //
    WindowsSyncSourceConfig* ssc = NULL;
    // CONTACTS
    if (isSourceVisible(CONTACT)) {
        saveSyncTypeContacts = true;
        ssc = getConfig()->getSyncSourceConfig(CONTACT_);
        if (!ssc->isEnabled()) {
            checkContacts.SetCheck(BST_UNCHECKED);
            butContacts.EnableWindow(FALSE);
        }
        else{
            checkContacts.SetCheck(BST_CHECKED);
        }
    }
    else {
        disableSource(checkContacts, butContacts, &saveSyncTypeContacts, IDC_SEPARATOR_1, 0);
        /*
        checkContacts.ShowWindow(SW_HIDE);
        butContacts.ShowWindow(SW_HIDE);
        saveSyncTypeContacts = false;
        GetDlgItem(IDC_SEPARATOR_1)->ShowWindow(SW_HIDE);
        */
    }

    // CALENDAR
    if (isSourceVisible(APPOINTMENT)) {
        saveSyncTypeCalendar = true;
        ssc = getConfig()->getSyncSourceConfig(APPOINTMENT_);
        if (!ssc->isEnabled()) {
            checkCalendar.SetCheck(BST_UNCHECKED);
            butCalendar.EnableWindow(FALSE);
        }
        else{
            checkCalendar.SetCheck(BST_CHECKED);
        }
    }
    else {
        disableSource(checkCalendar, butCalendar, &saveSyncTypeCalendar, IDC_SEPARATOR_1, IDC_SEPARATOR_2);
        /*
        checkCalendar.ShowWindow(SW_HIDE);
        butCalendar.ShowWindow(SW_HIDE);
        saveSyncTypeCalendar = false;
        GetDlgItem(IDC_SEPARATOR_1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SEPARATOR_2)->ShowWindow(SW_HIDE);
        */
    }

    // TASKS
    if (isSourceVisible(TASK)) {
        saveSyncTypeTasks = true;
        ssc = getConfig()->getSyncSourceConfig(TASK_);
        if (!ssc->isEnabled()) {
            checkTasks.SetCheck(BST_UNCHECKED);
            butTasks.EnableWindow(FALSE);
        }
        else{
            checkTasks.SetCheck(BST_CHECKED);
        }
    }
    else {
        disableSource(checkTasks, butTasks, &saveSyncTypeTasks, IDC_SEPARATOR_2, IDC_SEPARATOR_3);
        /*
        checkTasks.ShowWindow(SW_HIDE);
        butTasks.ShowWindow(SW_HIDE);
        saveSyncTypeTasks = false;
        GetDlgItem(IDC_SEPARATOR_2)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SEPARATOR_3)->ShowWindow(SW_HIDE);
        */
    }

    // NOTES
    if (isSourceVisible(NOTE)) {
        saveSyncTypeNotes = true;
        ssc = getConfig()->getSyncSourceConfig(NOTE_);
        if (!ssc->isEnabled()) {
            checkNotes.SetCheck(BST_UNCHECKED);
            butNotes.EnableWindow(FALSE);
        }
        else{
            checkNotes.SetCheck(BST_CHECKED);
        }
    }
    else {
        disableSource(checkNotes, butNotes, &saveSyncTypeNotes, IDC_SEPARATOR_3, IDC_SEPARATOR_4);
        /*
        checkNotes.ShowWindow(SW_HIDE);
        butNotes.ShowWindow(SW_HIDE);
        saveSyncTypeNotes = false;
        GetDlgItem(IDC_SEPARATOR_3)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SEPARATOR_4)->ShowWindow(SW_HIDE);
        */
    }

    // PICTURES
    if (isSourceVisible(PICTURE)) {
        saveSyncTypePictures = true;
        ssc = getConfig()->getSyncSourceConfig(PICTURE_);
        if (!ssc->isEnabled()) {
            checkPictures.SetCheck(BST_UNCHECKED);
            butPictures.EnableWindow(FALSE);
        }
        else{
            checkPictures.SetCheck(BST_CHECKED);
        }

        // Fix the source groupbox height (TODO: should be calculated dinamically)
        CRect sep3Rect, sep4Rect, sourceGroupBoxRect;
        GetDlgItem(IDC_SEPARATOR_3)->GetWindowRect(&sep3Rect);
        GetDlgItem(IDC_SEPARATOR_4)->GetWindowRect(&sep4Rect);
        int offset = sep4Rect.BottomRight().y - sep3Rect.BottomRight().y;
        
        CWnd* sourceGroupBox = GetDlgItem(IDC_SYNC_GROUP_ITEMS);
        GetDlgItem(IDC_SYNC_GROUP_ITEMS)->GetWindowRect(&sourceGroupBoxRect);
        sourceGroupBox->SetWindowPos(&CWnd::wndTop, 0, 0, 
                                     sourceGroupBoxRect.Width(), sourceGroupBoxRect.Height() + offset, 
                                     SWP_SHOWWINDOW | SWP_NOMOVE);
    }
    else {

        hideSource(checkPictures, butPictures, &saveSyncTypePictures, IDC_SEPARATOR_4, 0);
        /*
        checkPictures.ShowWindow(SW_HIDE);
        butPictures.ShowWindow(SW_HIDE);
        saveSyncTypePictures = false;
        GetDlgItem(IDC_SEPARATOR_4)->ShowWindow(SW_HIDE);
        */
    }

    
    // Load scheduler settings
    saveScheduler = false;
    if (minutesA.size() == 0 && hoursA.size() == 0) {
        checkEnabled.EnableWindow(FALSE);
        checkEnabled.ShowWindow(SW_HIDE);
        comboSchedulerValue.ShowWindow(SW_HIDE);
        groupScheduler.ShowWindow(SW_HIDE);

    } else {

    if(! getScheduler(&minutes)){
        checkEnabled.SetCheck(BST_UNCHECKED);
        comboSchedulerValue.EnableWindow(FALSE);
        int pos = defaultPosition; //getSchedulerPosition();
        comboSchedulerValue.SetCurSel(pos);
    }
    else{
        checkEnabled.SetCheck(BST_CHECKED);
        comboSchedulerValue.EnableWindow(TRUE);
        int pos = getSchedulerPosition(minutes);
        comboSchedulerValue.SetCurSel(pos);

        if (getSchedulerMinutes(pos) != minutes) {
            // Scheduler time was not exactly this one (manually modified?)
            saveScheduler = true;
        }
    }
    }

    // encryption is global
    if( (strcmp(getConfig()->getSyncSourceConfig(CONTACT_)->getEncryption(),"") != 0) ||
        (strcmp(getConfig()->getSyncSourceConfig(APPOINTMENT_)->getEncryption(),"") != 0) ||
        (strcmp(getConfig()->getSyncSourceConfig(TASK_)->getEncryption(),"") != 0) ||
        (strcmp(getConfig()->getSyncSourceConfig(NOTE_)->getEncryption(),"") != 0) )
        checkEncryption.SetCheck(BST_CHECKED);
    else
        checkEncryption.SetCheck(BST_UNCHECKED);

    //
    // Enable/disable encryption check
    //
    if (!ENABLE_ENCRYPTION_SETTINGS) {
        checkEncryption.SetCheck(BST_UNCHECKED);
        checkEncryption.EnableWindow(FALSE);
        checkEncryption.ShowWindow(FALSE);
        groupSecurity.ShowWindow(FALSE);
    }

    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupItems.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupScheduler.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupSecurity.m_hWnd,L" ",L" ");
    };

    return 0;
}

void CSyncSettings::OnBnClickedSyncCheckContacts()
{
    if(checkContacts.GetCheck() == BST_UNCHECKED){
        butContacts.EnableWindow(FALSE);
    }
    else{
        butContacts.EnableWindow(TRUE);
    }
    saveSyncTypeContacts = true;
}

void CSyncSettings::OnBnClickedSyncCheckCalendar()
{
    if(checkCalendar.GetCheck() == BST_UNCHECKED){
        butCalendar.EnableWindow(FALSE);
    }
    else{
        butCalendar.EnableWindow(TRUE);
    }
    saveSyncTypeCalendar = true;
}

void CSyncSettings::OnBnClickedSyncCheckTasks()
{
    if(checkTasks.GetCheck() == BST_UNCHECKED){
        butTasks.EnableWindow(FALSE);
    }
    else{
        butTasks.EnableWindow(TRUE);
    }
    saveSyncTypeTasks = true;
}

void CSyncSettings::OnBnClickedSyncCheckNotes()
{
    if(checkNotes.GetCheck() == BST_UNCHECKED)
        butNotes.EnableWindow(FALSE);
    else
        butNotes.EnableWindow(TRUE);

    saveSyncTypeNotes = true;
}

void CSyncSettings::OnBnClickedSyncCheckPictures()
{
    if(checkPictures.GetCheck() == BST_UNCHECKED)
        butPictures.EnableWindow(FALSE);
    else
        butPictures.EnableWindow(TRUE);

    saveSyncTypePictures = true;
}

void CSyncSettings::OnBnClickedSyncOk()
{
    // OK Button
    if(saveSettings(true)){
        ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
    }
}

void CSyncSettings::OnBnClickedSyncCancel()
{
    // CANCEL button
    getConfig()->read();
    ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
}

void CSyncSettings::OnBnClickedSyncButContacts()
{
    CContactSettings wndContacts;
    INT_PTR result = wndContacts.DoModal();
    saveSyncTypeContacts = true;
}

void CSyncSettings::OnBnClickedSyncButCalendar()
{
    CCalendarSettings wndCalendar;
    INT_PTR result = wndCalendar.DoModal();
    saveSyncTypeCalendar = true;
}

void CSyncSettings::OnBnClickedSyncButTasks()
{
    CTaskSettings wndTasks;
    INT_PTR result = wndTasks.DoModal();
    saveSyncTypeTasks = true;
}

void CSyncSettings::OnBnClickedSyncButNotes()
{
    CNotesSettings wndNotes;
    INT_PTR result = wndNotes.DoModal();
    saveSyncTypeNotes = true;
}

void CSyncSettings::OnBnClickedSyncButPictures()
{
    CPicturesSettings wndPictures;
    INT_PTR result = wndPictures.DoModal();
    saveSyncTypePictures = true;
}


bool CSyncSettings::saveSettings(bool saveToDisk)
{    
    CString s1, s2, msg;
    _bstr_t bst;
    int minutes=0, hours=0;

    // if scheduler values haven't changed, do not save it again
    if(saveScheduler){
        // check scheduler values
        if(checkEnabled.GetCheck() == BST_UNCHECKED){
            setScheduler(false, 0); 
            //return true; // return ok
        }
        else{
            // scheduler enabled
            int pos = comboSchedulerValue.GetCurSel();
            minutes = getSchedulerMinutes(pos);

            // save scheduler settings
            if(setScheduler(true, minutes) != 0){
                msg.LoadString(IDS_SCHEDULER_CANNOT_SCHEDULE);
                wsafeMessageBox(msg);
            }
        }
    }
    saveScheduler = false;

    if (saveSyncTypeContacts) {
        bool enabled = (checkContacts.GetCheck() == BST_CHECKED);
        getConfig()->getSyncSourceConfig(CONTACT_)->setIsEnabled(enabled);
    }
    if (saveSyncTypeCalendar) {
        bool enabled = (checkCalendar.GetCheck() == BST_CHECKED);
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(enabled);
    }
    if (saveSyncTypeTasks) {
        bool enabled = (checkTasks.GetCheck() == BST_CHECKED);
        getConfig()->getSyncSourceConfig(TASK_)->setIsEnabled(enabled);
    }
    if (saveSyncTypeNotes) {
        bool enabled = (checkNotes.GetCheck() == BST_CHECKED);
        getConfig()->getSyncSourceConfig(NOTE_)->setIsEnabled(enabled);
    }
    if (saveSyncTypePictures) {
        bool enabled = (checkPictures.GetCheck() == BST_CHECKED);
        getConfig()->getSyncSourceConfig(PICTURE_)->setIsEnabled(enabled);
    }

    // save encryption, global property 
    // NOTE: pictures excluded: cannot DES a largeObject read chunk by chunk via input stream
    if(checkEncryption.GetCheck()){
        getConfig()->getSyncSourceConfig(CONTACT_)->setEncryption("des");
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setEncryption("des");
        getConfig()->getSyncSourceConfig(TASK_)->setEncryption("des");
        getConfig()->getSyncSourceConfig(NOTE_)->setEncryption("des");

        // When encryption is used, encoding is always 'base64'.
        getConfig()->getSyncSourceConfig(CONTACT_)->setEncoding("b64");
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setEncoding("b64");
        getConfig()->getSyncSourceConfig(TASK_)->setEncoding("b64");
        getConfig()->getSyncSourceConfig(NOTE_)->setEncoding("b64");
    }
    else{
        getConfig()->getSyncSourceConfig(CONTACT_)->setEncryption("");
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setEncryption("");
        getConfig()->getSyncSourceConfig(TASK_)->setEncryption("");
        getConfig()->getSyncSourceConfig(NOTE_)->setEncryption("");

        // Ensure that encoding is the correct one ("b64" only for SIF).
        if ( !strcmp(getConfig()->getSyncSourceConfig(CONTACT_)->getType(), "text/x-vcard") ) {
            getConfig()->getSyncSourceConfig(CONTACT_)->setEncoding("bin");
        }
        if ( !strcmp(getConfig()->getSyncSourceConfig(APPOINTMENT_)->getType(), "text/x-vcalendar") ) {
            getConfig()->getSyncSourceConfig(APPOINTMENT_)->setEncoding("bin");
        }
    }

    if(saveToDisk)
        getConfig()->save();

    return true;
}

void CSyncSettings::OnNcPaint(){
    CFormView::OnNcPaint();
    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));   
}

void CSyncSettings::OnBnClickedSchedulerCheckEnabled()
{
    if(checkEnabled.GetCheck() == BST_UNCHECKED){
        comboSchedulerValue.EnableWindow(FALSE);
    }
    else{
        comboSchedulerValue.EnableWindow(TRUE);
        int pos = defaultPosition;
        comboSchedulerValue.SetCurSel(pos);
    }

    saveScheduler = true;
}



BOOL CSyncSettings::PreTranslateMessage(MSG* pMsg){
    bool bProcessed =false;
    if(pMsg->message == WM_KEYDOWN){
        if(pMsg->wParam == VK_ESCAPE){
            OnBnClickedSyncCancel();
            bProcessed = true;
        }
    };

    if(bProcessed)
        return TRUE;
    else
        return CFormView::PreTranslateMessage(pMsg);
}


void CSyncSettings::OnCbnSelchangeSchedulerComboValue()
{
    saveScheduler = true;
}
