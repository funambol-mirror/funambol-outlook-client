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
#include "OutlookPluginDoc.h"

/**
 * Account Settings panel.
 */
class CAccountSettings : public CFormView
{
	DECLARE_DYNCREATE(CAccountSettings)
protected:
    CAccountSettings();           // protected constructor used by dynamic creation
    virtual ~CAccountSettings();
public:
    enum { IDD = IDD_ACCOUNT };
    COutlookPluginDoc* GetDocument();
#ifdef _DEBUG
    virtual void AssertValid() const;
#ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support  
   virtual BOOL PreTranslateMessage(MSG* pMsg);
   DECLARE_MESSAGE_MAP()
public:   
    CEdit editUrl;
    CEdit editUser;
    CEdit editPassword;
    CButton butProxy;
    CStatic groupServer;

    bool saveSettings(bool saveToDisk);   
    void resetMediaSourcesParameters();

    afx_msg void OnBnClickedAccountButCancel();
    afx_msg void OnBnClickedAccountButOk();
    afx_msg LRESULT OnInitForm(WPARAM, LPARAM);
    afx_msg int OnCreate(LPCREATESTRUCT lpcs);
    afx_msg void OnNcPaint();
    afx_msg void OnBnClickedAccountButProxy();
};

/** @} */
/** @endcond */
