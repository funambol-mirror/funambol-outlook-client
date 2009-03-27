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


#include "outlook/defs.h"
#include "outlook/ClientException.h"
#include "base/Log.h"
#include "winmaincpp.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

#include <algorithm>
#include <string>

using namespace std;



//
// -------------- Conversion functions for item type coding ---------------------
//

/*
 * itemType (string) -> OlDefaultFolders (int)
 */
const OlDefaultFolders getDefaultFolderType(const wstring& itemType) {

    for(int i=0; i<ITEM_TYPES_COUNT; i++) {
        if (itemTypes[i].name == itemType) {
            return itemTypes[i].olFolderType;
        }
    }

    // If not found
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_BAD_ITEMTYPE, itemType.c_str());
    throwClientFatalException(getLastErrorMsg());
    return (OlDefaultFolders)NULL;
}


/*
 * itemType (string) -> OlItemType (int)
 */
const OlItemType getOlItemType(const wstring& itemType) {

    for(int i=0; i<ITEM_TYPES_COUNT; i++) {
        if (itemTypes[i].name == itemType) {
            return itemTypes[i].olType;
        }
    }

    // If not found
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_BAD_ITEMTYPE, itemType.c_str());
    throwClientFatalException(getLastErrorMsg());
    return (OlItemType)NULL;
}


/*
 * OlItemType (int) -> itemType (string)
 */
const wstring getItemTypeFromOlType(const OlItemType olType) {

    for(int i=0; i<ITEM_TYPES_COUNT; i++) {
        if (itemTypes[i].olType == olType) {
            return itemTypes[i].name;
        }
    }

    // If not found
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_BAD_OLTYPE, olType);
    throwClientFatalException(getLastErrorMsg());
    return NULL;
}
// ----------------------------------------------------------------------------



/*
 * Actions to execute when a COM pointer exception occurs.
 */
void manageComErrors(_com_error &e) {

    setErrorF((int)e.Error(), ERR_COM_POINTER, e.Error(), e.ErrorMessage());

    LOG.error(getLastErrorMsg());
    //safeMessageBox(getLastErrorMsg());
}





/*
 * Return the program name from version number.
 *
 * 12  -> Outlook 2007
 * 11  -> Outlook 2003
 * 10  -> Outlook XP
 * 9   -> Outlook 2000
 * 8.5 -> Outlook 98    *** TBD: NOT TESTED ***
 * 8.0 -> Outlook 97    *** TBD: NOT TESTED ***
 */
const wstring getNameFromVersion(wstring version) {

    int majorVersion = _wtoi(version.c_str());
    wstring name = EMPTY_WSTRING;

    switch (majorVersion) {
        case 12: {
            name = OUTLOOK_2007;
            break;
        }
        case 11: {
            name = OUTLOOK_2003;
            break;
        }
        case 10: {
            name = OUTLOOK_XP;
            break;
        }
        case 9: {
            name = OUTLOOK_2000;
            break;
        }
        case 8: {
            int minorVersion = _wtoi(&version[2]);
            if (minorVersion == 0) {
                name = OUTLOOK_97;
            }
            else if (minorVersion == 5) {
                name = OUTLOOK_98;
            }
            else {
                goto error;
            }
            break;
        }
        default: {
            goto error;
        }
    }

    return name;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_INVALID_VERSION, version.c_str());
    throwClientFatalException(getLastErrorMsg());
    return NULL;
}



//
// ------------------------------ DATE/TIME CONVERSIONS FUNCTIONS ------------------------------
//
/**
 * Variant time (double) -> System time ("YYYYMMDD" or "YYYYMMDDThhmmssZ")
 * The output value is a string. It's in UTC based on param 'toUTC'.
 * If onlyDate = true, toUTC is ignored.
 *
 * @param systemDate   [OUT] the date returned in SystemTime format
 * @param doubleDate   the input date in variant time format
 * @param toUTC        TRUE if we want conversion in UTC format (YYYYMMDDThhmmssZ)
 * @param onlyDate     if true, the output systemTime is in date format "yyyyMMdd" (hours info are cut)
 */
