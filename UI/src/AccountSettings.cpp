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
#include "AccountSettings.h"
#include "MainSyncFrm.h"
#include "SyncForm.h"
#include "ProxySettings.h"

#include "winmaincpp.h"
#include "OutlookConfig.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"


IMPLEMENT_DYNCREATE(CAccountSettings, CFormView)

CAccountSettings::CAccountSettings()
	: CFormView(CAccountSettings::IDD)
{
}

CAccountSettings::~CAccountSettings()
{
}

void CAccountSettings::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ACCOUNT_EDIT_URL, editUrl);
    DDX_Control(pDX, IDC_ACCOUNT_EDIT_USERNAME, editUser);
    DDX_Control(pDX, IDC_ACCOUNT_EDIT_PASSWORD, editPassword);
    DDX_Control(pDX, IDC_ACCOUNT_BUT_PROXY, butProxy);
    DDX_Control(pDX, IDC_ACCOUNT_GROUP_SERVER, groupServer);
}

BEGIN_MESSAGE_MAP(CAccountSettings, CFormView)
    ON_WM_CREATE()
    ON_BN_CLICKED(IDC_ACCOUNT_BUTOK, &CAccountSettings::OnBnClickedAccountButOk)
    ON_MESSAGE( WM_INITDIALOG, OnInitForm ) 
    ON_BN_CLICKED(IDC_ACCOUNT_BUT_CANCEL, &CAccountSettings::OnBnClickedAccountButCancel)
    ON_WM_NCPAINT()
    ON_BN_CLICKED(IDC_ACCOUNT_BUT_PROXY, &CAccountSettings::OnBnClickedAccountButProxy)
END_MESSAGE_MAP()


#ifdef _DEBUG
void CAccountSettings::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CAccountSettings::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CAccountSettings message handlers
int CAccountSettings::OnCreate(LPCREATESTRUCT lpcs){
    CFormView::OnCreate(lpcs);
    return 0;
}

LRESULT CAccountSettings::OnInitForm(WPARAM, LPARAM){
    CFormView::OnInitialUpdate(); //!! 
        
    const char* proxyUser = NULL;
    const char* proxyPass = NULL;
    CString s1, s2;

    // text limited to EDIT_TEXT_MAXLENGTH chars
    editUrl.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editUser.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editPassword.SetLimitText(EDIT_TEXT_MAXLENGTH);
    
    // load string resources
    s1.LoadString(IDS_ACCOUNT_URL);     SetDlgItemText(IDC_ACCOUNT_STATIC_URL, s1);
    s1.LoadString(IDS_ACCOUNT_USERNAME); SetDlgItemText(IDC_ACCOUNT_STATIC_USERNAME, s1);
    s1.LoadString(IDS_ACCOUNT_PASSWORD); SetDlgItemText(IDC_ACCOUNT_STATIC_PASSWORD, s1);
    s1.LoadString(IDS_SERVER); SetDlgItemText(IDC_ACCOUNT_GROUP_SERVER, s1);
    s1.LoadString(IDS_PROXY_SETTINGS); SetDlgItemText(IDC_ACCOUNT_BUT_PROXY, s1);
    
    s1.LoadString(IDS_OK); SetDlgItemText(IDC_ACCOUNT_BUTOK, s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDC_ACCOUNT_BUT_CANCEL, s1);

    // read config from Registry 
    OutlookConfig* conf = (OutlookConfig*)getConfig();

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* tmp = toWideChar(conf->getAccessConfig().getSyncURL());
    s2 = tmp; 
    delete [] tmp;
    SetDlgItemText(IDC_ACCOUNT_EDIT_URL, s2);

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    tmp = toWideChar(conf->getAccessConfig().getUsername());
    s2 = tmp; 
    delete [] tmp;
    SetDlgItemText(IDC_ACCOUNT_EDIT_USERNAME, s2);

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    tmp = toWideChar(conf->getAccessConfig().getPassword());
    s2 = tmp; 
    delete [] tmp;
    SetDlgItemText(IDC_ACCOUNT_EDIT_PASSWORD, s2);


    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupServer.m_hWnd,L" ",L" ");
    }
    
    return 0;
};

void CAccountSettings::OnBnClickedAccountButCancel()
{
    getConfig()->read();  
    ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
}


void CAccountSettings::OnBnClickedAccountButOk()
{
    if(saveSettings(true)){
        //  close settings, show main window
        ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
    }
}


bool CAccountSettings::saveSettings(bool saveToDisk)
{
    // check values
    CString url, username, password, proxyUser, proxyPassword;
    CString s1;
    _bstr_t bst;
    OutlookConfig* conf = (OutlookConfig*)getConfig();

    GetDlgItemText(IDC_ACCOUNT_EDIT_URL,url); 
    GetDlgItemText(IDC_ACCOUNT_EDIT_USERNAME,username);
    GetDlgItemText(IDC_ACCOUNT_EDIT_PASSWORD,password);

    if(url == ""){
        s1.LoadString(IDS_ERROR_SET_URL);
        wsafeMessageBox(s1);
        return false;
    };
    if(username == ""){
        s1.LoadString(IDS_ERROR_SET_USERNAME);
        wsafeMessageBox(s1);
        return false;
    };
    if(password == ""){
        s1.LoadString(IDS_ERROR_SET_PASSWORD);
        wsafeMessageBox(s1);
        return false;
    };


    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* tmp = toMultibyte(url.GetBuffer());
    if (tmp) {
        conf->getAccessConfig().setSyncURL(tmp);
        delete [] tmp;
    }
    tmp = toMultibyte(username.GetBuffer());
    if (tmp) {
        conf->getAccessConfig().setUsername(tmp);
        delete [] tmp;
    }
    tmp = toMultibyte(password.GetBuffer());
    if (tmp) {
        conf->getAccessConfig().setPassword(tmp);
        delete [] tmp;
    }
    
    // save values to registry 
    if(saveToDisk)
        conf->save();

    return true;
}

void CAccountSettings::OnNcPaint(){
    CFormView::OnNcPaint();
    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));   
}


void CAccountSettings::OnBnClickedAccountButProxy()
{
    CProxySettings wndProxySettings;
    INT_PTR result = wndProxySettings.DoModal();  
}

BOOL CAccountSettings::PreTranslateMessage(MSG* pMsg){
    bool bProcessed =false;
    if(pMsg->message == WM_KEYDOWN){
        if(pMsg->wParam == VK_ESCAPE){
            OnBnClickedAccountButCancel();
            bProcessed = true;
        }
    };

    if(bProcessed)
        return TRUE;
    else
        return CFormView::PreTranslateMessage(pMsg);
}