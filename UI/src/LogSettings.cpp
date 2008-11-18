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

// LogSettings.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "LogSettings.h"
#include "MainSyncFrm.h"

#include "winmaincpp.h"

// CLogSettings dialog

IMPLEMENT_DYNAMIC(CLogSettings, CDialog)

CLogSettings::CLogSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CLogSettings::IDD, pParent)
{

}

CLogSettings::~CLogSettings()
{
}

void CLogSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOG_RADIO_NONE, radioNone);
    DDX_Control(pDX, IDC_LOG_RADIO_INFO, radioInfo);
    DDX_Control(pDX, IDC_LOG_RADIO_DEBUG, radioDebug);
    DDX_Control(pDX, IDC_LOG_GROUP_LOGLEVEL, groupLevel);
}


BEGIN_MESSAGE_MAP(CLogSettings, CDialog)
    ON_BN_CLICKED(IDC_LOG_OK, &CLogSettings::OnBnClickedLogOk)
    ON_BN_CLICKED(IDC_LOG_CANCEL, &CLogSettings::OnBnClickedLogCancel)
    ON_BN_CLICKED(IDC_LOG_RADIO_NONE, &CLogSettings::OnBnClickedLogRadioNone)
    ON_BN_CLICKED(IDC_LOG_RADIO_INFO, &CLogSettings::OnBnClickedLogRadioInfo)
    ON_BN_CLICKED(IDC_LOG_RADIO_DEBUG, &CLogSettings::OnBnClickedLogRadioDebug)
    ON_BN_CLICKED(IDC_LOG_VIEWLOG, &CLogSettings::OnBnClickedLogViewlog)
END_MESSAGE_MAP()


// CLogSettings message handlers
void CLogSettings::OnBnClickedLogOk()
{
    // OK Button
    if(saveSettings(true))
        CDialog::OnOK();
}

void CLogSettings::OnBnClickedLogCancel()
{
    // CANCEL button
    getConfig()->read();
    CDialog::OnOK();
}

BOOL CLogSettings::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_LOGGING); SetWindowText(s1);
    CDialog::OnInitDialog();


    // load string resources
    s1.LoadString(IDS_LOG_LEVEL); SetDlgItemText(IDC_LOG_GROUP_LOGLEVEL, s1);
    s1.LoadString(IDS_LOG_NONE); SetDlgItemText(IDC_LOG_RADIO_NONE, s1);
    s1.LoadString(IDS_LOG_INFO); SetDlgItemText(IDC_LOG_RADIO_INFO, s1);
    s1.LoadString(IDS_LOG_DEBUG); SetDlgItemText(IDC_LOG_RADIO_DEBUG, s1);

    s1.LoadString(IDS_OK); SetDlgItemText(IDC_LOG_OK, s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDC_LOG_CANCEL, s1);
    s1.LoadString(IDS_APPLY); SetDlgItemText(IDC_LOG_APPLY, s1);

    // read config from Registry 
    OutlookConfig* conf = (OutlookConfig*)getConfig();
    if(conf->getDeviceConfig().getLogLevel() == LOG_LEVEL_NONE)
        radioNone.SetCheck(BST_CHECKED);
    if(conf->getDeviceConfig().getLogLevel() == LOG_LEVEL_INFO)
        radioInfo.SetCheck(BST_CHECKED);
    if(conf->getDeviceConfig().getLogLevel() == LOG_LEVEL_DEBUG)
        radioDebug.SetCheck(BST_CHECKED);

    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupLevel.m_hWnd,L" ",L" ");
    };

    return TRUE;
}



bool CLogSettings::saveSettings(bool saveToDisk)
{
    OutlookConfig* conf = (OutlookConfig*)getConfig();
    if(radioNone.GetCheck() == BST_CHECKED)
        conf->getDeviceConfig().setLogLevel(LOG_LEVEL_NONE);
    if(radioInfo.GetCheck() == BST_CHECKED)
        conf->getDeviceConfig().setLogLevel(LOG_LEVEL_INFO);
    if(radioDebug.GetCheck() == BST_CHECKED)
        conf->getDeviceConfig().setLogLevel(LOG_LEVEL_DEBUG);

    if(saveToDisk)
        conf->save();
    return true;
}


void CLogSettings::OnBnClickedLogRadioNone()
{
    radioNone.SetCheck(BST_CHECKED);
}

void CLogSettings::OnBnClickedLogRadioInfo()
{
    radioInfo.SetCheck(BST_CHECKED);
}

void CLogSettings::OnBnClickedLogRadioDebug()
{
    radioDebug.SetCheck(BST_CHECKED);
}

void CLogSettings::OnBnClickedLogViewlog()
{
    // open log file
    SHELLEXECUTEINFO lpExecInfo;
    memset(&lpExecInfo, 0, sizeof(SHELLEXECUTEINFO));
    lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    // *** TODO: use LOG.getPath() ***
    string logFile = ((OutlookConfig*)getConfig())->getLogDir();
    logFile += "\\";
    logFile += OL_PLUGIN_LOG_NAME;
    WCHAR* wlogFile = toWideChar(logFile.c_str());

    lpExecInfo.lpFile = wlogFile;
    lpExecInfo.nShow = SW_SHOWNORMAL;
    lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    lpExecInfo.lpVerb = _T("open");
    ShellExecuteEx(&lpExecInfo);
}