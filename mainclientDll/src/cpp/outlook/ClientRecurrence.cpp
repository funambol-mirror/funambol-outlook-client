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

#include "base/fscapi.h"
#include "base/Log.h"
#include "base/timeUtils.h"
#include "base/stringUtils.h"
#include "winmaincpp.h"
#include "outlook/defs.h"

#include "outlook/ClientRecurrence.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

using namespace std;

#define MAX_OCCURRENCES                     1000

// Constructor:
ClientRecurrence::ClientRecurrence() {

    pRec         = NULL;
    start        = EMPTY_WSTRING;
    isAllDay     = 1;
    
    startTime    = EMPTY_WSTRING;
    endTime      = EMPTY_WSTRING;

    // not recurring, nor updated.
    clearRecurrence();

    numRecursions = 0;

    hasTimezone = false;
}


// Destructor
ClientRecurrence::~ClientRecurrence() {

    appExceptions.clear();

    if (pRec) { pRec.Release(); }
}



/**
 * Set a COM pointer to this object.
 ************************************
 * This method is used to link the object to the correspondent
 * outlook COM pointer.
 * The method MUST be called before using this object, as the constructor
 * doesn't link the class COM pointer.
 *
 * @note RecPatternPtr could be passed NULL (if new item), so check it
 *       before using it (read() and save()).
 */
void ClientRecurrence::setCOMPtr(RecurrencePatternPtr& ptr) {

    pRec = ptr;

    appExceptions.clear();
    numRecursions = 0;

    // We just linked a different rec pattern, 
    // current members are not updated!
    isUpdated = false;
}





//Return true if the rec pattern is active.
bool ClientRecurrence::isRecurring() {
    return recurring;
}


// Set the recurrence (recurring = true).
void ClientRecurrence::setRecurrence() {
    recurring = true;
}


// Clear the recurrence (recurring = false).
void ClientRecurrence::clearRecurrence() {
    
    recurring = false;
    isUpdated = false;

    // initialize (these values indicate that props are not set)
    recurrenceType   = -1;
    interval         = -1;
    monthOfYear      = -1;
    dayOfMonth       = -1;
    dayOfWeekMask    = -1;
    instance         = -1;
    patternStartDate = EMPTY_WSTRING;
    noEndDate        = FALSE;
    patternEndDate   = EMPTY_WSTRING;
    occurrences      = -1;

    startTime        = EMPTY_WSTRING;
    endTime          = EMPTY_WSTRING;

}



/////////////////////////////////////////////////////////////////////////
/**
 * Retrieve all properties from Outlook -> set isUpdated = true.
 * If UTC is used, all props are converted to correct values.
 * For the Task remember the StartTime and EndTime are not valid properties
 * so the timezone is not considered and the PatternStartDate and PatterEndDate
 * are in the format yyyy-mm-dd according with the startdate and due date
 * 
 * @return: 0 if no errors
 */
int ClientRecurrence::read() {

    DATE patternEnd;
    VARIANT_BOOL vNoEnd;
    bool isAppointmentRecurrence = true;
    DATE patternStartTime;
    DATE patternEndTime;

    // Check COM Ptr / Recurrence active
    if (!pRec || !isRecurring()) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_NOT_SET, L"read");
        throwClientException(getLastErrorMsg());
        return 1;
    }



    //
    // ---------- Get ALL ------------
    //
    // TBD: add another try/catch (more specific)
    recurrenceType = pRec->GetRecurrenceType();
    interval       = pRec->GetInterval();
    monthOfYear    = pRec->GetMonthOfYear(); 
    dayOfMonth     = pRec->GetDayOfMonth();
    dayOfWeekMask  = pRec->GetDayOfWeekMask();
    instance       = pRec->GetInstance();
    vNoEnd         = pRec->GetNoEndDate();
    patternEnd     = pRec->GetPatternEndDate();
    occurrences    = pRec->GetOccurrences();

    patternStartTime = pRec->GetStartTime(); 
    patternEndTime   = pRec->GetEndTime();   

    

    //
    // Get patternStart from start (use exactly the same - new from 6.0.4).
    //
    patternStartDate = EMPTY_WSTRING;
    if (start != EMPTY_WSTRING) {
        patternStartDate = start;
    }
    else {
        // If 'Start' is empty, use the patternStartDate...
        DATE patternStart = pRec->GetPatternStartDate();
        if (patternStart < LIMIT_MAX_DATE) {
            doubleToSystemTime(patternStartDate, patternStart, FALSE, true);
        }
    }

     // set the startTime of the appointment
    if (patternStartTime < LIMIT_MAX_DATE) {
        doubleToSystemTime(startTime, patternStartTime, FALSE, false);
    }

    // set the endTime of the appointment
    if (patternEndTime < LIMIT_MAX_DATE) {
        doubleToSystemTime(endTime, patternEndTime, FALSE, false);
    }
    
    if (patternStartTime > LIMIT_MAX_DATE && patternEndTime > LIMIT_MAX_DATE) {
        isAppointmentRecurrence = false; // this recurrence is for a task
    }

    //
    // --------- Conversions ---------
    //
            
    // Outlook APIs bug!
    // yearly OR yearNth, interval returned is 12, 24, 36 instead of 1, 2, 3...
    if (recurrenceType == 5 || recurrenceType == 6) {
        if ((interval >= 12) && (interval%12 == 0)) {
            interval = interval / 12;
        }
    }
    // For compatibility: weekly with interval = 0 is not correct...
    if ((recurrenceType == 1) && (interval == 0)) {
        interval = 1;
    }

    noEndDate = vBoolToBOOL(vNoEnd);

    if (patternEnd < LIMIT_MAX_DATE) {
        // Fix for PatternEndDate (since v.6.5.3):
        // We need the exact date+time when the last occurrence starts, so we get the
        // time (hours) from startDate and add it to the patternEnd midnight.
        DATE startDate = NULL;
        systemTimeToDouble(start, &startDate);
        if (startDate) {
            double hours = startDate - (int)startDate;
            patternEnd = (int)patternEnd + hours;
        }
        if (isAppointmentRecurrence) {
            doubleToSystemTime(patternEndDate, patternEnd, USE_UTC, false);     // "YYYYMMDDThhmmssZ" - new from 6.0.4.
        } else {
            doubleToSystemTime(patternEndDate, patternEnd, FALSE, true); 
        }
    }
    else {
        patternEndDate = EMPTY_WSTRING;
    }

    // for compatibility
    if (noEndDate) {
        occurrences = 0;
    }

    ClientApplication* cp = ClientApplication::getInstance();
    if (cp->getOutgoingTimezone() && isAppointmentRecurrence) {
        
         // If 'Start' is empty, use the patternStartDate...
        DATE patternStart = pRec->GetPatternStartDate();
        if (patternStart < LIMIT_MAX_DATE) {
            doubleToSystemTime(patternStartDate, patternStart, FALSE, true);
        }

        // "PatternStartDate" is composed by PatternStartDate + StartTime
        if (patternStartDate.size() >= 8 && startTime.size() >= 15) {
            patternStartDate  = patternStartDate.substr(0, 8);      // the start date
            patternStartDate += TEXT("T");
            patternStartDate += startTime.substr(9, 6);   // the start time
        }
        
        if (patternEnd < LIMIT_MAX_DATE) {
            // We need the local time!
            DATE tmpDate = 0;
            systemTimeToDouble(patternEndDate, &tmpDate);       // This converts to localtime if it was in UTC
            doubleToSystemTime(patternEndDate, tmpDate, FALSE); // Now patternEndDate string is in localtime

            // "PatternEndDate" is composed by PatternEndDate + StartTime 
            // (note: StartTime, not EndTime!)
            if (patternEndDate.size() >= 8 && startTime.size() >= 15) {
                patternEndDate = patternEndDate.substr(0, 8);        // the end date
                patternEndDate += TEXT("T");
                patternEndDate += startTime.substr(9, 6);  // the start time
            }
        }
    } 
    // OLD WAY
    else if ( USE_CHANGE_DAY 
         && !isAllDay
         && USE_UTC ) {

        // CHANGE-DAY to UTC: modify monthOfYear / dayOfMonth / dayOfWeekMask.
        // All-day event and tasks don't need conversions.
        
        changeDay(L"UTC");
    }

    //
    // Read all exceptions.
    //
    appExceptions.clear();
    numRecursions = 0;
    int numExceptions = getExceptionsCountOnClient();
    ClientAppException* exc = NULL;
    for (int i=0; i<numExceptions; i++) {
        exc = getExceptionOnClient(i);
        if (exc) {
            appExceptions.push_back(*exc);
            delete exc;
            exc = NULL;
        }
    }


    isUpdated = true;
    return 0;
}

