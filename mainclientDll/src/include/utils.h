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

#ifndef INCL_UTILS_DLL
#define INCL_UTILS_DLL

/** @cond OLPLUGIN */
/** @addtogroup utils */
/** @{ */

#include "base/fscapi.h"
#include "base/log.h"
#include "spds/SyncReport.h"
#include "WindowsSyncSource.h"
#include "outlook/ClientRecurrence.h"
#include "winmaincpp.h"

#include <string>

/// Codes used by UI ("0" is the initialization)
#define SYNCSOURCE_CALENDAR     1
#define SYNCSOURCE_CONTACTS     2
#define SYNCSOURCE_NOTES        3
#define SYNCSOURCE_TASKS        4
#define SYNCSOURCE_PICTURES     5


/*-----------utils functions -----------------*/

// Open a message box with a timeout
int TimedMessageBox(HWND hwndOwner, LPCTSTR pszMessage, LPCTSTR pszTitle, UINT flags, DWORD dwTimeout);
// Display a message box only if not scheduled sync.
int safeMessageBox (const char*   message, const char*   title = NULL, unsigned int flags = 0);
int wsafeMessageBox(const WCHAR* wmessage, const WCHAR* wtitle = NULL, unsigned int flags = 0);

_declspec(dllexport) void printLog(const char* msg,  const char* level = LOG_INFO);
_declspec(dllexport) void printLog(const WCHAR* msg, const char* level = LOG_INFO);

bool  isSIF            (const std::wstring& dataType);
bool  isSIF            (const std::string&  dataType);
bool isAcceptedDataType(const std::wstring& dataType);
bool isAcceptedDataType(const std::string&  dataType);
char* syncModeName     (SyncMode code);
bool isFullSyncMode    (SyncMode mode);

void  toWindows        (char* str);
WCHAR* readAppDataPath ();
WCHAR* readDataPath    (const WCHAR* itemType);
int    makeDataDirs    ();
int    getWindowsUser  (std::wstring& userName);
int    getWindowsUserEx(std::wstring& userName);

/**
 * Returns the default path to store pictures 
 * (shell folder 'pictures' for this user)
 */
StringBuffer getDefaultPicturesPath();

std::wstring readFromFile          (const std::wstring& filePath);
int          writeToFile           (const std::wstring& content, const std::wstring& filePath, const WCHAR* mode = L"w");
int          writeToFile           (const std::string&  content, const std::string&  filePath, const char*  mode = "w");
std::string  getSyncMutexName      ();
char*        readSystemErrorMsg    (DWORD errorCode = 0);
std::wstring getSafeItemName       (ClientItem* cItem);
void         printReport           (SyncReport* sr, SyncSource** sources);
char*        friendlyName          (const char* sourceName);

int          getBuildNumberFromVersion(const char* swv);
long         variantTimeToTimeStamp   (const double vTime);

int          syncSourceNameToIndex(const StringBuffer& sourceName);
StringBuffer syncSourceIndexToName(const int sourceID);


/**
 * Used to quicky check if a specific source is visible or not.
 * It checks the sourcesVisible array.
 */
bool isSourceVisibleA(const char* sourceName);

/**
 * Used to quicky check if a specific source is visible or not (WCHAR version).
 * It checks the sourcesVisible array.
 */
bool isSourceVisible(const WCHAR* sourceName);

/**
 * Used to check the total number of sources visible.
 * It checks the sourcesVisible array.
 */
int countSourceVisible();


/**
 * Is the given status code an error status code? Error codes are the ones
 * outside the range 200-299.
 *
 * @param status the status code to check
 */
inline static bool isErrorStatus(int status) {
    return (status) && ((status < 200) || (status > 299));
}

/** @} */
/** @endcond */
#endif