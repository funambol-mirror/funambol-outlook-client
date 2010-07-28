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

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */


// CFullSync dialog
#include "OutlookPlugin.h"
#include "afxwin.h"


/**
 * Recover sync window.
 */
class CFullSync : public CDialog
{
	DECLARE_DYNAMIC(CFullSync)

public:
	CFullSync(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFullSync();
    virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_FULLSYNC };

private:

    /**
     * Resize/move dynamically the source checkboxes, based on the number of
     * sources visible.
     */
    void adjustCheckboxes();

    /// Returns true if at least one source checkbox is checked.
    bool isAtLeastOneSourceChecked();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox lstSyncType;
    CButton checkContacts;
    CButton checkCalendar;
    CButton checkTasks;
    CButton checkNotes;
    CButton checkPictures;
    CButton checkFiles;
    CButton radio1;
    CButton radio2;
    CButton radio3;
    CStatic groupDirection;
    CStatic groupItems;
    
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

    /// Enable/disable the 'Recover' button, checking if at least one source is selected.
    afx_msg void OnBnClickedSourceCheckBox();

    /// Disables and unchecks the 'picture' source checkbox.
    afx_msg void OnBnClickedRefreshC2S();

    /// Enables the 'picture' source checkbox.
    afx_msg void OnBnClickedRefreshS2C();
};

/** @} */
/** @endcond */
