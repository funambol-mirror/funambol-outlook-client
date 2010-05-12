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
#include "OutlookLogAccessor.h"
#include "client/SendLogManager.h"
#include "client/PostDataLogSender.h"
#include "http/WinTransportAgent.h"
#include "http/DigestAuthentication.h"
#include "UICustomization.h"
#include "SettingsHelper.h"

#include "winmaincpp.h"

// CLogSettings dialog

USE_FUNAMBOL_NAMESPACE

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

    DDX_Control(pDX, IDC_LOG_EDIT_NUM_LOGS, logFileNum);
    DDX_Control(pDX, IDC_LOG_EDIT_NUM_LOGS_SPIN, logFileNumSpin);

    DDX_Control(pDX, IDC_LOG_EDIT_LOG_SIZE, logFileSize);
    DDX_Control(pDX, IDC_LOG_EDIT_LOG_SIZE_SPIN, logFileSizeSpin);
}


BEGIN_MESSAGE_MAP(CLogSettings, CDialog)
    ON_BN_CLICKED(IDC_LOG_OK, &CLogSettings::OnBnClickedLogOk)
    ON_BN_CLICKED(IDC_LOG_CANCEL, &CLogSettings::OnBnClickedLogCancel)
    ON_BN_CLICKED(IDC_LOG_RADIO_NONE, &CLogSettings::OnBnClickedLogRadioNone)
    ON_BN_CLICKED(IDC_LOG_RADIO_INFO, &CLogSettings::OnBnClickedLogRadioInfo)
    ON_BN_CLICKED(IDC_LOG_RADIO_DEBUG, &CLogSettings::OnBnClickedLogRadioDebug)
    ON_BN_CLICKED(IDC_LOG_VIEWLOG, &CLogSettings::OnBnClickedLogViewlog)
    ON_BN_CLICKED(IDC_LOG_VIEWPATH, &CLogSettings::OnBnClickedLogViewpath)
    ON_BN_CLICKED(IDC_LOG_SEND, &CLogSettings::OnBnClickedLogSend)
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

    s1.LoadString(IDS_LOG_DETAILS); SetDlgItemText(IDC_LOG_GROUP_LOGDETAILS, s1);
    s1.LoadString(IDS_LOG_VIEWLOG); SetDlgItemText(IDC_LOG_VIEWLOG, s1);
    s1.LoadString(IDS_LOG_SEND); SetDlgItemText(IDC_LOG_SEND, s1);
    s1.LoadString(IDS_LOG_EDIT_LOG_SIZE); SetDlgItemText(IDC_LOG_STATIC_SIZE, s1);
    s1.LoadString(IDS_LOG_EDIT_NUM_LOGS); SetDlgItemText(IDC_LOG_STATIC_NUM, s1);

    s1.LoadString(IDS_OK); SetDlgItemText(IDC_LOG_OK, s1);
    s1.LoadString(IDS_CANCEL); SetDlgItemText(IDC_LOG_CANCEL, s1);
    s1.LoadString(IDS_APPLY); SetDlgItemText(IDC_LOG_APPLY, s1);

    if (!UICustomization::logRotateOptions) {
        GetDlgItem(IDC_LOG_STATIC_SIZE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LOG_STATIC_NUM)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LOG_EDIT_LOG_SIZE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LOG_EDIT_NUM_LOGS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LOG_EDIT_NUM_LOGS_SPIN)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_LOG_EDIT_LOG_SIZE_SPIN)->ShowWindow(SW_HIDE);

        int dy = -50;
        moveItem(this, GetDlgItem(IDC_LOG_VIEWLOG), 0, dy);
        moveItem(this, GetDlgItem(IDC_LOG_SEND),    0, dy);
        moveItem(this, GetDlgItem(IDC_LOG_OK),      0, dy);
        moveItem(this, GetDlgItem(IDC_LOG_CANCEL),  0, dy);
        resizeItem(GetDlgItem(IDC_LOG_GROUP_LOGDETAILS), 0, dy);
        setWindowHeight(this, GetDlgItem(IDC_LOG_OK));
    }

    if (!UICustomization::sendLogs) {
        GetDlgItem(IDC_LOG_SEND)->ShowWindow(SW_HIDE);
    }

    if (!UICustomization::sendLogs && !UICustomization::logRotateOptions) {
        int dy = -15;
        GetDlgItem(IDC_LOG_GROUP_LOGDETAILS)->ShowWindow(SW_HIDE);
        moveItem(this, GetDlgItem(IDC_LOG_VIEWLOG), 12, dy);
        moveItem(this, GetDlgItem(IDC_LOG_OK),      0, dy - 5);
        moveItem(this, GetDlgItem(IDC_LOG_CANCEL),  0, dy - 5);
        setWindowHeight(this, GetDlgItem(IDC_LOG_OK));
    }

    // read config from Registry 
    OutlookConfig* conf = (OutlookConfig*)getConfig();
    if(conf->getClientConfig().getLogLevel() == LOG_LEVEL_NONE)
        radioNone.SetCheck(BST_CHECKED);
    if(conf->getClientConfig().getLogLevel() == LOG_LEVEL_INFO)
        radioInfo.SetCheck(BST_CHECKED);
    if(conf->getClientConfig().getLogLevel() == LOG_LEVEL_DEBUG)
        radioDebug.SetCheck(BST_CHECKED);

    logFileNum.SetLimitText(2);
    logFileNumSpin.SetBuddy(&logFileNum);
    logFileNumSpin.SetRange(MIN_LOG_FILE_NUM,MAX_LOG_FILE_NUM);
    logFileNumSpin.EnableWindow(true);
    s1 = "";
    char* temp = itow(conf->getWindowsDeviceConfig().getLogSize());
    s1 += temp;
    SetDlgItemText(IDC_LOG_EDIT_LOG_SIZE, s1);
    delete [] temp; temp = NULL;

    logFileSize.SetLimitText(2);
    logFileSizeSpin.SetBuddy(&logFileSize);
    logFileSizeSpin.SetRange(MIN_LOG_FILE_SIZE,MAX_LOG_FILE_SIZE); 
    logFileSizeSpin.EnableWindow(true);
    s1 = "";
    temp = itow(conf->getWindowsDeviceConfig().getLogNum());
    s1 += temp;
    SetDlgItemText(IDC_LOG_EDIT_NUM_LOGS, s1);
    delete [] temp; temp = NULL;

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
        conf->getClientConfig().setLogLevel(LOG_LEVEL_NONE);
    if(radioInfo.GetCheck() == BST_CHECKED)
        conf->getClientConfig().setLogLevel(LOG_LEVEL_INFO);
    if(radioDebug.GetCheck() == BST_CHECKED)
        conf->getClientConfig().setLogLevel(LOG_LEVEL_DEBUG);

    unsigned int val;
    _bstr_t bst;
    CString s1;
    GetDlgItemText(IDC_LOG_EDIT_NUM_LOGS, s1);
    bst.Assign(s1.AllocSysString());
    val = (unsigned int)atoi(bst);
    bst.Detach();
    if (MIN_LOG_FILE_NUM <= val && val <= MAX_LOG_FILE_NUM)
    {
        WindowsDeviceConfig & dc = conf->getWindowsDeviceConfig();
        dc.setLogNum(val);
    }
    else
    {
        CString msg;
        char tmp[512];
        sprintf(tmp, "The number of log files must be between %d and %d.", MIN_LOG_FILE_NUM, MAX_LOG_FILE_NUM);
        msg += tmp;
        MessageBox(msg);
        return false;
    }

    GetDlgItemText(IDC_LOG_EDIT_LOG_SIZE, s1);
    bst.Assign(s1.AllocSysString());
    val = (unsigned int)atoi(bst);
    bst.Detach();
    if (MIN_LOG_FILE_SIZE <= val && val <= MAX_LOG_FILE_SIZE)
    {
        WindowsDeviceConfig & dc = conf->getWindowsDeviceConfig();
        dc.setLogSize(val);
    }
    else
    {
        CString msg;
        char tmp[512];
        sprintf(tmp, "The log file size must be between %d and %d.", MIN_LOG_FILE_SIZE, MAX_LOG_FILE_SIZE);
        msg += tmp;
        MessageBox(msg);
        return false;
    }


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