void doubleToSystemTime(wstring& systemDate, const DATE doubleDate, const BOOL toUTC, const bool onlyDate) {

    if (!doubleDate || doubleDate > LIMIT_MAX_DATE) {
        systemDate = L"";
        return;
    }

    SYSTEMTIME t;
    VariantTimeToSystemTime(doubleDate, &t);
    if (!onlyDate && toUTC) {
        localTimeToUTC(t);
    }

    WCHAR date[20];
    wsprintf(date, TEXT("%i%02i%02i"), t.wYear, t.wMonth, t.wDay);
    if (!onlyDate) {
        wsprintf(&date[8], TEXT("T%02i%02i%02i"), t.wHour, t.wMinute, t.wSecond);
    }

    systemDate = date;

    if (!onlyDate && toUTC) {
        systemDate += TEXT("Z");
    }
}




/**
 * String time ("YYYYMMDDThhmmssZ" or "YYYYMMDD") -> Variant time (double).
 * Automatic conversion from UTC to Local time if input time is UTC.
 * If onlyDate = true, time is not converted to UTC and hour is forced to 00:00.
 *
 * @param dataString  the input string in System time format
 * @param date        [OUT] the returned value into VariantTime format
 * @param onlyDate    if true, time is not converted to UTC and hour is forced to 00:00.
 */
void systemTimeToDouble(const wstring& dataString, DATE* date, bool onlyDate) {

    bool isUTC = false;
    WCHAR inputTime[20];
    SYSTEMTIME t;

    if (dataString.size() < 8) {
        *date = NULL;  // Error!
        return;
    }

    wsprintf(inputTime, dataString.c_str());

    wstring::size_type pos = dataString.find(L"-", 0);
    if (pos == wstring::npos) {
        // "yyyyMMdd"
        swscanf(inputTime, L"%4d%2d%2d", &t.wYear, &t.wMonth, &t.wDay);

        if (dataString.size() > 9 && dataString.size() < 17) {
            // "hhmmss"
            swscanf(&inputTime[9], L"%2d%2d%2d", &t.wHour, &t.wMinute, &t.wSecond);

            // Check if UTC format.
            pos = dataString.find(L"Z", 8);
            if (pos != wstring::npos)  isUTC = true;
        }
        else {
            t.wHour   = 0;
            t.wMinute = 0;
            t.wSecond = 0;
        }
    }
    else {
        // old format: "yyyy-MM-dd"
        swscanf(inputTime, L"%4d-%2d-%2d", &t.wYear, &t.wMonth, &t.wDay);
        t.wHour   = 0;
        t.wMinute = 0;
        t.wSecond = 0;
    }
    t.wMilliseconds = 0;
    t.wDayOfWeek    = 0;


    // Convert to local-time if necessary.
    if(!onlyDate && isUTC) {
        UTCToLocalTime(t);
    }

    // Force hours to 00:00 if asked.
    if (onlyDate) {
        t.wHour   = 0;
        t.wMinute = 0;
        t.wSecond = 0;
    }

    SystemTimeToVariantTime(&t, date);
}




/**
 * Converts 'sysTime' from local time to UTC.
 * Uses the specific settings for the time zone and daylight saving time
 * that are applied to the 'sysTime' date.
 * @note  correction is applied for DST change in the US since 2007.
 */
void localTimeToUTC(SYSTEMTIME &sysTime) {

    TIME_ZONE_INFORMATION timeZoneInfo;
    SYSTEMTIME utcTime;

    GetTimeZoneInformation(&timeZoneInfo);
    adjustDSTSettings(&timeZoneInfo, sysTime);
    TzSpecificLocalTimeToSystemTime(&timeZoneInfo, &sysTime, &utcTime);
    sysTime = utcTime;
}


/**
 * Converts 'sysTime' from UTC to local time.
 * Uses the specific settings for the time zone and daylight saving time
 * that are applied to the 'sysTime' date.
 * @note  correction is applied for DST change in the US since 2007.
 */