/**
 * Retrieve all properties from Outlook -> set isUpdated = true.
 * If UTC is used, all props are converted to correct values.
 * For the Task remember the StartTime and EndTime are not valid properties
 * so the timezone is not considered and the PatternStartDate and PatterEndDate
 * are in the format yyyy-mm-dd according with the startdate and due date
 * 
 * @return: 0 if no errors
 */
int ClientRecurrence::refresh() {
    
    DATE patternEnd;
    VARIANT_BOOL vNoEnd;
    bool isAppointmentRecurrence = true;
    DATE patternStartTime;
    DATE patternEndTime;

    // Check COM Ptr / Recurrence active
    if (!pRec || !isRecurring()) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_NOT_SET, L"read");
        throwClientException(getLastErrorMsg());
        return 1;
    }



    //
    // ---------- Get ALL ------------
    //
    // TBD: add another try/catch (more specific)
    recurrenceType = pRec->GetRecurrenceType();
    interval       = pRec->GetInterval();
    monthOfYear    = pRec->GetMonthOfYear(); 
    dayOfMonth     = pRec->GetDayOfMonth();
    dayOfWeekMask  = pRec->GetDayOfWeekMask();
    instance       = pRec->GetInstance();
    vNoEnd         = pRec->GetNoEndDate();
    patternEnd     = pRec->GetPatternEndDate();
    occurrences    = pRec->GetOccurrences();
    
    patternStartTime = pRec->GetStartTime(); 
    patternEndTime   = pRec->GetEndTime();   
    
    

    //
    // Get patternStart from start (use exactly the same - new from 6.0.4).
    //
    patternStartDate = EMPTY_WSTRING;
    if (start != EMPTY_WSTRING) {
        patternStartDate = start;
    }
    else {
        // If 'Start' is empty, use the patternStartDate...
        DATE patternStart = pRec->GetPatternStartDate();
        if (patternStart < LIMIT_MAX_DATE) {
            doubleToSystemTime(patternStartDate, patternStart, FALSE, true);
        }
    }
             
     // set the startTime of the appointment
    if (patternStartTime < LIMIT_MAX_DATE) {
        doubleToSystemTime(startTime, patternStartTime, FALSE, false);
    }
    
    // set the endTime of the appointment
    if (patternEndTime < LIMIT_MAX_DATE) {
        doubleToSystemTime(endTime, patternEndTime, FALSE, false);
    }
    
    if (patternStartTime > LIMIT_MAX_DATE && patternEndTime > LIMIT_MAX_DATE) {
        isAppointmentRecurrence = false; // this recurrence is for a task
    }
    //
    // --------- Conversions ---------
    //
    if (recurrenceType == 5) {
        // Yearly recurrence.
        if ((interval > 12) && (interval%12 == 0)) {
            // It's a monthly recurrence, like "every 24 months". 
            // Outlook bug, fix: recType 5 -> recType 2.
            recurrenceType = 2;
        }
        else {
            // It's the normal "yearly" recurrence.
            // Outlook bug, fix: interval 12 -> interval 1.
            interval = 1;
        }
    }
    // For compatibility: weekly with interval = 0 is not correct...
    if ((recurrenceType == 1) && (interval == 0)) {
        interval = 1;
    }

    noEndDate = vBoolToBOOL(vNoEnd);

    if (patternEnd < LIMIT_MAX_DATE) {
        // Fix for PatternEndDate (since v.6.5.3):
        // We need the exact date+time when the last occurrence starts, so we get the
        // time (hours) from startDate and add it to the patternEnd midnight.
        DATE startDate = NULL;
        systemTimeToDouble(start, &startDate);
        if (startDate) {
            double hours = startDate - (int)startDate;
            patternEnd = (int)patternEnd + hours;
        }
        if (isAppointmentRecurrence) {
            doubleToSystemTime(patternEndDate, patternEnd, USE_UTC, false);     // "YYYYMMDDThhmmssZ" - new from 6.0.4.
        } else {
            doubleToSystemTime(patternEndDate, patternEnd, FALSE, true); 
        }
    }
    else {
        patternEndDate = EMPTY_WSTRING;
    }

    // for compatibility
    if (noEndDate) {
        occurrences = 0;
    }
    
    ClientApplication* cp = ClientApplication::getInstance();
    if (cp->getOutgoingTimezone() && isAppointmentRecurrence) {
        
         // If 'Start' is empty, use the patternStartDate...
        DATE patternStart = pRec->GetPatternStartDate();
        if (patternStart < LIMIT_MAX_DATE) {
            doubleToSystemTime(patternStartDate, patternStart, FALSE, true);
        }

        // "PatternStartDate" is composed by PatternStartDate + StartTime
        if (patternStartDate.size() >= 8 && startTime.size() >= 15) {
            patternStartDate  = patternStartDate.substr(0, 8);      // the start date
            patternStartDate += TEXT("T");
            patternStartDate += startTime.substr(9, 6);   // the start time
        }
        
        if (patternEnd < LIMIT_MAX_DATE) {
            // We need the local time!
            DATE tmpDate = 0;
            systemTimeToDouble(patternEndDate, &tmpDate);       // This converts to localtime if it was in UTC
            doubleToSystemTime(patternEndDate, tmpDate, FALSE); // Now patternEndDate string is in localtime

            // "PatternEndDate" is composed by PatternEndDate + StartTime 
            // (note: StartTime, not EndTime!)
            if (patternEndDate.size() >= 8 && startTime.size() >= 15) {
                patternEndDate = patternEndDate.substr(0, 8);        // the end date
                patternEndDate += TEXT("T");
                patternEndDate += startTime.substr(9, 6);  // the start time
            }
        }
    } 
    // OLD WAY
    else if ( USE_CHANGE_DAY 
         && !isAllDay
         && USE_UTC ) {

        // CHANGE-DAY to UTC: modify monthOfYear / dayOfMonth / dayOfWeekMask.
        // All-day event and tasks don't need conversions.
        
        changeDay(L"UTC");
    }

    //
    // Read all exceptions.
    //
