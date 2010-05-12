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

        // No error: out
        case 0: {
            return;
        }
        // Generic error -> see log.
        case 1: {
            s1.LoadString(IDS_ERROR_SYNC_NOT_COMPLETED);
            break;
        }
        // Aborted by user (soft termination) -> no msgbox
        case 2: {
            return;
        }
        
        case 3:     // Outlook fatal exception              -> force exit the plugin!
        case 4:     // Thread terminated (hard termination) -> force exit the plugin!
        {
            s1.LoadString(IDS_ERROR_SYNC_TERMINATED);
            wsafeMessageBox(s1.GetBuffer());
            exit(1);
        }

        // Aborted by user to avoid full-sync -> no msgbox
        case 5: {
            return;
        }

        case 6:     // Unexpected exception.
        case 7:     // Unexpected STL exception.
        {
            s1.LoadString(IDS_UNEXPECTED_EXCEPTION);
            break;
        }


        case -10:
            s1.LoadString(IDS_CODE_NOSOURCE_10); break;             // *** OBSOLETE? ***
        case -6:
            s1.LoadString(IDS_CODE_SYNC_STOPPED);break;             // *** OBSOLETE? ***

        case ERR_CODE_FOLDER_PATH_MATCH:
            s1.LoadString(IDS_CODE_FOLDER_PATH_NOT_FOUND);
            break;

        case 402:
            s1.LoadString(IDS_CODE_AUTH_EXPIRED_402); break;
        case 403:
            s1.LoadString(IDS_CODE_FORBIDDEN_403);    break;

        case 404:       // Remote name of some source is wrong
            s1.LoadString(IDS_CODE_NOTFOUND_404); break;
        case 407:
        case 401:
            s1.LoadString(IDS_CODE_INVALID_CREDENTIALS_401); break;

        case 417:       // Retry later
            s1.LoadString(IDS_CODE_SERVER_BUSY); break;
        case 503:       // Service unavailable (another sync in progress)
            s1.LoadString(IDS_CODE_SERVER_BUSY_SYNC); break;

        case 2001:      // Host name is wrong
        case 2060:      // Server path is wrong
            s1.LoadString(IDS_CODE_ERROR_CONNECT_2001); break;
        case 2061:      // Server timeout
            s1.LoadString(IDS_ERR_SERVER_TIMOUT); break;

        case 2002:
            s1.LoadString(IDS_CODE_ERROR_READING_CONTENT_2002); break;
        case 2003:
            s1.LoadString(IDS_CODE_SERVER_NOT_FOUND_2003); break;
        case 2005:
            s1.LoadString(IDS_CODE_INTERNET_CONNECTION_MISSING_2005); break;
        case 2007:
        case 2029:
        case 2050:
            s1.LoadString(IDS_CODE_NETWORK_ERROR_2007); break;
        case 2052:
            s1.LoadString(IDS_CODE_SERVER_ERROR_2052); break;

        case ERR_CODE_DROPPED_ITEMS:         // Dropped items on Client
        {
            s1.LoadString(IDS_CODE_DROPPED_ITEMS); 
            break;
        }
        case ERR_CODE_DROPPED_ITEMS_SERVER:  // Dropped items on Server
        {
            s1.LoadString(IDS_CODE_DROPPED_ITEMS_SERVER); 
            break;
        }

        case ERR_CODE_NO_SOURCES:           // No sources to sync
        {
            s1.LoadString(IDS_CODE_NO_SOURCES); 
            break;
        }

        default: break;
    }

    //
    // Display the messagebox with error to the user.
    //
    if(s1 == "") {
        s1.LoadString(IDS_ERROR_SYNC_NOT_COMPLETED);
    }
    wsafeMessageBox(s1.GetBuffer());

}


int getSyncModeCode(const char* syncMode){
    int code = SYNC_NONE;

    if(strcmp(syncMode, "none") == 0)
        code = SYNC_NONE;
    if(strcmp(syncMode, "two-way") == 0)
        code = SYNC_TWO_WAY;
    if(strcmp(syncMode, "slow") == 0)
        code = SYNC_SLOW;
    if(strcmp(syncMode, "one-way-from-client") == 0)
        code = SYNC_ONE_WAY_FROM_CLIENT;
    if(strcmp(syncMode, "refresh-from-client") == 0)
        code = SYNC_REFRESH_FROM_CLIENT;
    if(strcmp(syncMode, "one-way-from-server") == 0)
        code = SYNC_ONE_WAY_FROM_SERVER;
    if(strcmp(syncMode, "refresh-from-server") == 0)
        code = SYNC_REFRESH_FROM_SERVER;

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
    swscanf(wdate, L"%4d%2d%2d", &timeDest.wYear, &timeDest.wMonth, &timeDest.wDay);
    
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
