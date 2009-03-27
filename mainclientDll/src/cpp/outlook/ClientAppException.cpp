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
#include "winmaincpp.h"
#include "outlook/defs.h"
#include "SIFFields.h"

#include "outlook/ClientAppException.h"
#include "outlook/ClientAppointment.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

using namespace std;



/* 
 * Constructor:
 * Initialize class members.
*/
ClientAppException::ClientAppException() {

    pException   = NULL;
    pAppointment = NULL;

    originalDate = 0;
    deleted      = FALSE;

    subject      = EMPTY_WSTRING;
    body         = EMPTY_WSTRING;
    location     = EMPTY_WSTRING;
    start        = EMPTY_WSTRING;
    end          = EMPTY_WSTRING;
    allDayEvent  = EMPTY_WSTRING;
    busyStatus   = EMPTY_WSTRING;
    reminderSet  = EMPTY_WSTRING;
    reminderMinutesBeforeStart  = EMPTY_WSTRING;
    importance   = EMPTY_WSTRING;

    saved     = false;
    isUpdated = false;
}


// Destructor
ClientAppException::~ClientAppException() {
    if (pException)   { pException.Release();   }
    if (pAppointment) { pAppointment.Release(); }
}



/*
 * Set the Exception COM pointer of this object.
 * ---------------------------------------------
 * This method is used to link the object to the correspondent
 * outlook COM pointer. Also the Appointment exception COM pointer
 * is linked (if exist inside Exception pointer).
 * Throws a ClientException in case of errors.
 */
void ClientAppException::setCOMPtr(ExceptionPtr& ptr) {

    pException = ptr;

    try {
        // If occurrence is deleted, we don't have the appointment pointer. 
        if (pException->GetDeleted() == VARIANT_TRUE) {
            pAppointment = NULL;
            deleted = TRUE;
            return;
        }
        else {
            pAppointment = pException->GetAppointmentItem();
            if (!pAppointment) goto error;
            deleted = FALSE;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    return;

error:
    throwClientException(ERR_OUTLOOK_EXAPP_INIT);
}


/*
 * Set the Appointment COM pointer of this object.
 * -----------------------------------------------
 * This method is used to link the object to the correspondent
 * outlook COM pointer. The exception COM pointer is set to NULL.
 */
void ClientAppException::setCOMPtr(_AppointmentItemPtr& ptr) {

    pException   = NULL;
    pAppointment = ptr;
}




/**
 * Reads all properties from Outlook.
 * All data is stored into this object.
 * First gets the exception properties ("OriginalDate" and "Deleted") if the 
 * exception already exist (already saved, or reading from Outlook).
 * Then reads all occurrence properties from client (if the occurrence exists).
 * @return : 0 if no errors
 */
int ClientAppException::read() {

    HRESULT hr;
    int i=0;
    _bstr_t bstrValue;
    DATE date = NULL;
    WCHAR tmp[10];
    VARIANT_BOOL vbool;


    // Get originalDate/deleted if the exception already exists.
    if (pException) {
        try {
            originalDate = pException->GetOriginalDate();

            vbool = pException->GetDeleted();
            deleted = vBoolToBOOL(vbool);
        }
        catch(_com_error &e) {
            manageComErrors(e);
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_EXC_PROP_VALUE, L"OriginalDate");
            throwClientException(getLastErrorMsg());
            return 1;
        }
    }


    if (!pAppointment) {
        // The occurrence does not exist (yet) -> out.
        isUpdated = true;
        return 0;
    }

    try {
        //
        // *** Note ***
        // Get each property one by one, because pAppointment properties seems to be 
        // *WRONGLY* related to the pException properties!
        // So we MUST use specific methods to get each property...
        //

        // "Subject"
        i=0;
        bstrValue = pAppointment->GetSubject();
        if (bstrValue.length() > 0) {
            subject = (WCHAR*)bstrValue;
        }
        i++;

        // "Body"
        Redemption::ISafeAppointmentItemPtr pSafeAppointment;
        hr = pSafeAppointment.CreateInstance(RED_SAFEAPPOINTMENT);
        if (FAILED(hr)) {
            throwClientFatalException(ERR_OUTLOOK_SAFEITEM);
        }

        pSafeAppointment->Item = pAppointment;
        BSTR tmpVal;
        hr = pSafeAppointment->get_Body(&tmpVal);
        if (FAILED(hr)) {
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_EXC_PROP_VALUE, L"Body");
            throwClientException(getLastErrorMsg());
        }
        if (tmpVal) {
            body = tmpVal;
        }
        i++;


        // "Location"
        bstrValue = pAppointment->GetLocation();
        if (bstrValue.length() > 0) {
            location = (WCHAR*)bstrValue;
        }
        i++;


        // "Start"                                                  // All day: "yyyyMMdd"
        date = pAppointment->GetStart();                            // else   : "YYYYMMDDThhmmssZ"
        bool allDay = vBoolToBool(pAppointment->GetAllDayEvent());
        doubleToSystemTime(start, date, USE_UTC, allDay);
        i++;


        // "End"                                                    // All day: "yyyyMMdd"
        date = pAppointment->GetEnd();                              // else   : "YYYYMMDDThhmmssZ"
        doubleToSystemTime(end, date, USE_UTC, allDay);
        i++;


        // "AllDayEvent"
        vbool = pAppointment->GetAllDayEvent();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        allDayEvent = tmp;
        i++;


        // "BusyStatus"
        bstrValue = pAppointment->GetBusyStatus();
        if (bstrValue.length() > 0) {
            busyStatus = (WCHAR*)bstrValue;
        }
        i++;


        // "ReminderSet"
        vbool = pAppointment->GetReminderSet();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        reminderSet = tmp;
        i++;


        // "ReminderMinutesBeforeStart"
        bstrValue = pAppointment->GetReminderMinutesBeforeStart();
        if (bstrValue.length() > 0) {
            reminderMinutesBeforeStart = (WCHAR*)bstrValue;
        }
        i++;


        // "Importance"
        bstrValue = pAppointment->GetImportance();
        if (bstrValue.length() > 0) {
            importance = (WCHAR*)bstrValue;
        }
        i++;
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    isUpdated = true;
    return 0;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_EXC_PROP_VALUE, exAppointmentFields[i]);
    throwClientException(getLastErrorMsg());
    return 1;
}