/*    appExceptions.clear();
    numRecursions = 0;
    int numExceptions = getExceptionsCountOnClient();
    ClientAppException* exc = NULL;
    for (int i=0; i<numExceptions; i++) {
        exc = getExceptionOnClient(i);
        if (exc) {
            appExceptions.push_back(*exc);
            delete exc;
            exc = NULL;
        }
    }
*/

    isUpdated = true;
    return 0;
}

/**
* It converts the string time and the timezone information into a local time.
* The server could send the data in local time when the timezone is specified.
* At least for vcalendar format
*/
wstring getStringDateWithTz(TIME_ZONE_INFORMATION& t, wstring start) {

    wstring patternStartDate;
    DATE d;
    SYSTEMTIME st, local;
    stringTimeToDouble(start, &d);
    VariantTimeToSystemTime(d, &st);
    SystemTimeToTzSpecificLocalTime(&t, &st, &local);   
    SystemTimeToVariantTime(&local, &d);
    doubleToStringTime(patternStartDate, d);
    return patternStartDate;
}
/*
 * Save all the properties to Outlook.
 * Properties are verified to be consistent all together, 
 * as Outlook doesn't accept wrong values.
 * If UTC is used, all props are converted to correct values.
 * 
 * The "getHasTimezone()" info decides if uses the new method that takes
 * care about the timezone or the old one.
 *
 * @return: 0 if no errors

 */
