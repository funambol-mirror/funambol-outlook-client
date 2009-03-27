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

// ProxySettings.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "ProxySettings.h"
#include "ClientUtil.h"
#include "winmaincpp.h"


// CProxySettings dialog

IMPLEMENT_DYNAMIC(CProxySettings, CDialog)

CProxySettings::CProxySettings(CWnd* pParent /*=NULL*/)
	: CDialog(CProxySettings::IDD, pParent)
{

}

CProxySettings::~CProxySettings()
{
}

void CProxySettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROXY_ENABLE, checkEnable);
    DDX_Control(pDX, IDC_PROXY_AUTH, checkAuth);
    DDX_Control(pDX, IDC_PROXY_EDIT_USER, editUser);
    DDX_Control(pDX, IDC_PROXY_EDIT_PASS, editPass);
}


BEGIN_MESSAGE_MAP(CProxySettings, CDialog)
    ON_BN_CLICKED(IDC_PROXY_ENABLE, &CProxySettings::OnBnClickedProxyEnable)
    ON_BN_CLICKED(IDC_PROXY_AUTH, &CProxySettings::OnBnClickedProxyAuth)
END_MESSAGE_MAP()


// CProxySettings message handlers

BOOL CProxySettings::OnInitDialog(){
    CString s1, s2;
    s1.LoadString(IDS_PROXY_SETTINGS2); SetWindowText(s1);
    CDialog::OnInitDialog();

    const char* proxyUser = NULL;
    const char* proxyPass = NULL;
    

    editUser.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editPass.SetLimitText(EDIT_TEXT_MAXLENGTH);

    s1.LoadString(IDS_ACCOUNT_USEPROXY);  SetDlgItemText(IDC_PROXY_ENABLE, s1);
    s1.LoadString(IDS_ACCOUNT_PROXY_AUTH); SetDlgItemText(IDC_PROXY_AUTH, s1);
    s1.LoadString(IDS_ACCOUNT_PROXY_USER); SetDlgItemText(IDC_PROXY_STATIC_USER, s1);
    s1.LoadString(IDS_ACCOUNT_PROXY_PASS); SetDlgItemText(IDC_PROXY_STATIC_PASSWORD, s1);

    // read config from Registry 
    OutlookConfig* conf = (OutlookConfig*)getConfig();

    proxyUser = conf->getAccessConfig().getProxyUsername();
    proxyPass = conf->getAccessConfig().getProxyPassword();

    // proxy authentication
    if( (strcmp(proxyUser,"")) && (strcmp(proxyPass,"")) ){
        // proxy authentication required
        checkAuth.SetCheck(BST_CHECKED);
        editUser.EnableWindow(TRUE);
        editPass.EnableWindow(TRUE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(TRUE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(TRUE);

        // Note: use 'toWideChar' because we need UTF-8 conversion.
        WCHAR* tmp = toWideChar(proxyUser);
        s2 = tmp;
        SetDlgItemText(IDC_PROXY_EDIT_USER, s2);
        delete [] tmp;

        // Note: use 'toWideChar' because we need UTF-8 conversion.
        tmp = toWideChar(proxyUser);
        s2 = tmp; 
        SetDlgItemText(IDC_PROXY_EDIT_PASS, s2);
        delete [] tmp;
    }
    else{
        checkAuth.SetCheck(BST_UNCHECKED);
        editUser.EnableWindow(FALSE);
        editPass.EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(FALSE);
    }

    // use proxy
    if(conf->getAccessConfig().getUseProxy()){
        checkEnable.SetCheck(BST_CHECKED);
    }
    else{
        checkEnable.SetCheck(BST_UNCHECKED);
        checkAuth.EnableWindow(FALSE);
        editUser.EnableWindow(FALSE);
        editPass.EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(FALSE);
    }

    checkEnable.SetFocus();
    return FALSE;
}

void CProxySettings::OnBnClickedProxyEnable()
{
    if(checkEnable.GetCheck() == BST_CHECKED){
        checkAuth.EnableWindow(TRUE);
        if(checkAuth.GetCheck() == BST_CHECKED){
            editUser.EnableWindow(TRUE);
            editPass.EnableWindow(TRUE);
            GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(TRUE);
            GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(TRUE);   
        }
    }
    else{
        checkAuth.EnableWindow(FALSE);
        editUser.EnableWindow(FALSE);
        editPass.EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(FALSE);
    }
}

void CProxySettings::OnBnClickedProxyAuth()
{
    if(checkAuth.GetCheck() == BST_CHECKED){
        editUser.EnableWindow(TRUE);
        editPass.EnableWindow(TRUE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(TRUE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(TRUE);
    }
    else{
        editUser.EnableWindow(FALSE);
        editPass.EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_USER)->EnableWindow(FALSE);
        GetDlgItem(IDC_PROXY_STATIC_PASSWORD)->EnableWindow(FALSE);
    }
}


void CProxySettings::OnOK(){
    CString proxyUser, proxyPassword;
    CString s1;
    _bstr_t bst;
    bool cancelSave = false;
    OutlookConfig* conf = (OutlookConfig*)getConfig();

    GetDlgItemText(IDC_PROXY_EDIT_USER,proxyUser);
    GetDlgItemText(IDC_PROXY_EDIT_PASS,proxyPassword);

    if( (checkEnable.GetCheck() == BST_CHECKED) && (checkAuth.GetCheck() == BST_CHECKED)){
        if (proxyUser == ""){
            s1.LoadString(IDS_ERROR_PROXY_USERNAME);
            MessageBox(s1);
            cancelSave = true;
            goto finally;
        };
        if (proxyPassword == ""){
            s1.LoadString(IDS_ERROR_PROXY_PASSWORD);
            MessageBox(s1);
            cancelSave = true;
            goto finally;
        };
    }

    if(checkEnable.GetCheck() == BST_CHECKED)
        conf->getAccessConfig().setUseProxy(TRUE);
    else
        conf->getAccessConfig().setUseProxy(FALSE);

    if(checkAuth.GetCheck() == BST_CHECKED){
        // Note: use 'toMultibyte' which uses charset UTF-8.
        char* tmp = toMultibyte(proxyUser.GetBuffer());
        if (tmp) {
            conf->getAccessConfig().setProxyUsername(tmp);
            delete [] tmp;
        }
        // Note: use 'toMultibyte' which uses charset UTF-8.
        tmp = toMultibyte(proxyPassword.GetBuffer());
        if (tmp) {
            conf->getAccessConfig().setProxyPassword(tmp);
            delete [] tmp;
        }
    }
    else{
        conf->getAccessConfig().setProxyUsername("");
        conf->getAccessConfig().setProxyPassword("");
    }

    // save values
  finally:
    if(!cancelSave){
        conf->save();

        CDialog::OnOK();
    }
    
}