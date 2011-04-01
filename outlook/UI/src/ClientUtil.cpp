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
#include "HwndFunctions.h"
#include "Resource.h"
#include "utils.h"
#include "winmaincpp.h"
#include "CalendarSettings.h"
#include "ClientUtil.h"


int getSyncTypeIndex(const char* syncType){
    int value = 0;
    if(! strcmp(syncType,"two-way"))
        value = 0;
    if(! strcmp(syncType,"one-way-from-server"))
        value = 1;
    if(! strcmp(syncType,"one-way-from-client"))
        value = 2;

    return value;
}

const char* getSyncTypeName( int index )
{
    switch(index)
    {
    case 0:
        return "two-way"; break;
    case 1:
        return "one-way-from-server"; break;
    case 2:
        return "one-way-from-client"; break;
    default:
        return "none"; break;
    }
}

const char* getFullSyncTypeName( int index )
{
    switch(index)
    {
    case 0:
        return "slow"; break;
    case 1:
        return "refresh-from-server"; break;
    case 2:
        return "refresh-from-client"; break;
    default:
        return "none"; break;
    }
}


int getDateFilterIndex(const DateFilter::RelativeLowerDate value) {
    
    // Now it's the same, but can be different in general.
    return value;
}

DateFilter::RelativeLowerDate getDateFilterValue(const int index) {
    
    if (index >= 0 && index < DATE_FILTER_NUM_ITEMS) {
        // Now it's the same, but can be different in general.
        return (DateFilter::RelativeLowerDate)index;
    }
    return DateFilter::LAST_MONTH;   // Default
}




/**
 * Error messages prompted to the user are managed only inside this function.
 * A message box will be displayed, based on the error code passed.
 */
void manageSyncErrorMsg(long code) {

    CString s1("");

    switch(code) {

        case WIN_ERR_NONE: {                            // 0: No error
            return;
        }
        case WIN_ERR_GENERIC: {                         // 1: Generic error -> see log.
            s1.LoadString(IDS_ERROR_SYNC_NOT_COMPLETED);
            break;
        }
        case WIN_ERR_SYNC_CANCELED: {                   // 2: Aborted -> no msgbox
            return;
        }        
        case WIN_ERR_FATAL_OL_EXCEPTION:                // 3 -> force exit the plugin!
        case WIN_ERR_THREAD_TERMINATED:                 // 4 -> force exit the plugin!
        {
            s1.LoadString(IDS_ERROR_SYNC_TERMINATED);
            wsafeMessageBox(s1.GetBuffer());
            exit(1);
        }
        case WIN_ERR_FULL_SYNC_CANCELED: {              // 5 -> deprecated, no msgbox
            return;
        }
        case WIN_ERR_UNEXPECTED_EXCEPTION:              // 6
        case WIN_ERR_UNEXPECTED_STL_EXCEPTION:          // 7
        {
            s1.LoadString(IDS_UNEXPECTED_EXCEPTION);
            break;
        }
        case WIN_ERR_SERVER_QUOTA_EXCEEDED:             // 8: Server quota exceeded
        {
            s1.LoadString(IDS_MEDIA_QUOTA_EXCEEDED);
            break;
        }
        case WIN_ERR_LOCAL_STORAGE_FULL:                // 9: Local storage full
        {
            s1.LoadString(IDS_MEDIA_STORAGE_FULL);
            break;
        }
        case WIN_ERR_DROPPED_ITEMS:                     // 10: Dropped items on Client
        {
            s1.LoadString(IDS_CODE_DROPPED_ITEMS); 
            break;
        }
        case WIN_ERR_DROPPED_ITEMS_SERVER:              // 11: Dropped items on Server
        {
            s1.LoadString(IDS_CODE_DROPPED_ITEMS_SERVER); 
            break;
        }
        case WIN_ERR_NO_SOURCES:                        // 12: No sources to sync
        {
            s1.LoadString(IDS_CODE_NO_SOURCES); 
            break;
        }
        case WIN_ERR_SAPI_NOT_SUPPORTED: {              // 13: Source (sapi) not supported
            return;
        }
        case WIN_ERR_INVALID_CREDENTIALS:               // 401
        case WIN_ERR_PROXY_AUTH_REQUIRED:               // 407
        {
            s1.LoadString(IDS_CODE_INVALID_CREDENTIALS_401);
            break;
        }
        case WIN_ERR_REMOTE_NAME_NOT_FOUND:             // 404
        {
            s1.LoadString(IDS_CODE_NOTFOUND_404); 
            break;
        }
        case WIN_ERR_WRONG_HOST_NAME:                   // 2001
        case WIN_ERR_NETWORK_ERROR:                     // 2050
        {
            s1.LoadString(IDS_CODE_NETWORK_ERROR_2007); 
            break;
        }


        //
        // following are obsolete?
        //
        case 402:
            s1.LoadString(IDS_CODE_AUTH_EXPIRED_402); 
            break;
        case 403:
            s1.LoadString(IDS_CODE_FORBIDDEN_403);    
            break;
        case 417:       // Retry later
            s1.LoadString(IDS_CODE_SERVER_BUSY); 
            break;

        case 2061:      // Server timeout
            s1.LoadString(IDS_ERR_SERVER_TIMOUT); 
            break;
        case 2007:
        case 2029:
        case 2060:      // Server path is wrong
            s1.LoadString(IDS_CODE_NETWORK_ERROR_2007); 
            break;
        case 2052:
            s1.LoadString(IDS_CODE_SERVER_ERROR_2052); 
            break;

        default: 
            break;
    }

    //
    // Display the messagebox with error to the user.
    //
    if(s1 == "") {
        s1.LoadString(IDS_ERROR_SYNC_NOT_COMPLETED);
    }
    wsafeMessageBox(s1.GetBuffer());

}