/**
 * Saves the occurrence into Outlook. Makes sure the 'pAppointment' pointer has
 * been correctly set before calling this method.
 * All occurrence properties are retrieved from internal attributes.
 * @return : 0 if no errors
 */
int ClientAppException::saveOccurrence() {

    _bstr_t bstrValue;
    DATE date = NULL;
    int len;
    int intValue;
    VARIANT_BOOL vbool;
    int i=0;

    if (!pAppointment) {
        goto errorSave;
    }

    //
    // First put ALL occurrence properties:
    // ------------------------------------
    try {
        //
        // *** Note ***
        // Put each property one by one, because pAppointment properties seems to be 
        // *WRONGLY* related to the pException properties!
        // So we MUST use specific methods to get each property...
        //

        i=0;
        // "Subject"
        bstrValue = (_bstr_t)subject.c_str();
        pAppointment->PutSubject(bstrValue);
        i++;


        // "Body"
        bstrValue = (_bstr_t)body.c_str();
        pAppointment->PutBody(bstrValue);
        i++;


        // "Location"
        bstrValue = (_bstr_t)location.c_str();
        pAppointment->PutLocation(bstrValue);
        i++;


        // "Start"
        len = start.length();
        if (!len) {
            // this is required!
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_REQUIRED, L"Start", L"appointment exception");
            return 1;
        }
        systemTimeToDouble(start, &date);
        pAppointment->PutStart(date);
        i++;


        // "End"
        len = end.length();
         if (!len) {
            // this is required!
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_REQUIRED, L"End", L"appointment exception");
            return 1;
        } 
        systemTimeToDouble(end, &date);
        pAppointment->PutEnd(date);
        i++;


        // "AllDayEvent"
        len = allDayEvent.length();
        if (len) {
            intValue = _wtoi(allDayEvent.c_str());
            vbool = BOOLToVBool(intValue);
            pAppointment->PutAllDayEvent(vbool);                        // Put only if not empty.
        }
        i++;


        // "BusyStatus"
        len = busyStatus.length();
        if (len) {
            intValue = _wtoi(busyStatus.c_str());
            pAppointment->PutBusyStatus((OlBusyStatus)intValue);        // Put only if not empty.
        }
        i++;


        // "ReminderSet"
        len = reminderSet.length();
        if (len) {
            intValue = _wtoi(reminderSet.c_str());
            vbool = BOOLToVBool(intValue);
            pAppointment->PutReminderSet(vbool);                        // Put only if not empty.
        }
        i++;


        // "ReminderMinutesBeforeStart"
        len = reminderMinutesBeforeStart.length();
        if (len) {
            intValue = _wtoi(reminderMinutesBeforeStart.c_str());
            pAppointment->PutReminderMinutesBeforeStart(intValue);      // Put only if not empty.
        }
        i++;


        // "Importance"
        len = importance.length();
        if (len) {
            intValue = _wtoi(importance.c_str());
            pAppointment->PutImportance((OlImportance)intValue);        // Put only if not empty.
        }
        i++;

    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto errorPut;
    }


    //
    // Now save the exception occurrence:
    // ----------------------------------
    try {
        HRESULT hr = pAppointment->Save();
        if (FAILED(hr)) {
            goto errorSave;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto errorSave;
    }

    saved = true;
    return 0;


errorPut:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_EXAPP_PROP_SET, exAppointmentFields[i]);
    throwClientException(getLastErrorMsg());
    return 1;