void UTCToLocalTime(SYSTEMTIME &sysTime) {

    TIME_ZONE_INFORMATION timeZoneInfo;
    SYSTEMTIME localTime;

    GetTimeZoneInformation(&timeZoneInfo);
    adjustDSTSettings(&timeZoneInfo, sysTime);
    SystemTimeToTzSpecificLocalTime(&timeZoneInfo, &sysTime, &localTime);
    sysTime = localTime;
}


/**
 * Correct the DST dates inside timeZoneInfo passed, if necessary.
 * In 2007 the DST rules chaged for U.S. and Canada, so we need to
 * adjust the timeZoneInfo if current Timezone is one of the timezones
 * affected by the change of Energy Policy Act of 2005.
 * @see http://en.wikipedia.org/wiki/Energy_Policy_Act_of_2005
 *
 * @param timeZoneInfo  [IN-OUT] the TIME_ZONE_INFORMATION structure to adjust
 * @param targetTime    the target date/time analyzed
 * @return              true if the adjustment was done
 */
bool adjustDSTSettings(TIME_ZONE_INFORMATION* timeZoneInfo, SYSTEMTIME targetTime) {

    // TZ to be corrected only for U.S. and Canada.
    OutlookConfig* config = getConfig();
    if (isTZForDSTChange(config->getCurrentTimezone()->keyName) == false) {
        return false;
    }

    // DST changed in 2007
    if (targetTime.wYear < 2007) {
        if (timeZoneInfo->DaylightDate.wMonth == 3 &&
            timeZoneInfo->DaylightDate.wDay   == 2) {
            // New TZinfo detected with old targetTime.
            // Adjust DST start and end dates: from first Sunday in April to
            // last Sunday in October.
            timeZoneInfo->DaylightDate.wMonth = 4;          // April
            timeZoneInfo->DaylightDate.wDay   = 1;          // First occurrence
            timeZoneInfo->StandardDate.wMonth = 10;         // October
            timeZoneInfo->StandardDate.wDay   = 5;          // Last occurrence
            return true;
        }
    }
    else {
        if (timeZoneInfo->DaylightDate.wMonth == 4 &&
            timeZoneInfo->DaylightDate.wDay   == 1) {
            // Old TZinfo detected with new targetTime.
            // Adjust DST start and end dates: from second Sunday in March to
            // first Sunday in November.
            timeZoneInfo->DaylightDate.wMonth = 3;          // March
            timeZoneInfo->DaylightDate.wDay   = 2;          // Second occurrence
            timeZoneInfo->StandardDate.wMonth = 11;         // November
            timeZoneInfo->StandardDate.wDay   = 1;          // First occurrence
            return true;
        }
    }
    return false;
}


/**
 * Returns true if the passed timezone name is one of the timezones
 * that changed their rule in 2007.
 */
bool isTZForDSTChange(const wstring& tzName) {

    if ( tzName == TZ_ALASKA     ||
         tzName == TZ_CENTRAL_US ||
         tzName == TZ_EASTERN_US ||
         tzName == TZ_MOUNTAIN   ||
         tzName == TZ_PACIFIC_US ) {
        return true;
    }
    return false;
}




//
// ------------------------------ BOOLEAN CONVERSIONS FUNCTIONS ------------------------------
//

/// Variant bool (-1/0) to BOOL (1/0).
BOOL vBoolToBOOL(VARIANT_BOOL vbool) {
    if (vbool==VARIANT_TRUE)
        return TRUE;
    else
        return FALSE;
}

/// BOOL (1/0) to Variant bool (-1/0).
VARIANT_BOOL BOOLToVBool(BOOL b) {
    if (b==TRUE)
        return VARIANT_TRUE;
    else
        return VARIANT_FALSE;
}

/// Variant bool (-1/0) to bool (true/false).
bool vBoolToBool(VARIANT_BOOL vbool) {
    if (vbool==VARIANT_TRUE)
        return true;
    else
        return false;
}