int manageWinErrors(const int winErrorCode) {

    int sourceState;
    switch (winErrorCode) {
        case 0:
            sourceState = SYNCSOURCE_STATE_OK;
            break;
        case 2:
            sourceState = SYNCSOURCE_STATE_CANCELED;
            break;
        case WIN_ERR_SERVER_QUOTA_EXCEEDED:
            sourceState = SYNCSOURCE_STATE_QUOTA_EXCEEDED;
            break;
        case WIN_ERR_LOCAL_STORAGE_FULL:
            sourceState = SYNCSOURCE_STATE_STORAGE_FULL;
            break;
        case WIN_ERR_SAPI_NOT_SUPPORTED:
            sourceState = SYNCSOURCE_STATE_NOT_SUPPORTED;
            break;
        default:
            sourceState = SYNCSOURCE_STATE_FAILED;
            break;
    }
    return sourceState;
}


int getSyncModeCode(const char* syncMode){
    int code = SYNC_NONE;

    if(strcmp(syncMode, "none") == 0)
        code = SYNC_NONE;
    else if(strcmp(syncMode, "two-way") == 0)
        code = SYNC_TWO_WAY;
    else if(strcmp(syncMode, "slow") == 0)
        code = SYNC_SLOW;
    else if(strcmp(syncMode, "one-way-from-client") == 0)
        code = SYNC_ONE_WAY_FROM_CLIENT;
    else if(strcmp(syncMode, "refresh-from-client") == 0)
        code = SYNC_REFRESH_FROM_CLIENT;
    else if(strcmp(syncMode, "one-way-from-server") == 0)
        code = SYNC_ONE_WAY_FROM_SERVER;
    else if(strcmp(syncMode, "refresh-from-server") == 0)
        code = SYNC_REFRESH_FROM_SERVER;
    else if(strcmp(syncMode, "smart-one-way-from-client") == 0)
        code = SYNC_SMART_ONE_WAY_FROM_CLIENT;
    else if(strcmp(syncMode, "smart-one-way-from-server") == 0)
        code = SYNC_SMART_ONE_WAY_FROM_SERVER;

    return code;
}


/**
 * Utility to retrieve the relative position of 'wnd' window respect to 'parentWnd'.
 */
CPoint getRelativePosition(CWnd* wnd, CWnd* parentWnd) {
    
    CPoint pos(0,0);
    if (!wnd || !parentWnd) {
        return pos;
    }

    WINDOWINFO pwi, pwiParent;
    wnd->GetWindowInfo(&pwi);
    parentWnd->GetWindowInfo(&pwiParent);

    pos.x = pwi.rcClient.left - pwiParent.rcClient.left;
    pos.y = pwi.rcClient.top  - pwiParent.rcClient.top;
    return pos;
}

CPoint getMainWindowSize() {
    
    HDC hdc = ::GetDC(0);
    int dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
    ::ReleaseDC(0, hdc);

    //
    // TODO: set the window size dynamically based on the source number
    //
    int sizeX = FRAME_MAIN_X;
    int sizeY = FRAME_MAIN_Y;
    if (isSourceVisible(PICTURE)) {
        sizeY += SOURCE_PANE_SIZE_Y;
    }

    double dx = sizeX * ((double)dpiX/96);      // default DPI = 96
    double dy = sizeY * ((double)dpiY/96);      // default DPI = 96

    CPoint point((int)dx, (int)dy);
    return point;
}


void trim(wstring& str) {
    wstring::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != string::npos) 
            str.erase(0, pos);
    }
    else 
        str.erase(str.begin(), str.end());
}

wstring formatDate(StringBuffer& date) {
    
    wstring dd(TEXT(""));
    wchar_t* wdate = toWideChar(date);
    if (wdate == NULL) {
        return dd;
    }
    wchar_t data[80];
    wchar_t formatDate[80];
    int found = 0;
    SYSTEMTIME timeDest;
    swscanf_s(wdate, L"%4d%2d%2d", &timeDest.wYear, &timeDest.wMonth, &timeDest.wDay);
    
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, data, 80);

    dd = data;            
    if ((found = dd.find(TEXT("dddd, "))) != wstring::npos) {
        dd.replace(found, 6, TEXT(""));
    } else if ((found = dd.find(TEXT("dddd,"))) != wstring::npos) {
        dd.replace(found, 5, TEXT(""));
    }else if ((found = dd.find(TEXT("dddd"))) != wstring::npos) {
        dd.replace(found, 4, TEXT(""));
    }

    trim(dd);            
    GetDateFormat(LOCALE_USER_DEFAULT, NULL, &timeDest, dd.c_str(), formatDate, 80); 
    dd = formatDate;
    return dd;
}

StringBuffer ConvertToChar(CString &s)
{
    StringBuffer ret("");
    char* buf = toMultibyte(s.GetBuffer());
    if (buf) {
        ret = buf;
    }
    delete [] buf;
    return ret;
}
