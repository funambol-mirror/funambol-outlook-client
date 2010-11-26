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

#pragma once
#include "afxwin.h"

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */


/**
 * Returns the scheduler # of minutes, given the droplist position.
 * If minutes is different from the exact values, we go to the upper value available.
 * Default value is 15 minutes.
 */
int getSchedulerMinutes(int position);

/**
 * Returns the scheduler droplist position, given the # of minutes.
 * Position is 0 to 11 (default = 2);
 */
int getSchedulerPosition(int minutes);



/**
 * Sync options panel.
 */
class CSyncSettings : public CFormView
{
	DECLARE_DYNCREATE(CSyncSettings)

protected:
	CSyncSettings();           // protected constructor used by dynamic creation
	virtual ~CSyncSettings();

public:
	enum { IDD = IDD_SYNC };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
    virtual void PostNcDestroy( ){delete this;}
#endif
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg LRESULT OnInitForm(WPARAM, LPARAM);
    
    /// hide a source given its components
    void hideSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2);
    
    /// disable a source (it remaining visible by greyed) given its components
    void disableSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2);
 
    /**
     * Creates and returns a string to be displayed in the source checkbox
     * like "Contacts", or "Pictures (Download only)".
     * Checks the config to know the current syncmode set.
     */
    CString composeCheckboxText(const char* sourceName);

	DECLARE_MESSAGE_MAP()

public:
    
    CButton checkContacts;
    CButton checkCalendar;
    CButton checkTasks;
    CButton checkNotes;
    CButton checkPictures;
    CButton checkFiles;

    CButton butContacts;
    CButton butCalendar;
    CButton butTasks;
    CButton butNotes;
    CButton butPictures;
    CButton butFiles;

    bool saveSyncTypeContacts;
    bool saveSyncTypeCalendar;
    bool saveSyncTypeTasks;
    bool saveSyncTypeNotes;
    bool saveSyncTypePictures;
    bool saveSyncTypeFiles;
    bool saveScheduler; // true if scheduler settings have changed
    bool saveAttach; // true if the checkbox to require outlook open is changed

    // scheduler
    CButton checkEnabled;
    CButton checkEncryption;
    CButton checkAttach;
    CStatic groupItems;
    CStatic groupScheduler;
    CStatic groupSecurity;
    CComboBox comboSchedulerValue;
    
    bool saveSettings(bool);

    afx_msg void OnBnClickedSyncCheckContacts();
    afx_msg void OnBnClickedSyncCheckCalendar();
    afx_msg void OnBnClickedSyncCheckTasks();
    afx_msg void OnBnClickedSyncCheckNotes();
    afx_msg void OnBnClickedSyncCheckPictures();
    afx_msg void OnBnClickedSyncCheckFiles();
    afx_msg void OnBnClickedSyncOk();
    afx_msg void OnBnClickedSyncCancel();
    afx_msg void OnBnClickedSyncButContacts();
    afx_msg void OnBnClickedSyncButCalendar();
    afx_msg void OnBnClickedSyncButTasks();
    afx_msg void OnBnClickedSyncButNotes();
    afx_msg void OnBnClickedSyncButPictures();
    afx_msg void OnBnClickedSyncButFiles();
    afx_msg void OnNcPaint();   
  
    afx_msg void OnBnClickedSchedulerCheckEnabled();
    afx_msg void OnCbnSelchangeSchedulerComboValue();
    afx_msg void OnBnClickedSyncCheckOutlookOpen();
};


/** @} */
/** @endcond */