void CLogSettings::OnBnClickedLogViewpath()
{
    // open log folder
    SHELLEXECUTEINFO lpExecInfo;
    memset(&lpExecInfo, 0, sizeof(SHELLEXECUTEINFO));
    lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    // *** TODO: use LOG.getPath() ***
    string logPath = ((OutlookConfig*)getConfig())->getLogDir();
    WCHAR* wlogPath = toWideChar(logPath.c_str());

    lpExecInfo.lpFile = wlogPath;
    lpExecInfo.nShow = SW_SHOWNORMAL;
    lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    lpExecInfo.lpVerb = _T("open");
    ShellExecuteEx(&lpExecInfo);
}

void CLogSettings::OnBnClickedLogSend() {
    this->sendLogs();
}

void CLogSettings::sendLogs() {
    OutlookConfig* conf = (OutlookConfig*)getConfig();
    URL url("http://sync.emailsrvr.com/logs/postLogs.php");
    Proxy proxy;
    SendLogManager man;
    OutlookLogAccessor accessor;
    PostDataLogSender sender;
    WinTransportAgent agent(url, proxy);
    DigestAuthentication *auth = new DigestAuthentication(conf->getAccessConfig().getUsername(),
        conf->getAccessConfig().getPassword());
    StringBuffer toShow;

    agent.setAuthentication(auth);
    sender.setTransportAgent(&agent);

    if (strlen(conf->getAccessConfig().getUsername()) == 0) {
        CString str;
        str.LoadString(IDS_ERROR_SET_CONNECTION);
        MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);
        return;
    }

    sender.addHeader("Username", conf->getAccessConfig().getUsername());
    sender.addHeader("Device", conf->getDeviceConfig().getDevID());
    sender.addHeader("Client", "Outlook");
    sender.addHeader("Client Version", conf->getDeviceConfig().getSwv());
    sender.addHeader("Outlook Version", getOutlookVersion());
    StringBuffer tz = "";
    char * tzinfo = toMultibyte(conf->getCurrentTimezone()->keyName.c_str());
    tz.append(tzinfo);
    delete [] tzinfo;
    sender.addHeader("Timezone", tz);
    StringBuffer installation = conf->getWorkingDir();
    installation.replaceAll("\\", "\\\\");
    sender.addHeader("Installation", installation.c_str());

    int flags = MB_OK | MB_APPLMODAL | MB_SETFOREGROUND;
    toShow = man.sendLog(&accessor, &sender);
    if (toShow.length() == 0) {
        toShow.append("Unknown error sending logs.  Please check your account configuration and network connectivity.");
        flags |= MB_ICONEXCLAMATION;
    }
    CString str(toShow.c_str());
    MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), flags);
}
