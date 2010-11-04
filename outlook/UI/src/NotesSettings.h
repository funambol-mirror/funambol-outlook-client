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

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */

#pragma once
#include "afxwin.h"

#define SIF_CHECKED         0
#define VNOTE_CHECKED       1


/**
 * Notes options window.
 */
class CNotesSettings : public CDialog
{
	DECLARE_DYNCREATE(CNotesSettings)

private:
    int currentRadioChecked;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    CComboBox lstSyncType;
    CEdit editFolder;
    CButton checkInclude;
    CButton butSelectFolder;
    CEdit editRemote;
    CStatic groupDirection;
    CStatic groupFolder;
    CStatic groupAdvanced;
    CButton radioSif;
    CButton radioVNote;

    CButton checkShared;

    /**
     * Loads the string data into the syncmode editbox/dropdown box.
     * If only 1 syncmode is available, the editbox is used.
     * Otherwise the dropdown box is used.
     */
    void loadSyncModesBox(const char* sourceName);

public:

	CNotesSettings();           
	virtual ~CNotesSettings();

	enum { IDD = IDD_NOTES };

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

    bool saveSettings(bool);
    afx_msg void OnBnClickedNotesOk();
    afx_msg void OnBnClickedNotesCancel();
    afx_msg void OnBnClickedNotesButSelect();
    afx_msg void OnBnClickedNotesRadioVNote();
    afx_msg void OnBnClickedNotesRadioSif();
    afx_msg void OnBnClickedNotesCheckShared();
};

/** @} */
/** @endcond */