int ClientRecurrence::save() {

    HRESULT hr;
    DATE patternStart = 0, patternEnd = 0;
    int i = -1;    // index for 'recurrenceProps' array.

    // Check COM Ptr / Recurrence active
    if (!pRec || !isRecurring()) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_NOT_SET, L"save");
        throwClientException(getLastErrorMsg());
        return 1;
    }

    // This is required correct!
    if (recurrenceType < 0 || recurrenceType > 6) {
        i=0;
        goto error;
    }


    // TBD: check consistance of props for this recType...
    checkIfRecIsCorrect();
    
    // The boolean need to take care about the appointment synced from a 7.0.x client
    // against a server 6.5.x that doesn't support the timezone.
    // the data were store with local time
    bool useLocal = false;
    if ((patternStartDate != EMPTY_WSTRING) && 
            patternStartDate.find(TEXT("Z")) == wstring::npos) {
        useLocal = true;
    }
    //
    // --------- Conversions ---------
    //
    if (interval < 1) {
        interval = 1;
    }
    
    if (getHasTimezone() || useLocal) {
        
        if (patternStartDate == EMPTY_WSTRING) {              
            if (start.find(TEXT("Z")) != wstring::npos) {
                TIME_ZONE_INFORMATION t = getRecurringTimezone();
                patternStartDate = getStringDateWithTz(t, start);                
            } else {
                patternStartDate = start;
            }
        }
                
        replaceAll(L"-", L"", patternStartDate); // Safe: to accept SIF format "yyyy-mm-dd" too.
        if (patternStartDate.size() >= 15) {
            startTime  = TEXT("18991230T");      // stands for the date = 0
            startTime += patternStartDate.substr(9, 6);
        }
        if (patternStartDate.size() >= 8) {
            patternStartDate     = patternStartDate.substr(0, 8);
            patternStartDate    += TEXT("T000000");
        }
        
        if (patternEndDate.size() >= 8) {
            replaceAll(L"-", L"", patternEndDate);        // Safe: to accept SIF format "yyyy-mm-dd" too.
            patternEndDate  = patternEndDate.substr(0, 8);
            patternEndDate += TEXT("T000000");
        }
        
        // ---- EndTime ----
        // It's now mandatory, if we set the startTime. We have to calculate it.
        if (startTime.size() > 0) {
            // "Duration" is constant, can be retrieved from (Start - End)
            DATE startDate, endDate, startTimeDate;
            stringTimeToDouble(start,     &startDate);                      // not converting to local time
            stringTimeToDouble(end,       &endDate);
            stringTimeToDouble(startTime, &startTimeDate);

            DATE endTimeDate = startTimeDate + (endDate - startDate);
            doubleToSystemTime(endTime, endTimeDate, FALSE, FALSE);     // not converting to UTC
            
        }
        systemTimeToDouble(patternStartDate, &patternStart, true);
        systemTimeToDouble(patternEndDate,   &patternEnd,   true);

    } else { // old method

        //
        // PatternStartDate: get value from <Start> if not empty.
        // (ignore the passed value and overwrite it with start - new from 6.0.4)
        if (start != EMPTY_WSTRING) {
            patternStartDate = start;
        }
        //systemTimeToDouble(patternStartDate, &patternStart, true);      // Convert to local-time and force time to 00:00.
                                                                        // (MUST put 00:00 to Outlook!)
        systemTimeToDouble(patternStartDate, &patternStart);            // As above but first convert to local if needed
        patternStart = (long)patternStart;
        
        //
        // PatternEndDate: UTC value is expected, but accept also "yyyyMMdd"
        //
        systemTimeToDouble(patternEndDate, &patternEnd);
        if (patternEnd)
            patternEnd -= .5; // Timezone fix


        // CHANGE-DAY to Local: modify monthOfYear / dayOfMonth / dayOfWeekMask.
        // All-day event and tasks don't need conversions.
        if ( USE_CHANGE_DAY 
             && !isAllDay
             && USE_UTC ) {

            changeDay(L"Local");
        }

    }


    //
    // -----------Put ALL ------------
    // (note that this order is mandatory to put props)
    //
    i=0;
    hr = pRec->put_RecurrenceType((OlRecurrenceType)recurrenceType);
    if (FAILED(hr)) goto error;
    
    i++;
    hr = pRec->put_Interval((long)interval);
    if (FAILED(hr)) goto error;
    
    i++; 
    // Ignore if '0' (default) or '-1' (tag not found)
    if (monthOfYear != 0 && monthOfYear != -1) {
        hr = pRec->put_MonthOfYear((long)monthOfYear);
        if (FAILED(hr)) goto error;
    }
    
    i++;
    // Ignore if '0' (default) or '-1' (tag not found)
    if (dayOfMonth != 0 && dayOfMonth != -1) {
        hr = pRec->put_DayOfMonth((long)dayOfMonth);
        if (FAILED(hr)) goto error;
    }
    
    i++;
    // Ignore if '0' (default) or '-1' (tag not found)
    if (dayOfWeekMask != 0 && dayOfWeekMask != -1) {
        hr = pRec->put_DayOfWeekMask((OlDaysOfWeek)dayOfWeekMask);
        if (FAILED(hr)) goto error;
    }
    
    i++;
    // Ignore if '0' (default) or '-1' (tag not found)
    if (instance != 0 && instance != -1) {
        hr = pRec->put_Instance((long)instance);
        if (FAILED(hr)) goto error;
    }
    
    i++;
    if (patternStart < LIMIT_MAX_DATE) {
        // Outlook wants PatternStartDate with time = "00:00".
        patternStart = (int)patternStart;
        hr = pRec->put_PatternStartDate(patternStart);
        if (FAILED(hr)) goto error;
    }
  
    i++;

    if (getHasTimezone() || useLocal) {
        DATE sTime;
        stringTimeToDouble(startTime, &sTime);            
        if (sTime < LIMIT_MAX_DATE) {
            hr = pRec->put_StartTime(sTime);
            if (FAILED(hr)) goto error;
        }
        i++;
    }

    hr = pRec->put_NoEndDate(BOOLToVBool(noEndDate));
    if (FAILED(hr)) goto error;


    i++;
    // Ignore if '0' (default) or '-1' (tag not found)
    // Note: set "patternEndDate" only if "occurrences" not received or wrong.
    // ----- (ignore patternEndDate if values disagree)
    if (occurrences != 0 && occurrences != -1) {
        i++;
        if (occurrences >= MAX_OCCURRENCES) {
            hr = pRec->put_NoEndDate(BOOLToVBool(true)); //Just set it to no end date.
        } else {
            hr = pRec->put_Occurrences((long)occurrences);
        }
        if (FAILED(hr)) goto error;
    }
    else {
        if (patternEnd && patternEnd < LIMIT_MAX_DATE) {
            hr = pRec->put_PatternEndDate(patternEnd);
            if (FAILED(hr)) goto error;
        }
        i++;
        if (getHasTimezone() || useLocal) {
            // only if the Timezone is used
            DATE eTime;
            stringTimeToDouble(endTime, &eTime);            
            if (eTime < LIMIT_MAX_DATE) {
                hr = pRec->put_EndTime(eTime);
                if (FAILED(hr)) goto error;
            }
            i++;
        }

    }

    isUpdated = true;
    return 0;

error:
    isUpdated = false;
    if (i>=0 && recurrenceProps[i]) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_SAVE, recurrenceProps[i]);
        LOG.error(getLastErrorMsg());

        // Add some verbose logging, since recurrence issues are quite common
        LOG.debug("List of Outlook recurrence properties that caused the error:");
        LOG.debug("  * RecurrenceType  : %d", recurrenceType);
        LOG.debug("  * Interval        : %d", interval);
        LOG.debug("  * MonthOfYear     : %d", monthOfYear);
        LOG.debug("  * DayOfMonth      : %d", dayOfMonth); 
        LOG.debug("  * DayOfWeekMask   : %d", dayOfWeekMask); 
        LOG.debug("  * Instance        : %d", instance); 
        LOG.debug("  * PatternStartDate: %ls (%lf)", patternStartDate.c_str(), patternStart); 
        LOG.debug("  * StartTime       : %ls", startTime.c_str());
        LOG.debug("  * NoEndDate       : %s", noEndDate? "true":"false");
        LOG.debug("  * Occurrences     : %d", occurrences);
        LOG.debug("  * PatternEndDate  : %ls (%lf)", patternEndDate.c_str(), patternEnd);
        LOG.debug("  * EndTime         : %ls", endTime.c_str());
        LOG.debug("  * (hasTimezone)   : %s", getHasTimezone()? "true":"false");
        LOG.debug("  * (useLocal)      : %s", useLocal? "true":"false");

        throwClientException(getLastErrorMsg());
    }
    return 1;
}
///////////////////////////////////////////////////////////////////////////



void ClientRecurrence::checkIfRecIsCorrect() {
    //
    // TBD: verify if some props are not correct for the rec type.
    // (display warning?)
    // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vbaol11/html/olobjRecurrencePattern_HV05247371.asp
    //
}



