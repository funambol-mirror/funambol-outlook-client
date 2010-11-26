/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2010 Funambol, Inc.
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
#include "winmaincpp.h"
#include "afxwin.h"
#include <string>


/**
 * Files options window.
 */
class CFilesSettings : public CDialog {

	DECLARE_DYNCREATE(CFilesSettings)

private:

    /// The SyncSource configuration for files
    WindowsSyncSourceConfig* ssconf;

    /**
     * Opens the default Windows dialog to select a folder in the file system.
     * @param folderpath    [IN-OUT] the user selected folder path, untouched if the user cancelled
     * @param defaultFolder [OPTIONAL] the default folder to start browsing
     * @param szCaption     [OPTIONAL] the caption of the dialog to display
     * @param hOwner        [OPTIONAL] handle to the parent window. Set it in order to make the dialog modal
     * @return              true if successful, false if cancelled or an error occurs
     */
    bool browseFolder(std::wstring& folderpath, 
                      const WCHAR* defaultFolder = NULL, 
                      const WCHAR* szCaption = NULL, 
                      const HWND hOwner = NULL);

    /**
     * Loads the string data into the syncmode editbox/dropdown box.
     * If only 1 syncmode is available, the editbox is used.
     * Otherwise the dropdown box is used.
     */
    void loadSyncModesBox(const char* sourceName);

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    CEdit     editSyncType;
    CEdit     editFolder;
    CButton   checkInclude;
    CButton   butSelectFolder;
    CEdit     editRemote;
    CStatic   groupDirection;
    CStatic   groupFolder;
    CStatic   groupAdvanced;

public:

	CFilesSettings();           
	virtual ~CFilesSettings();

	enum { IDD = IDD_FILES };

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

    bool saveSettings(bool);
    afx_msg void OnBnClickedFilesOk();
    afx_msg void OnBnClickedFilesCancel();
    afx_msg void OnBnClickedFilesButSelect();
};

/** @} */
/** @endcond */