errorSave:
    LOG.error(ERR_OUTLOOK_EXAPP_SAVE);
    return 1;
}



/**
 * Deletes the occurrence from Outlook appointment. Make sure the 'pAppointment' pointer has
 * been correctly set before calling this method.
 * @return : 0 if no errors
 */
int ClientAppException::deleteOccurrence() {

    if (!pAppointment) {
        goto error;
    }

    try {
        HRESULT hr = pAppointment->Delete();
        if (FAILED(hr)) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    saved = true;
    return 0;

error:
    LOG.error(ERR_OUTLOOK_EXAPP_DELETE);
    return 1;
}




// ------------------------ get/set of exception properties -------------------------------

const DATE ClientAppException::getOriginalDate() {
    if (!isUpdated) {
        read();
    }
    return originalDate;
}
void ClientAppException::setOriginalDate(DATE val) {
    originalDate = val;
    saved = false;
}



/**
 * Returns TRUE if this exception is deleted (occurrence deleted).
 */
const BOOL ClientAppException::getDeleted() {
    if (!isUpdated) {
        read();
    }
    return deleted;
}


void ClientAppException::setDeleted(const BOOL val) {
    deleted = val;
}


bool ClientAppException::isSaved() {
    return saved;
}



// ------------------------ get/set of occurence properties -------------------------------
/**
 * Returns the value of 'propertyName' of this exception.
 * These properties are retrieved from stored buffers (attributes of object).
 *
 * @param propertyName : the property name requested
 * @return             : the property value
 */
const wstring ClientAppException::getAppProperty(const wstring& propertyName) {

    //
    // Switch to correct method
    //
    if (propertyName == L"Subject") {
        return getSubject();
    }
    else if (propertyName == L"Body") {
        return getBody();
    }
    else if (propertyName == L"Location") {
        return getLocation();
    }
    else if (propertyName == L"Start") {
        return getStart();
    }
    else if (propertyName == L"End") {
        return getEnd();
    }
    else if (propertyName == L"AllDayEvent") {
        return getAllDayEvent();
    }
    else if (propertyName == L"BusyStatus") {
        return getBusyStatus();
    }
    else if (propertyName == L"ReminderSet") {
        return getReminderSet();
    }
    else if (propertyName == L"ReminderMinutesBeforeStart") {
        return getReminderMinutesBeforeStart();
    }
    else if (propertyName == L"Importance") {
        return getImportance();
    }
    else {
        // wrong propertyName...
        return EMPTY_WSTRING;
    }
}




/**
 * Sets the value of field 'propertyName' of this exception.
 * These properties are stored to buffers (object attributes).
 * Properties will be put to Outlook when calling 'saveOccurrence()' method.
 *
 * @param propertyName  : the property name to be set
 * @param propertyValue : the property value
 * @return              : 0 if no errors
 */
int ClientAppException::setAppProperty(const wstring& propertyName, const wstring& propertyValue) {

    //
    // Switch to correct method
    //
    if (propertyName == L"Subject") {
        setSubject(propertyValue);
    }
    else if (propertyName == L"Body") {
        setBody(propertyValue);
    }
    else if (propertyName == L"Location") {
        setLocation(propertyValue);
    }
    else if (propertyName == L"Start") {
        setStart(propertyValue);
    }
    else if (propertyName == L"End") {
        setEnd(propertyValue);
    }
    else if (propertyName == L"AllDayEvent") {
        setAllDayEvent(propertyValue);
    }
    else if (propertyName == L"BusyStatus") {
        setBusyStatus(propertyValue);
    }
    else if (propertyName == L"ReminderSet") {
        setReminderSet(propertyValue);
    }
    else if (propertyName == L"ReminderMinutesBeforeStart") {
        setReminderMinutesBeforeStart(propertyValue);
    }
    else if (propertyName == L"Importance") {
        setImportance(propertyValue);
    }
    else {
        // wrong propertyName...
        return 1;
    }

    return 0;
}





const wstring ClientAppException::getSubject() {
    if (!isUpdated) {
        read();
    }
    return subject;
}
void ClientAppException::setSubject(const wstring& val) {
    subject = val;
}


const wstring ClientAppException::getBody() {
    if (!isUpdated) {
        read();
    }
    return body;
}
void ClientAppException::setBody(const wstring& val) {
    body = val;
}


const wstring ClientAppException::getLocation() {
    if (!isUpdated) {
        read();
    }
    return location;
}
void ClientAppException::setLocation(const wstring& val) {
    location = val;
}


const wstring ClientAppException::getStart() {
    if (!isUpdated) {
        read();
    }
    return start;
}
void ClientAppException::setStart(const wstring& val) {
    start = val;
}


const wstring ClientAppException::getEnd() {
    if (!isUpdated) {
        read();
    }
    return end;
}
void ClientAppException::setEnd(const wstring& val) {
    end = val;
}


const wstring ClientAppException::getAllDayEvent() {
    if (!isUpdated) {
        read();
    }
    return allDayEvent;
}
void ClientAppException::setAllDayEvent(const wstring& val) {
    allDayEvent = val;
}


const wstring ClientAppException::getBusyStatus() {
    if (!isUpdated) {
        read();
    }
    return busyStatus;
}
void ClientAppException::setBusyStatus(const wstring& val) {
    busyStatus = val;
}


const wstring ClientAppException::getReminderSet() {
    if (!isUpdated) {
        read();
    }
    return reminderSet;
}
void ClientAppException::setReminderSet(const wstring& val) {
    reminderSet = val;
}


const wstring ClientAppException::getReminderMinutesBeforeStart() {
    if (!isUpdated) {
        read();
    }
    return reminderMinutesBeforeStart;
}
void ClientAppException::setReminderMinutesBeforeStart(const wstring& val) {
    reminderMinutesBeforeStart = val;
}


const wstring ClientAppException::getImportance() {
    if (!isUpdated) {
        read();
    }
    return importance;
}
void ClientAppException::setImportance(const wstring& val) {
    importance = val;
}





/**
 * Returns the 'OriginalDate' property of this exception in string format.
 * Uses the pException COM pointer (must be previously set).
 * Throws a ClientException in case of errors.
 * 
 * @param isAllDay : flag to know if the current appointment is an all-day event
 *                     0 -> NO ALL-DAY -> "YYYYMMDDThhmmssZ" (we use the specific date&time)
 *                     1 -> ALL-DAY    -> "yyyyMMdd"         (only the date is used)
 * @param start    : the "Start" property of current appointment
 *                   this is necessary to correct the OriginalDate in case occurrence is deleted
 * @return         : the OriginalDate value
 */
const wstring ClientAppException::formatOriginalDate(BOOL isAllDay, const wstring& start) {

    wstring propertyValue = EMPTY_WSTRING;
    DATE startDate = NULL;
    DATE date = getOriginalDate();

    if (!date) {
        return EMPTY_WSTRING;
    }

    if (isAllDay) {
        doubleToSystemTime(propertyValue, date, false, true);        // "yyyyMMdd"
    }
    else {
        if (getDeleted()) {
            //
            // Note: correct the originalDate time.
            // If occurrence is deleted, midnight it's returned but we need the exact time
            // so we add the hour value from "Start".
            //
            systemTimeToDouble(start, &startDate);
            int startDateMidnight = (int)startDate;
            date = (int)(date) + (startDate - startDateMidnight);
        }
        doubleToSystemTime(propertyValue, date, USE_UTC);           // "YYYYMMDDThhmmssZ"
    }

    return propertyValue;
}

/**
 * Returns the 'OriginalDate' property of this exception in string format.
 * Uses by default ALLDAY = TRUE.
 */
const wstring ClientAppException::formatOriginalDate() {
    return formatOriginalDate(TRUE, EMPTY_WSTRING);
}