/*
 * Change-day option: if USE_CHANGE_DAY is true, rec pattern props
 * are changed according to Start value and USE_UTC.
 * Returns true if change-day is applied
 */
bool ClientRecurrence::changeDay(const wstring dest) {
    
    DATE date = 0;
    SYSTEMTIME time, localTime, utcTime;

    // This call gets 'date' ALWAYS in local time...
    systemTimeToDouble     (start, &date);
    VariantTimeToSystemTime(date,  &time);

    localTime = time;
    localTimeToUTC(time);
    utcTime   = time;


    // Day to be changed!
    if (localTime.wDay != utcTime.wDay) {
        if (dest == L"Local") {
            time = localTime;
        }

        //
        // <MonthOfYear>: get from <Start>
        //
        if (monthOfYear != 0 && monthOfYear != -1) {
            monthOfYear = time.wMonth;
        }

        //
        // <DayOfMonth>: get from <Start>
        //
        if (dayOfMonth != 0 && dayOfMonth != -1) {
            dayOfMonth = time.wDay;
        }

        //
        // <DayOfWeekMask>: shift the mask
        //
        if (dayOfWeekMask != 0 && dayOfWeekMask != -1) {
            
            // Calculate if forward or backward 1 day.
            TIME_ZONE_INFORMATION tmz;
            GetTimeZoneInformation(&tmz);
            bool forward;
            if (dest == L"Local") {
                if (tmz.Bias < 0) forward = true;
                else              forward = false;
            }
            else {
                if (tmz.Bias < 0) forward = false;
                else              forward = true;
            }

            // Forward = left shift.
            if (forward) {
                dayOfWeekMask = dayOfWeekMask << 1;
                if (dayOfWeekMask > 127) {
                    dayOfWeekMask -= 127;
                }
            }
            // Backward = right shift.
            else {
                if (dayOfWeekMask & 1) {
                    dayOfWeekMask += 127;
                }
                dayOfWeekMask = dayOfWeekMask >> 1;
            }
        }
        return true;
    }
    return false;
}



/**
 * Return the property value (wstring) from its name.
 * Properties are retrieved from internal members (switch to correct property).
 * Rec pattern values must be first retrieved all together, this is done
 * internally when the first property is retrieved.
 *
 * @param propertyName  : the name  of the property
 * @return              : the value retrieved as wstring
 */
const wstring ClientRecurrence::getProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    WCHAR tmp[64];

    if (propertyName == L"RecurrenceType") {
        wsprintf(tmp, TEXT("%d"), getRecurrenceType());
        propertyValue = tmp;
    }
    else if (propertyName == L"Interval") {
        wsprintf(tmp, TEXT("%d"), getInterval());
        propertyValue = tmp;
    }
    else if (propertyName == L"MonthOfYear") {                  // Set EMPTY if = 0 ?
        wsprintf(tmp, TEXT("%d"), getMonthOfYear());
        propertyValue = tmp;
    }
    else if (propertyName == L"DayOfMonth") {                   // Set EMPTY if = 0 ?
        wsprintf(tmp, TEXT("%d"), getDayOfMonth());
        propertyValue = tmp;
    }
    else if (propertyName == L"DayOfWeekMask") {                // Set EMPTY if = 0 ?
        wsprintf(tmp, TEXT("%d"), getDayOfWeekMask());
        propertyValue = tmp;
    }
    else if (propertyName == L"Instance") {
        wsprintf(tmp, TEXT("%d"), getInstance());
        propertyValue = tmp;
    }
    else if (propertyName == L"PatternStartDate") {
        propertyValue = getPatternStartDate();
    }
    else if (propertyName == L"NoEndDate") {
        wsprintf(tmp, TEXT("%d"), getNoEndDate());
        propertyValue = tmp;
    }
    else if (propertyName == L"PatternEndDate") {
        propertyValue = getPatternEndDate();
    }
    else if (propertyName == L"Occurrences") {
        int occ = getOccurrences();
        if (occ > 0) {
            wsprintf(tmp, TEXT("%d"), occ);
            propertyValue = tmp;
        }
        else {
            // for compatibility
            propertyValue = EMPTY_WSTRING;
        }
    }
    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_PROP_NOT_FOUND, propertyName.c_str());
        throwClientException(getLastErrorMsg());
        return EMPTY_WSTRING;
    }

    return propertyValue;
}



/**
 * Set property value from its name: set internal object values.
 * All rec pattern must be saved together at the end calling 'save()'
 * method. This is because:
 *  - rec props MUST be verified consistent each others
 *  - if UTC is used, rec props are first converted inside recPattern.
 *
 * @param propertyName  : the name of the property
 * @param propertyValue : the value to store
 * @return              : 0 if no errors, 1 if errors
 */
int ClientRecurrence::setProperty(const wstring& propertyName, const wstring& propertyValue) {
    int intValue = 0;

    if (propertyName == L"RecurrenceType") {
        intValue = _wtoi(propertyValue.c_str());
        setRecurrenceType(intValue);
    }
    else if (propertyName == L"Interval") {
        intValue = _wtoi(propertyValue.c_str());
        setInterval(intValue);
    }
    else if (propertyName == L"MonthOfYear") {
        intValue = _wtoi(propertyValue.c_str());
        setMonthOfYear(intValue);
    }
    else if (propertyName == L"DayOfMonth") {
        intValue = _wtoi(propertyValue.c_str());
        setDayOfMonth(intValue);
    }
    else if (propertyName == L"DayOfWeekMask") {
        intValue = _wtoi(propertyValue.c_str());
        setDayOfWeekMask(intValue);
    }
    else if (propertyName == L"Instance") {
        intValue = _wtoi(propertyValue.c_str());
        setInstance(intValue);
    }
    else if (propertyName == L"PatternStartDate") {
        setPatternStartDate(propertyValue);
    }
    else if (propertyName == L"NoEndDate") {
        BOOL bValue = _wtoi(propertyValue.c_str());
        setNoEndDate(bValue);
    }
    else if (propertyName == L"PatternEndDate") {
        setPatternEndDate(propertyValue);
    }
    else if (propertyName == L"Occurrences") {
        intValue = _wtoi(propertyValue.c_str());
        setOccurrences(intValue);
    }
    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_REC_PROP_NOT_FOUND, propertyName.c_str());
        throwClientException(getLastErrorMsg());
        return 1;
    }
    return 0;
}





