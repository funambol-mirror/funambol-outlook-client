/*
 * Funambol is a mobile platform developed by Funambol, Inc.
 * Copyright (C) 2003 - 2009 Funambol, Inc.
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
#include "MediaHubSetting.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"

#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"
#include "customization.h"

#include "shlobj.h"     // to browse for folder

using namespace std;

// used to exchange the default browse folder (between BrowseCallbackProc and browseFolder)
static wstring defaultBrowseFolder;

IMPLEMENT_DYNCREATE(CMediaHubSetting, CDialog)

CMediaHubSetting::CMediaHubSetting() : CDialog(CMediaHubSetting::IDD) {

    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(PICTURE_);
    if (!ssconf) {
        printLog("Config not found for source videos!", LOG_ERROR);
    }
}

CMediaHubSetting::~CMediaHubSetting() {}

void CMediaHubSetting::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);    
    DDX_Control(pDX, IDC_MEDIA_HUB_BUT_SELECT,      butSelectFolder);    
    DDX_Control(pDX, IDC_MEDIA_HUB_GROUP_FOLDER,    groupFolder);
    DDX_Control(pDX, IDC_MEDIA_HUB_EDIT_FOLDER,     editFolder);
    
}

BEGIN_MESSAGE_MAP(CMediaHubSetting, CDialog)
    ON_BN_CLICKED(IDC_MEDIA_HUB_OK,         &CMediaHubSetting::OnBnClickedMediaHubOk)
    ON_BN_CLICKED(IDC_MEDIA_HUB_CANCEL,     &CMediaHubSetting::OnBnClickedMediaHubCancel)
    ON_BN_CLICKED(IDC_MEDIA_HUB_BUT_SELECT, &CMediaHubSetting::OnBnClickedMediaHubButSelect)  
    ON_BN_CLICKED(IDC_MEDIA_HUB_BUT_RESET,  &CMediaHubSetting::OnBnClickedMediaHubButReset)
END_MESSAGE_MAP()


#ifdef _DEBUG
void CMediaHubSetting::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CMediaHubSetting::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


StringBuffer getMediaHubDefault() {
    StringBuffer path;
    path.append("\\"); 
    path.append(MEDIA_HUB_DEFAULT_FOLDER);         
    return path;
}

StringBuffer getFullMediaHubDefault() {
    StringBuffer path = getDefaultMyDocumentsPath();
    path.append("\\"); 
    path.append(MEDIA_HUB_DEFAULT_FOLDER);         
    return path;
}


BOOL CMediaHubSetting::OnInitDialog() {

    if (!ssconf) return FALSE;

    bool showAdvanced = true;
    CString s1;
    s1.LoadString(IDS_MEDIA_HUB_TITLE);
    SetWindowText(s1);
    CDialog::OnInitDialog();

    editFolder.SetLimitText  (EDIT_TEXT_MAXLENGTH);    
  
    // load string resources
    s1.LoadString(IDS_MEDIA_HUB_GROUP_FOLDER_LABEL);    SetDlgItemText(IDC_MEDIA_HUB_GROUP_FOLDER,       s1);    
    //s1.LoadString(IDS_CURRENT);             SetDlgItemText(IDC_MEDIA_HUB_STATIC_FOLDER,      s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_MEDIA_HUB_BUT_SELECT,         s1);
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_MEDIA_HUB_OK,                 s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_MEDIA_HUB_CANCEL,             s1);
    s1.LoadString(IDS_RESET_BUTTON);        SetDlgItemText(IDC_MEDIA_HUB_BUT_RESET,          s1);
    s1.LoadString(IDS_MEDIA_HUB_EXPLAIN_LABEL);  SetDlgItemText(IDC_MEDIA_HUB_EXPLAIN_LABEL, s1);
    
   
    StringBuffer path = ssconf->getCommonConfig()->getProperty(PROPERTY_MEDIAHUB_PATH);
    if (path.empty() == false) {
        return true;        
    }
    
    path = getFullMediaHubDefault();        
    WCHAR* wpath = toWideChar(path.c_str());
    s1 = wpath;
    delete [] wpath;
    SetDlgItemText(IDC_MEDIA_HUB_EDIT_FOLDER, s1);
           
    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if (((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");        
        pfnSetWindowTheme (groupFolder.m_hWnd,    L" ", L" ");
        
    }   

    GetDlgItem(IDC_MEDIA_HUB_BUT_SELECT)->SetFocus();
    GetDlgItem(IDC_MEDIA_HUB_BUT_RESET)->EnableWindow(FALSE);

    return FALSE;
}


void CMediaHubSetting::OnBnClickedMediaHubOk()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CMediaHubSetting::OnBnClickedMediaHubCancel()
{
    // will save when 'OK' is clicked on SyncSettings.
    CDialog::OnCancel();
}

bool CMediaHubSetting::saveSettings(bool saveToDisk) {
 
    CString mediaPath;
    GetDlgItemText(IDC_MEDIA_HUB_EDIT_FOLDER, mediaPath);
    char* path = toMultibyte(mediaPath.GetBuffer());
    
    if (path) {
        StringBuffer p(path);
        createFolder(p.c_str());
        setPathOfAllSources(p);        
        delete [] path;
    }        
    return true;
}


void CMediaHubSetting::OnBnClickedMediaHubButSelect() {
   
    // Get the default browse folder to the current path of videos       
    CString path;
    GetDlgItemText(IDC_MEDIA_HUB_EDIT_FOLDER, path);
    
    WCHAR* t = toWideChar(getMediaHubDefault().c_str());
    CString mediaHub = t;
    CString mediaDir = path;
    int found = mediaDir.Find(mediaHub);
    if (found > -1) {
        mediaDir.Truncate(found);
    }

    CString caption;
    caption.LoadString(IDS_MEDIA_HUB_TITLE_PICKER);
    
    // Open the browse for folder window (modal)
    wstring newPath;
    if ( browseFolder(newPath, mediaDir.GetBuffer(), caption.GetBuffer(), GetSafeHwnd()) ) {
        // Update the UI label and save the new path
        newPath.append(t);
        SetDlgItemText(IDC_MEDIA_HUB_EDIT_FOLDER, newPath.c_str());     

        // check if the folder is different to the default one we enable the "reset" button
        StringBuffer t; t.convert(newPath.c_str());
        if (t != getFullMediaHubDefault()) {
            GetDlgItem(IDC_MEDIA_HUB_BUT_RESET)->EnableWindow(TRUE);
        } else {
            GetDlgItem(IDC_MEDIA_HUB_BUT_RESET)->EnableWindow(FALSE);
        }

    }   
    delete [] t;
}

/// Callback fuction for the 'browse for folder' window. Sets the default folder.
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {

    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(defaultBrowseFolder.c_str()));
    }
    return 0;
}

bool CMediaHubSetting::browseFolder(wstring& folderpath, const WCHAR* defaultFolder, const WCHAR* szCaption, const HWND hOwner) {

    bool retVal = false;

    // The BROWSEINFO struct tells the shell how it should display the dialog.
    BROWSEINFO bi;
    memset(&bi, 0, sizeof(bi));

    bi.ulFlags   = BIF_USENEWUI | BIF_VALIDATE;
    //bi.ulFlags   = BIF_NEWDIALOGSTYLE | BIF_VALIDATE;
    bi.hwndOwner = hOwner;
    bi.lpszTitle = szCaption;

    if (defaultFolder) {
        // The callback function will set this folder as the default one for browse
        defaultBrowseFolder = defaultFolder;
        bi.lpfn = BrowseCallbackProc;
    }

    // must call this if using BIF_USENEWUI
    OleInitialize(NULL);

    // Show the dialog and get the itemIDList for the selected folder.
    LPITEMIDLIST pIDL = SHBrowseForFolder(&bi);

    if (pIDL) {
        // Create a buffer to store the path, then get the path.
        WCHAR buffer[_MAX_PATH] = {'\0'};

        if (SHGetPathFromIDList(pIDL, buffer) != 0) {
            folderpath = buffer;
            retVal = true;
        }
        // free the item id list
        CoTaskMemFree(pIDL);
    }

    OleUninitialize();
    return retVal;
}


void CMediaHubSetting::OnBnClickedMediaHubButReset()
{       
    // Get the default browse folder to the current path of videos
    StringBuffer path = getFullMediaHubDefault();
    WCHAR* defaultPath = toWideChar(path.c_str());

    SetDlgItemText(IDC_MEDIA_HUB_EDIT_FOLDER, defaultPath);      
    delete [] defaultPath;
    GetDlgItem(IDC_MEDIA_HUB_BUT_RESET)->EnableWindow(FALSE);
}


void CMediaHubSetting::setPathOfAllSources(StringBuffer path) {

    WindowsSyncSourceConfig* ssconf;
    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(PICTURE_);
    if (!ssconf) {
        printLog("Config not found for source picture!", LOG_ERROR);
    }
    ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
    
    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(VIDEO_);
    if (!ssconf) {
        printLog("Config not found for source videos!", LOG_ERROR);
    }
    ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
    
    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(FILES_);
    if (!ssconf) {
        printLog("Config not found for source files!", LOG_ERROR);
    }
    ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());

}