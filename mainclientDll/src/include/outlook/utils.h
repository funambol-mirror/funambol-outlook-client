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

#ifndef INCL_UTILS_OL
#define INCL_UTILS_OL

/** @cond OLPLUGIN */
/** @addtogroup outlook_utils */
/** @{ */


// Timezones that are changing their rule in 2007
#define TZ_EASTERN_US           TEXT("Eastern Standard Time" )
#define TZ_CENTRAL_US           TEXT("Central Standard Time" )
#define TZ_MOUNTAIN             TEXT("Mountain Standard Time")
#define TZ_PACIFIC_US           TEXT("Pacific Standard Time" )
#define TZ_ALASKA               TEXT("Alaska Standard Time"  )


#include "base/fscapi.h"
#include "outlook/defs.h"
#include "winmaincpp.h"

#include <string>


// Item types conversions.
const OlDefaultFolders  getDefaultFolderType (const std::wstring& itemType);
const OlItemType        getOlItemType        (const std::wstring& itemType);
const std::wstring      getItemTypeFromOlType(const OlItemType olType);

void manageComErrors(_com_error &e);
const std::wstring getNameFromVersion(std::wstring version);


// DATE/TIME conversions.
void doubleToSystemTime(std::wstring& systemDate, const DATE doubleDate, const BOOL toUTC, const bool onlyDate = false);
void systemTimeToDouble(const std::wstring& dataString, DATE* date, bool onlyDate = false);
void localTimeToUTC    (SYSTEMTIME &sysTime);
void UTCToLocalTime    (SYSTEMTIME &sysTime);
bool adjustDSTSettings (TIME_ZONE_INFORMATION* timeZoneInfo, SYSTEMTIME targetTime);
bool isTZForDSTChange  (const std::wstring& tzName);


// Boolean conversions.
BOOL         vBoolToBOOL(VARIANT_BOOL vbool);
bool         vBoolToBool(VARIANT_BOOL vbool);
VARIANT_BOOL BOOLToVBool(BOOL b);


/** @} */
/** @endcond */
#endif