/*
 * Methods to get/set internal members.
 * -------------------------------------------------------
 */
const int ClientRecurrence::getRecurrenceType() {
    if (!isUpdated) {
        read();
    }
    return recurrenceType;
}
void ClientRecurrence::setRecurrenceType(const int val) {
    recurrenceType = val;
    isUpdated = false;
}

const int ClientRecurrence::getInterval() {
    if (!isUpdated) {
        read();
    }
    return interval;
}
void ClientRecurrence::setInterval(const int val) {
    interval = val;
    isUpdated = false;
}

const int ClientRecurrence::getMonthOfYear() {
    if (!isUpdated) {
        read();
    }
    return monthOfYear;
}
void ClientRecurrence::setMonthOfYear(const int val) {
    monthOfYear = val;
    isUpdated = false;
}

const int ClientRecurrence::getDayOfMonth() {
    if (!isUpdated) {
        read();
    }
    return dayOfMonth;
}
void ClientRecurrence::setDayOfMonth(const int val) {
    dayOfMonth = val;
    isUpdated = false;
}

const int ClientRecurrence::getDayOfWeekMask() {
    if (!isUpdated) {
        read();
    }
    return dayOfWeekMask;
}
void ClientRecurrence::setDayOfWeekMask(const int val) {
    dayOfWeekMask = val;
    isUpdated = false;
}

const int ClientRecurrence::getInstance() {
    if (!isUpdated) {
        read();
    }
    return instance;
}
void ClientRecurrence::setInstance(const int val) {
    instance = val;
    isUpdated = false;
}

const wstring& ClientRecurrence::getPatternStartDate() {
    if (!isUpdated) {
        read();
    }
    return patternStartDate;
}
void ClientRecurrence::setPatternStartDate(const wstring& val) {
    patternStartDate = val;
    isUpdated = false;
}

const BOOL ClientRecurrence::getNoEndDate() {
    if (!isUpdated) {
        read();
    }
    return noEndDate;
}
void ClientRecurrence::setNoEndDate(const BOOL val) {
    noEndDate = val;
    isUpdated = false;
}

const wstring& ClientRecurrence::getPatternEndDate() {
    if (!isUpdated) {
        read();
    }
    return patternEndDate;
}
void ClientRecurrence::setPatternEndDate(const wstring& val) {
    patternEndDate = val;
    isUpdated = false;
}

const int ClientRecurrence::getOccurrences() {
    if (!isUpdated) {
        read();
    }
    return occurrences;
}
void ClientRecurrence::setOccurrences(const int val) {
    occurrences = val;
    isUpdated = false;
}


void ClientRecurrence::setStart(const wstring& val) {
    start = val;
}
void ClientRecurrence::setIsAllDay(const BOOL val) {
    isAllDay = val;
}
void ClientRecurrence::setEnd(const wstring& val) {
    end = val;
}
    


// ------------------------- Methods to manage Appointment Exceptions -----------------------------


/**
 * Returns the number of exceptions stored in this recurrence.
 */
const int ClientRecurrence::getExceptionsCount() {
    if (!isUpdated) {
        read();
    }
    return appExceptions.size();
}


/**
 * Returns the desired ClientAppException stored in 'appExceptions' internal list.
 * @param index : the index of exception desired
 * @return      : the ClientAppException pointer (to internal item, MUST NOT free it)
 */
ClientAppException* ClientRecurrence::getException(const int index) {

    if (index<0 || (unsigned int)index>=appExceptions.size()) {
        return NULL;
    }

    clientExceptionIterator it = appExceptions.begin();
    for (int i=0; i<index; i++) {
        it++;
    }
    return &(*it);
}


/**
 * Adds a ClientAppException into 'appExceptions' internal list.
 * The object passed is copied, so can be safely deleted by the caller.
 * @param cException : the ClientAppException* to add
 * @return           : the number of exceptions stored after the add operation
 */
int ClientRecurrence::addException(ClientAppException* cException) {

    if (cException) {
        appExceptions.push_back(*cException);
    }
    return appExceptions.size();
}


/**
 * Reset the exception list 'appExceptions'.
 */
void ClientRecurrence::resetExceptions() {
    appExceptions.clear();
}



/**
 * This utility method is used to remove all duplicated exceptions that are
 * extracted from Outlook.
 * We don't want more than one exception with the same "OriginalDate" (it's nonsense...).
 * Outlook may send more than one, for example:
 *
 *  <Exception>
 *    <OriginalDate>2007-01-15</OriginalDate>                                              
 *    <ExAppointment>
 *        <ExSubject>Subject modified</ExSubject>          
 *        <ExBody/>         
 *        <ExLocation/>  
 *        <ExStart>2007-01-16</ExStart>          
 *        <ExEnd>2007-01-16</ExEnd>      
 *        <ExAllDayEvent>1</ExAllDayEvent>
 *        <ExBusyStatus>1</ExBusyStatus> 
 *    </ExAppointment>    
 *  </Exception>
 *  <Exception>
 *    <OriginalDate>2007-01-15</OriginalDate>  
 *  </Exception>
 *
 * In this case, the second exception is nonsense, and will be removed from list.
 * 
 * @return : the number of exceptions removed from list.
 */
int ClientRecurrence::removeDuplicatedExceptions() {

    if (!appExceptions.size()) {
        return 0;
    }

    bool restart = false;
    int numRemoved = 0;
    clientExceptionIterator i = appExceptions.begin();

    //
    // Cycle through all exceptions
    //
    while (i != appExceptions.end()) {
        // Jump normal exceptions
        if ( (*i).getDeleted() == FALSE ) {
            i++;
            continue;
        }

        // Search for another exception with the same "OriginalDate"
        clientExceptionIterator j = i;
        j++;
        while (j != appExceptions.end()) {
            if ( (int)((*i).getOriginalDate()) == (int)((*j).getOriginalDate()) ) {
                // Found! -> remove the one wich is a deleted occurrence.
                appExceptions.erase(i);
                numRemoved ++;
                restart = true;
                break;
            }
            j++;
        }

        if (restart) {
            // Need to restart again, "i" is not valid now.
            restart = false;
            i = appExceptions.begin();
            continue;
        }
        i++;
    }

    return numRemoved;
}





/**
 * Returns the number of appointment exceptions.
 * Value is always retrieved from Outlook (not saved in this object).
 * This method is used from client to server, initially to populate
 * the exceptions list.
 */
const int ClientRecurrence::getExceptionsCountOnClient() {

    // Not recurring -> no exceptions :)
    if (!isRecurring()) {
        return 0;
    }

    int numExceptions = 0;
    try {
        ExceptionsPtr pExceptions = pRec->GetExceptions();
        if (!pExceptions) return 0;
        numExceptions = pExceptions->GetCount();
    }
    catch (_com_error &e) {
        manageComErrors(e);
        return 0;
    }

    return numExceptions;
}



/**
 * Returns the ClientAppException object of the desired index [0, numExceptions-1].
 * Value is always retrieved from Outlook (not saved in this object).
 * This method is used from client to server, initially to populate
 * the exceptions list.
 * Note: the pointer returned is a new allocated object.
 *
 * @param index : the index of Exception required
 * @return      : the correspondent pointer (new allocated) of ClientAppException (NULL if not found).
 */
ClientAppException* ClientRecurrence::getExceptionOnClient(const int index) {

    // Not recurring -> no exceptions :)
    if (!isRecurring()) {
        return NULL;
    }

    ClientAppException* appException = NULL;

    try {
        ExceptionsPtr pExceptions = pRec->GetExceptions();
        if (!pExceptions) return NULL;
        int numExceptions = pExceptions->GetCount();

        if (index<0 || index>=numExceptions) return NULL;

        // Get the appointment exception.
        ExceptionPtr pException = pExceptions->Item(index+1);       // First index = 1.
        if (!pException) return NULL;

        appException = new ClientAppException();
        appException->setCOMPtr(pException);
    }

    catch (_com_error &e) {
        manageComErrors(e);
        return NULL;
    }

    return appException;
}



/**
 * Returns the specific occurrence of date "originalDate" from the recurrence pattern. 
 * Returned object is an appointement COM pointer '_AppointmentItemPtr' 
 * (each occurrence is an appointment itself). If the occurrence is not found, returns NULL.
 *
 * @param originalDate : the original date of the occurrence to be found
 *                       (double format)
 * @return             : the new COM pointer "_AppointmentItemPtr" which is the desired occurrence
 *                       (NULL if not found)
 */
_AppointmentItemPtr ClientRecurrence::getOccurrence(const DATE originalDate) {

    if (!isRecurring()) {
        return NULL;
    }
    _AppointmentItemPtr pOcc = NULL;

    try {
        pOcc = pRec->GetOccurrence(originalDate);
        if (!pOcc) {
            return NULL;        // Not found
        }
    }
    catch (_com_error&) {
        return NULL;            // Not found
    }

    return pOcc;
}



/**
 * Save all exceptions from internal exception list to Outlook (server to client).
 * All exceptions are 'deleted occurrence' exceptions, 'modified exceptions'
 * are not expected at this level (should be normalized in upper layer).
 *
 * @return : 0 if all exceptions saved correctly
 */
int ClientRecurrence::saveAllExceptions() {

    int ret = 0;
    int numExceptions = appExceptions.size();
    if (numExceptions>0) {

        // First save all exceptions which are deleted occurrence.
        clientExceptionIterator it = appExceptions.begin();
        while (it != appExceptions.end()) {
            if ( (*it).getDeleted() ) {
                saveException(&(*it));
            }
            it++;
        }

        //
        // ****** SAVE OF MODIFIED EXCEPTIONS IS DISABLED ******
        // Only deleted exceptions are currently used, modified exc
        // are transformed into separate events.
        //
        // Then save all other exceptions (safeSave).
        //numRecursions = 0;
        //it = appExceptions.begin();
        //while (it != appExceptions.end()) {
        //    ret += safeSaveException(&(*it));
        //    it++;
        //}
    }
    return ret;
}


//
// ****** SAVE OF MODIFIED EXCEPTIONS IS DISABLED ******
// Only deleted exceptions are currently used, modified exc
// are transformed into separate events.
//
/**
 * Save the exception 'cException' into Outlook.
 * First check if this exception depends on another exception of the list.
 * In this case, we first execute that exception (recursive call).
 * Than saves this exception to Outlook.
 *
 * @return 0 if no errors
 */
//int ClientRecurrence::safeSaveException(ClientAppException* cException) {
//
//    // Already saved -> nothing to do.
//    if (cException->isSaved()) {
//        return 0;
//    }
//    // Occurrence to delete -> save without problem
//    if (cException->getDeleted()) {
//        return saveException(cException);
//    }
//
//    //
//    // OriginalDate = StartDate -> save without problem
//    //
//    int thisOriginalDay = (int)cException->getOriginalDate();
//    wstring thisStart = cException->getStart();
//    DATE thisStartDate = 0;
//    if (thisStart.size() > 0) {
//        systemTimeDateToDouble(thisStart, &thisStartDate);
//    }
//    if (thisOriginalDay == (int)thisStartDate) {
//        return saveException(cException);
//    }
//
//    
//    // Check to avoid recursion deadlock...
//    if (numRecursions > ((int)appExceptions.size() + 1)) {
//        LOG.error(ERR_EXCEPTIONS_DEADLOCK, numRecursions);
//        return 1;
//    }
//
//    //
//    // Search if another exception has the OriginalDate = this startDate.
//    // In this case there's a dependence: we first save that exception.
//    //
//    clientExceptionIterator it = appExceptions.begin();
//    while (it != appExceptions.end()) {
//
//        if ( (*it).getOriginalDate() == cException->getOriginalDate() ){
//            it++;      // It's me...
//            continue;
//        }
//
//        if ((int)thisStartDate == (int)(*it).getOriginalDate()) {
//            // *** Save first that exception (recursive call!) ***
//            numRecursions++;
//            safeSaveException(&(*it));
//        }
//        it++;
//    }
//
//    // Finally save this exception
//    return saveException(cException);
//}



/**
 * Save the exception 'cException' into Outlook.
 * No check on exceptions dependences is done here.
 * Before saving the exception, we MUST check that the destination date
 * is free (the "StartDate") -> call 'freeDestinationDays()'.
 * Note: before calling this method, the Appointment Item MUST be
 *       already saved to Outlook.
 *
 * @return 0 if no errors
 */
int ClientRecurrence::saveException(ClientAppException* cException) {

    //
    // Get the occurrence (appointment item MUST be already saved)
    //
    DATE originalDate = cException->getOriginalDate();
    if (!originalDate) { 
        // Error: no originalDate
        setErrorF(getLastErrorCode(), ERR_PROPERTY_REQUIRED, L"appointment exception", L"OriginalDate");
        LOG.error(getLastErrorMsg());
        return 1;
    }

    // *** MUST correct the originalDate in case of ***
    // - occurrence deleted  (yes, only deleted now supported)
    // - not all day event
    if (!isAllDayFormat(start)) {
        wstring realOriginalDate = cException->formatOriginalDate(FALSE, start);
        systemTimeToDouble(realOriginalDate, &originalDate);
    }

    _AppointmentItemPtr pOcc = getOccurrence(originalDate);
    if (!pOcc) {
        // Occurrence not found - simply ignore.
        // setErrorF(getLastErrorCode(), ERR_OCCURRENCE_NOT_FOUND, cException->formatOriginalDate().c_str());
        // LOG.debug(getLastErrorMsg());
        return 1;
    }

    //
    // Link the appointment occurrence pointer
    //
    cException->setCOMPtr(pOcc);


    //
    // Save exception / delete occurrence.
    //
    if (cException->getDeleted()) {
        if (cException->deleteOccurrence()) {
            setErrorF(getLastErrorCode(), ERR_OCCURRENCE_NOT_DELETED, cException->formatOriginalDate().c_str());
            LOG.error(getLastErrorMsg());
            return 1;
        }
    }
    else {
        setErrorF(getLastErrorCode(), "Internal error: only 'deleted' exceptions can be saved. Trying to save a modified exception (date = %ls).", cException->formatOriginalDate().c_str());
        LOG.debug(getLastErrorMsg());
        return 1;
        //
        // ****** SAVE OF MODIFIED EXCEPTIONS IS DISABLED ******
        // Only deleted exceptions are currently used, modified exc
        // are transformed into separate events.
        //
        // Need to free the destination day (and all days between "OriginalDate" and "Start")
        //DATE destination;
        //systemTimeDateToDouble(cException->getStart(), &destination);
        //freeDestinationDays(destination, cException->getOriginalDate());

        //if (cException->saveOccurrence()) {
        //    setErrorF(getLastErrorCode(), ERR_OCCURRENCE_NOT_SAVED, cException->formatOriginalDate().c_str());
        //    LOG.error(getLastErrorMsg());
        //    return 1;
        //}
    }

    return 0;
}


//
// ****** SAVE OF MODIFIED EXCEPTIONS IS DISABLED ******
// Only deleted exceptions are currently used, modified exc
// are transformed into separate events.
// This utility method is currently not used.
//
/**
 * Delete all occurrences between "OriginalDate" and "StartDate" (if found).
 * We check all days in this interval.
 */
//void ClientRecurrence::freeDestinationDays(const DATE startDate, const DATE originalDate) {
//
//    int originalDay = (int)originalDate;
//    int startDay    = (int)startDate;
//    DATE date1, date2;
//    _AppointmentItemPtr pOcc = NULL;
//
//    // Same day: nothing to do
//    if (originalDay == startDay) {
//        return;
//    }
//
//    // Get date interval [date1 - date2] of occurrences to delete.
//    if (originalDay < startDay) {
//        date1 = originalDate + 1;
//        date2 = startDate;
//    }
//    else {
//        date1 = startDate;
//        date2 = originalDate - 1;
//    }
//
//    
//    //
//    // Delete all occurrences found between date1 and date2.
//    //
//    for (DATE i=date1; i<=date2; i++) {
//        pOcc = getOccurrence(i);
//        if (!pOcc) continue;
//
//        try {
//            HRESULT hr = pOcc->Delete();
//            if (FAILED(hr)) {
//                wstring tmp = EMPTY_WSTRING;
//                doubleToSystemTimeDate(tmp, i);
//                setErrorF(getLastErrorCode(), ERR_OCCURRENCE_NOT_DELETED, tmp);
//                LOG.error(getLastErrorMsg());
//                continue;
//            }
//        }
//        catch(_com_error &e) {
//            manageComErrors(e);
//            wstring tmp = EMPTY_WSTRING;
//            doubleToSystemTimeDate(tmp, i);
//            setErrorF(getLastErrorCode(), ERR_OCCURRENCE_NOT_DELETED, tmp);
//            LOG.error(getLastErrorMsg());
//            continue;
//        }
//    }
//}





//
// ---------------- default ones are OK ------------------
//
///* 
// * Copy Constructor
// */
//ClientRecurrence::ClientRecurrence(const ClientRecurrence& c) {
//
//    pRec             = c.pRec;
//    recurring        = c.recurring;
//    isUpdated        = c.isUpdated;
//
//    recurrenceType   = c.recurrenceType;
//    interval         = c.interval;
//    monthOfYear      = c.monthOfYear;
//    dayOfMonth       = c.dayOfMonth;
//    dayOfWeekMask    = c.dayOfWeekMask;
//    instance         = c.instance;
//    patternStartDate = c.patternStartDate;
//    noEndDate        = c.noEndDate;
//    patternEndDate   = c.patternEndDate;
//    occurrences      = c.occurrences;
//}
//
//
///* 
// * Operator =
//*/
//ClientRecurrence ClientRecurrence::operator=(const ClientRecurrence& c) {
//
//    ClientRecurrence cnew;
//
//    cnew.pRec             = c.pRec;
//    cnew.recurring        = c.recurring;
//    cnew.isUpdated        = c.isUpdated;
//
//    cnew.recurrenceType   = c.recurrenceType;
//    cnew.interval         = c.interval;
//    cnew.monthOfYear      = c.monthOfYear;
//    cnew.dayOfMonth       = c.dayOfMonth;
//    cnew.dayOfWeekMask    = c.dayOfWeekMask;
//    cnew.instance         = c.instance;
//    cnew.patternStartDate = c.patternStartDate;
//    cnew.noEndDate        = c.noEndDate;
//    cnew.patternEndDate   = c.patternEndDate;
//    cnew.occurrences      = c.occurrences;
//
//    return cnew;
//}

const double ClientRecurrence::getPatternStart()
{
    double ret;
    systemTimeDateToDouble(patternStartDate, &ret);
    return ret;
}

const double ClientRecurrence::getPatternEnd()
{
    double ret;
    systemTimeDateToDouble(patternEndDate, &ret);
    return ret;
}
