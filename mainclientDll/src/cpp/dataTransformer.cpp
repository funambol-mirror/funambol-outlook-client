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

#include "base/Log.h"
#include "base/stringUtils.h"
#include "winmaincpp.h"
#include "utils.h"
#include "vocl/VConverter.h"

#include "vocl/WinContact.h"
#include "vocl/WinEvent.h"
#include "vocl/WinTask.h"
#include "vocl/WinNote.h"
#include "vocl/WinContactSIF.h"
#include "vocl/WinEventSIF.h"
#include "vocl/WinTaskSIF.h"
#include "vocl/WinNoteSIF.h"

#include "outlook/utils.h"
#include "outlook/ClientItem.h"
#include "outlook/ClientAppException.h"
#include "outlook/ClientException.h"
#include "SIFFields.h"

#include <string>

using namespace std;


/**
 * Creates an empty WinItem object of the desired type (Client to Server).
 * 
 * @param useSIF    true if we use SIF data
 * @param itemType  the item type ("contact", "task", ...)
 * @return          a new allocated WinItem* of the desired type (WinContact, WinEvent,...)
 */
WinItem* createWinItem(bool useSIF, const wstring itemType) {
    
    WinItem* item = NULL;

    if (itemType == CONTACT) {
        if (useSIF) item = new WinContactSIF();
        else        item = new WinContact();
    }
    else if (itemType == APPOINTMENT) {
        if (useSIF) item = new WinEventSIF();
        else        item = new WinEvent();
    }
    else if (itemType == TASK) {
        if (useSIF) item = new WinTaskSIF();
        else        item = new WinTask();
    }
    else if (itemType == NOTE) {
        if (useSIF) item = new WinNoteSIF();
        else        item = new WinNote();
    }
    else {
        LOG.error("Internal error! createWinItem: item type '%ls' not supported", itemType.c_str());
    }

    return item;
}



/**
 * Creates and fills a WinItem object of the desired type (Server to Client).
 * Fills the WinItem map parsing the passed 'data' wstring.
 * 
 * @param useSIF    true if we use SIF data
 * @param itemType  the item type ("contact", "task", ...)
 * @param data      the data input string to parse
 * @param sifFields pointer to the static array of SIF fields used
 * @return          a new allocated WinItem* of the desired type (WinContact, WinEvent,...)
 */
WinItem* createWinItem(bool useSIF, const wstring itemType, const wstring& data, const WCHAR** sifFields) {
    
    WinItem* item = NULL;

    if (itemType == CONTACT) {
        if (useSIF) item = new WinContactSIF(data, sifFields);
        else        item = new WinContact(data);
    }
    else if (itemType == APPOINTMENT) {
        if (useSIF) item = new WinEventSIF(data, sifFields, (const WCHAR**)recurrenceFields);
        else        item = new WinEvent(data);
    }
    else if (itemType == TASK) {
        if (useSIF) item = new WinTaskSIF(data, sifFields, (const WCHAR**)recurrenceFields);
        else        item = new WinTask(data);
    }
    else if (itemType == NOTE) {
        if (useSIF) item = new WinNoteSIF(data, sifFields);
        else        item = new WinNote(data);
    }
    else {
        LOG.error("Internal error! createWinItem: item type '%ls' not supported", itemType.c_str());
    }

    return item;
}




/**
 * ClientItem object -> SyncItem object (client to server).
 * SyncItem data is the string generated from all ClientItem properties.
 * Data is converted into SIF or other mime types, based on 'dataType' parameter.
 *
 * @param cItem          [INPUT] pointer to ClientItem
 * @param dataType       the mime type of data we want into the SyncItem (SIF/VCard/...)
 * @param defaultFolder  the default folder path for this syncsource
 * @return               the (new allocated) SyncItem object.
 *                       Returned pointer MUST be freed by the caller
 */
SyncItem* convertToSyncItem(ClientItem* cItem, const char* dataType, const wstring& defaultFolder) {

    if (!cItem) {
        return NULL;
    }

    wstring id   = cItem->getID();
    wstring type = cItem->getType();
    if (id == EMPTY_WSTRING) {
        return NULL;
    }

    SyncItem* sItem = new SyncItem(id.c_str());
    char* data = NULL;
    WCHAR** sifFields = getProperSifArray(type);
    bool useSIF = isSIF(dataType);

    // Replace "\\Personal Folders\Contacts" with "DEFAULT_FOLDER"
    wstring path = cItem->getParentPath();
    replaceDefaultPath(path, defaultFolder);


    // Internally switch to the correct WinObject.
    WinItem* winItem = createWinItem(useSIF, cItem->getType());


    //
    // Fill all properties: ClientItem -> WinItem.
    //
    for (int i=0; sifFields[i]; i++) {
        const wstring value = cItem->getProperty(sifFields[i]);
        winItem->setProperty(sifFields[i], value);
    }
    winItem->setProperty(L"Folder", path);


    //
    // Set specific properties for each item-type.
    //
    if (cItem->getType() == CONTACT) {
        ClientApplication* ol = ClientApplication::getInstance();
        int version = _wtoi(ol->getVersion().c_str());
        if (version < 11) {
            // Photo supported on Outlook2003 or later. We remove the property
            // so that the <Photo> tag will not be sent.
            winItem->removeElement(L"Photo");
        }
        else {
            // Photo type is always "jpg".
            WinContact* winC = (WinContact*)winItem;
            winC->setPhotoType(L"JPEG");
        }
    }

    else if (cItem->getType() == APPOINTMENT) {
        WinEvent* winE = (WinEvent*)winItem;

        // Manage recurrence properties
        ClientAppointment* cApp = (ClientAppointment*)cItem;
        ClientRecurrence*  cRec = NULL;
        if ( cApp && (cRec = cApp->getRecPattern()) ) {
            // Fill recurrence properties.
            
            // getTimezone from appointment            
            ClientApplication* application = ClientApplication::getInstance();                        
            TIME_ZONE_INFORMATION* tz = application->getTimezone(cApp);
            winE->setTimezone(tz);
            delete tz;

            WinRecurrence* rec = winE->getRecPattern();
            for (int i=0; recurrenceFields[i]; i++) {
                const wstring value = cRec->getProperty(recurrenceFields[i]);
                rec->setProperty(recurrenceFields[i], value);
            }                       
            
            // Fill exception list (only EXDATE).
            int numExc = cRec->getExceptionsCount();
            if (numExc > 0) {
                BOOL isAllDay = cApp->isAllDayEvent();
                wstring start = cApp->getProperty(L"Start");
                for (int i=0; i<numExc; i++) {
                    ClientAppException* cEx = cRec->getException(i);
                    if (cEx) {
                        // Necessary to get the correct value! (bug on outlook for allday deleted)
                        wstring exDate = cEx->formatOriginalDate(isAllDay, start);
                        winE->getExcludeDates()->push_back(exDate);
                    }
                }
            }
        }
    }

    else if (cItem->getType() == TASK) {
        WinTask* winT = (WinTask*)winItem;

        // Manage recurrence properties
        ClientTask* cTask = (ClientTask*)cItem;
        ClientRecurrence* cRec = NULL;
        if ( cTask && (cRec = cTask->getRecPattern()) ) {
            // Fill recurrence properties.
            WinRecurrence* rec = winT->getRecPattern();
            for (int i=0; recurrenceFields[i]; i++) {
                const wstring value = cRec->getProperty(recurrenceFields[i]);
                rec->setProperty(recurrenceFields[i], value);
            }
        }
    }


    //
    // Format the data string. 'toString' will call the specialized method
    // for the correct WinObject.
    //
    wstring& wdata = winItem->toString();
    data = toMultibyte(wdata.c_str());
    if (winItem) {
        delete winItem; winItem = NULL;
    }


    //
    // Set SyncItem data.
    //
    if (data) {
        // Just replace chars < 0x20 (excluding 0x0a, 0x0d, 0x09).
        checkIllegalXMLChars(data);

        long dataSize = strlen(data);
        sItem->setData(data, dataSize);
        delete [] data;
    }
    else {
        // Data NULL: not allowed -> return NULL.
        if (sItem) {
            delete sItem;  
            sItem = NULL;
        }
    }

    return sItem;
}




/**
 * Check for illegal XML chars inside 'data'.
 * Chars < 0x20 are converted to 0x20 (space) - preserved '\t' '\n' '\r'.
 *
 * @param  data   [IN/OUT] the buffer to analyze
 * @return        1 if some char is converted, 0 otherwise
 */
int checkIllegalXMLChars(char* data) {
    
    if (!data) return 0;
    int len = strlen(data);

    int ret = 0;
    for (int i=0; i<len; i++) {
        if ( (unsigned char)data[i] < 0x20 ) {
            if ( (data[i] != 0x09) &&       // '\t'
                 (data[i] != 0x0a) &&       // '\n'
                 (data[i] != 0x0d) ) {      // '\r'
                data[i] = 0x20;             // ' '
                ret = 1;
            }
        }
    }
    return ret;
}

    

/**
 * data string (SIF/VCard/...) -> ClientItem (server to client).
 * Fill the passed ClientItem object with all properties from 'data' string.
 * Data is parsed from SIF or other mime types, based on 'dataType' parameter.
 *
 * @param data     : the input string to read from
 * @param cItem    : [IN-OUT] the ClientItem object to fill
 * @param itemType : the type of item (contact/task/...)
 * @param dataType : the mime type of data sent by server
 * @return           0 if no errors
 */
int fillClientItem(const wstring& data, ClientItem* cItem, const wstring& itemType, const WCHAR* dataType) {


    // If appointment/task, we clear the recurrence pattern (all fields are always sent). 
    // Also appointment exceptions are removed here (if any).
    if (itemType == APPOINTMENT) {
        ((ClientAppointment*)cItem)->clearRecPattern();
    }
    else if (itemType == TASK) {
        ((ClientTask*)cItem)->clearRecPattern();
    }

    bool useSIF = isSIF(dataType);

    wstring propertyValue;
    WCHAR** sifFields = getProperSifArray(itemType);
    

    // Internally switch to the correct WinObject and
    // fill it (parse data string + fill propertyMap).
    WinItem* winItem = createWinItem(useSIF, itemType, data, (const WCHAR**)sifFields);


    //
    // WinItem -> ClientItem (set only properties found)
    // "Folder" is retrieved separately in WindowsSyncSource.
    //
    for (int i=0; sifFields[i]; i++) {
        if (winItem->getProperty(sifFields[i], propertyValue)) {
            cItem->setProperty(sifFields[i], propertyValue);
        }
    }

    //
    // Set specific properties for each item-type.
    //
    if (itemType == APPOINTMENT) {
        WinEvent* winE = (WinEvent*)winItem;

        // Set recurrence properties
        ClientAppointment* cApp = (ClientAppointment*)cItem;
        ClientRecurrence*  cRec = NULL;
        if ( cApp && (cRec = cApp->getRecPattern()) ) {     // cRec is not NULL if 'IsRecurring' is set = 1.
            
            WinRecurrence* rec = winE->getRecPattern();
                        
            // first of all check if there is the Timezone Information set
            // and set the prop in the recurrence. It is used in the 
            if (winE->hasTimezone()) {
                cApp->setRecurringTimezone(winE->getTimezone());
                cRec->setHasTimezone(winE->hasTimezone());
                // timezone in the recurring for the conversion 
                // of the date with timezonetz function
                cRec->setRecurringTimezone(winE->getTimezone());
            }   
            
            for (int i=0; recurrenceFields[i]; i++) {
                wstring value;
                // Set only properties found!
                if (rec->getProperty(recurrenceFields[i], value)) {
                    cRec->setProperty(recurrenceFields[i], value);
                }
            }

            // Set events exceptions
            setRecurrenceExceptions(cItem, *(winE->getExcludeDates()), *(winE->getIncludeDates()));
        }
    }

    else if (itemType == TASK) {
        WinTask* winT = (WinTask*)winItem;

        // Set recurrence properties
        ClientTask* cTask = (ClientTask*)cItem;
        ClientRecurrence*  cRec = NULL;
        if ( cTask && (cRec = cTask->getRecPattern()) ) {     // cRec is not NULL if 'IsRecurring' is set = 1.

            WinRecurrence* rec = winT->getRecPattern();
            for (int i=0; recurrenceFields[i]; i++) {
                wstring value;
                // Set only properties found!
                if (rec->getProperty(recurrenceFields[i], value)) {
                    cRec->setProperty(recurrenceFields[i], value);
                }
            }
        }
    }

    if (winItem) {
        delete winItem; winItem = NULL;
    }

    return 0;
}




/**
 * Utility to return the right pointer to the static WCHAR** array 
 * of SIF fields, given the 'type' itemType.
 */
WCHAR** getProperSifArray(const wstring& type) {

    if (type == CONTACT) {
        return contactFields;
    }
    else if (type == APPOINTMENT) {
        return appointmentFields;
        }
    else if (type == TASK) {
        return taskFields;
        }
    else if (type == NOTE) {
        return noteFields;
    }
    else {
        LOG.error("Internal error: getProperSifArray, bad item type '%ls'", type.c_str());
        return NULL;
    }
}





/**
 * Normalize all appointment exceptions for passed item (client to server).
 * This is done before sending items to server. After normalization we will have
 * only exceptions that are 'deleted occurrences', and new unlinked appointments for
 * other exceptions. New appointments are also added to the 'allItems' list.
 * In case of errors, 'lastErrorMsg' is set and '1' is returned.
 *
 * @param allItems : [IN-OUT] the list of all appointments to scan
 * @return           0 if no errors
 */
int normalizeExceptions(ClientItem* cItem, itemKeyList& allItems, itemKeyList& allItemsPaths) {

    ClientAppointment* cApp = (ClientAppointment*)cItem;
    if (!cApp) {
        // Maybe is not an appointment.
        return 1;
    }

    ClientRecurrence* cRec = cApp->getRecPattern();
    if (!cRec) {
        // Not recurring -> no exceptions
        return 0;
    }

    int exCount = cRec->getExceptionsCount();
    if (exCount == 0) {
        // No exceptions
        return 0;
    }


    //
    // Scan all exceptions
    //
    for (int i=0; i<exCount; i++) {
        ClientAppException* cEx = cRec->getException(i);
        if (cEx->getDeleted()) {
            // Deleted exceptions are OK.
            continue;
        }
        else {
            // Modified exceptions must be converted.
            // --------------------------------------
            LOG.debug(DBG_NORMALIZING_EXCEPTION, getSafeItemName(cItem).c_str(), cEx->formatOriginalDate().c_str());

            //
            // 1. Create new (unlinked) single event.
            //
            // Copy from main appointment (all original props are copied).
            ClientItem* cItemNew = cItem->copyItem();

            if (cItemNew) {
                // MUST clear the rec pattern...
                cItemNew->setProperty(L"IsRecurring", L"0");
                ((ClientAppointment*)cItemNew)->clearRecPattern();

                // Set exception properties on the new appointment.
                wstring propertyValue = EMPTY_WSTRING;
                for (int j=0; exAppointmentFields[j]; j++) {
                    propertyValue = cEx->getAppProperty(exAppointmentFields[j]);
                    cItemNew->setProperty(exAppointmentFields[j], propertyValue);
                }

                // Save the new appointment and add its LUID to the new items list.
                if (!cItemNew->saveItem()) {
                    allItems.push_back(cItemNew->getID());
                    allItemsPaths.push_back(cItemNew->getParentPath());
                }
                else {
                    setErrorF(getLastErrorCode(), ERR_ITEM_SAVE, APPOINTMENT, getSafeItemName(cItemNew).c_str(), cItemNew->getParentPath().c_str());
                    goto error;
                }
                delete cItemNew;
                cItemNew = NULL;
            }
            else {
                setErrorF(getLastErrorCode(), ERR_ITEM_CREATE, APPOINTMENT, cItemNew->getParentPath().c_str());
                goto error;
            }

            //
            // 2. Modify exception -> convert into a 'occurrence deleted'
            //    Must also save the recurring appointment (it's modified)
            //
            cEx->setDeleted(TRUE);
            if (cItem->saveItem()) {
                setErrorF(getLastErrorCode(), ERR_ITEM_SAVE, L"recurring appointment", getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
                goto error;
            }

            //
            // 3. Delete also ALL (existing) occurrences between originalDate and StartDate!
            //    (create an exception 'occurrence deleted' for each occurrence found).
            //
            DATE startDate  = NULL;
            systemTimeToDouble(cEx->getStart(), &startDate, true);
            DATE originalDate = cEx->getOriginalDate();
            int numExCreated = deleteOccurrencesInInterval(startDate, originalDate, cRec);

            //
            // 4. Save the recurring appointment (if it's modified)
            //
            if (numExCreated > 0) {
                if (cItem->saveItem()) {
                    setErrorF(getLastErrorCode(), ERR_ITEM_SAVE, L"recurring appointment", getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
                    goto error;
                }
            }
        }
    } // end: for (int i=0; i<exCount; i++)

    return 0;

error:
    LOG.error(getLastErrorMsg());
    return 1;
}





/**
 * Deletes all existing occurrences in the interval [startDate - originalDate[ , 
 * from ClientRecurrence object 'cRec'.
 * When an occurrence is found in the given interval, a proper exception
 * is added to 'cRec' (a 'deleted occurrence' exception).
 * 
 * @param startDate    : the start date (included in interval)
 * @param originalDate : the original date (excluded from interval)
 * @param cRec         : the ClientRecurrence pointer to work on
 * @return               the number of occurrences deleted 
 *                       ( = number of exceptions created in cRec)
 */
int deleteOccurrencesInInterval(const DATE startDate, const DATE originalDate, ClientRecurrence* cRec) {

    int exCreated = 0;
    int originalDay = (int)originalDate;
    int startDay    = (int)startDate;
    DATE date1, date2;
    _AppointmentItemPtr pOcc = NULL;

    // Same day: nothing to do
    if (originalDay == startDay) {
        return 0;
    }

    // Get date interval [date1 - date2] of occurrences to delete.
    if (originalDay < startDay) {
        date1 = originalDate + 1;
        date2 = startDate;
    }
    else {
        date1 = startDate;
        date2 = originalDate - 1;
    }

    // Delete all occurrences found between date1 and date2.
    // (create exceptions as 'occurrence deleted').
    for (DATE i=date1; i<=date2; i++) {

        if (cRec->getOccurrence(i)) {
            ClientAppException* cExNew = new ClientAppException();

            cExNew->setOriginalDate(i);
            cExNew->setDeleted(TRUE);
            cRec->addException(cExNew);

            delete cExNew; cExNew = NULL;
            exCreated ++;
        }
    }
    return exCreated;
}





/**
 * Sets the appointment exceptions for the item passed (server to client).
 * Exceptions dates are passed with 2 lists 'escludeDates' and 'includeDates'.
 * Each exception found is set into a ClientAppException object. Each ClientRecurrence
 * object can have a list of ClientAppExceptions.
 * All exceptions are finally saved to Outlook when calling the method 'save()' of
 * the corrispondent ClientAppointment object.
 *
 * - Deleted occurrences are normally added to Client
 * - Added occurrences are not supported by Outlook 
 *   -> a new (unlinked event) is created
 *   -> the item is modified itself, must be added next sync to the MOD items list
 * 
 * @param cItem        : the ClientItem object to modify (it's an appointment)
 * @param excludeDates : a list of exclude-dates (occurrences to delete)
 * @param includeDates : a list of include-dates (occurrences to add)
 * @return               0 if exceptions saved with no errors.
 *                       1 if not recurring (nothing done).
 *                      -1 if errors.
 */
int setRecurrenceExceptions(ClientItem* cItem, list<wstring> &excludeDates, list<wstring> &includeDates) {

    if (!cItem) return -1;
    ClientAppointment*  cApp = (ClientAppointment*)cItem;
    if (!cApp)  return -1;
    ClientRecurrence*   cRec = cApp->getRecPattern();

    // Not recurring: nothing to do.
    if (!cRec) {
        if (excludeDates.size() || includeDates.size()) {
            LOG.error(ERR_PARSE_EXC_NOREC);
            return -1;
        }
        return 1;
    }

    // No data: reset exceptions.
    if (!excludeDates.size() && !includeDates.size()) {
        cRec->resetExceptions();
        return 0;
    }


    ClientAppException* cOcc = NULL;
    wstring excludeDate      = EMPTY_WSTRING;
    wstring includeDate      = EMPTY_WSTRING;
    list<wstring>::iterator  exIterator;
    list<wstring>::iterator  inIterator;
    bool modified = false;


    //
    // 1. EXCLUDE_DATE: 'occurrences to delete'.
    //
    if (excludeDates.size() > 0) {
        exIterator = excludeDates.begin();

        while (exIterator != excludeDates.end()) {
            excludeDate = *exIterator;
            if (!excludeDate.size()) {
                continue;
            }

            DATE date = NULL;
            systemTimeToDouble(excludeDate, &date);

            cOcc = new ClientAppException();
            cOcc->setOriginalDate(date);
            cOcc->setDeleted(TRUE);

            // Add the exception to the list of exceptions (will be saved during 'save()').
            cRec->addException(cOcc);

            if (cOcc) {
                delete cOcc;  cOcc = NULL;
            }
            exIterator ++;
        }
    }


    //
    // 2. INCLUDE_DATE: 'occurrences to add' -> create new appointment (unlinked)
    //    (added occurrences are not supperted by Outlook)
    //
    if (includeDates.size() > 0) {
        inIterator = includeDates.begin();

        while (inIterator != includeDates.end()) {
            includeDate = *inIterator;
            if (!includeDate.size()) {
                continue;
            }

            // Mark the recurring item as modified: an 'IncludeDate' is converted here to a new item unlinked
            // and so the item will be sent back to server next time.
            modified = true;

            //
            // Copy all properties from original item.
            //
            ClientItem* cItemNew = cItem->copyItem();

            // Set properties modified.
            ((ClientAppointment*)cItemNew)->clearRecPattern();                              // Not recurring, please...
            cItemNew->setProperty(L"IsRecurring", L"0");
            cItemNew->setProperty(L"Start", includeDate);                                   // Set the (different) Start property.
            cItemNew->setProperty(L"AllDayEvent", cItem->getProperty(L"AllDayEvent"));      // Seems necessary to set this property again... (bug?)
            // *** TODO: shift and set the "End" field? ***

            //
            // Save the new appointment to Outlook.
            //
            if (cItemNew->saveItem()) {
                setErrorF(getLastErrorCode(), ERR_ITEM_SAVE, APPOINTMENT, getSafeItemName(cItemNew).c_str(), cItemNew->getParentPath().c_str());
                goto error;
            }

            // Note: this new appointment will be automatically sent to server next
            //       sync as a NEW item.
            LOG.debug(DBG_ITEM_CREATED_FROM_EXCEPTION, getSafeItemName(cItemNew).c_str());

            if (cItemNew) {
                delete cItemNew;
                cItemNew = NULL;
            }
            inIterator ++;
        }
    }


    //
    // 3. If this appointment has been modified, we MUST add it to the list 
    //    of MOD items on next sync! (use a list of forced modified items)
    //
    if (modified) {

        // Open the file to store IDs for next sync
        WCHAR* filePath = readDataPath(APPOINTMENT_FORCED_MODIFIED);
        if (!filePath) {
            setErrorF(getLastErrorCode(), ERR_WFILE_OPEN, APPOINTMENT_FORCED_MODIFIED);
            goto error;
        }

        // MUST first save the item - to have the item ID...
        if (cItem->saveItem()) {
            setErrorF(getLastErrorCode(), ERR_ITEM_SAVE, L"recurring appointment", getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
            goto error;
        }

        // This item is added to a list of additional MOD items for next sync.
        wstring xml = L"<Item>";
        xml += cItem->getID();
        xml += L"</Item>\n";
        if (writeToFile(xml, filePath, L"a")) {     // Append at the end of file.
            goto error;
        }

        if (filePath) {
            delete [] filePath;
            filePath = NULL;
        }
    }

    return 0;

error:
    LOG.error(getLastErrorMsg());
    return -1;
}


/**
 * Get a property value from a string formatted vCard / vCalendar.
 * Parses the string, and returns the property value from the passed name.
 * @note  Used if vObject is not available, to parse properties
 *        it's better to use the vConveter::parse() method.
 *
 * @param dataString    the input string (vCard/vCalendar)
 * @param propertyName  the name of property to retrieve
 * @return              the property value (empty if not found)
 */
wstring getVPropertyValue(const wstring& dataString, const wstring& propertyName) {

    if (propertyName.length() == 0 || dataString.length() == 0) {
        return EMPTY_WSTRING;
    }

    wstring value = EMPTY_WSTRING;
    wstring::size_type pos = dataString.find(propertyName, 0);
    const wstring delim = L":\n\r";

    if (pos != wstring::npos) {
        pos += propertyName.length();
        wstring::size_type start = dataString.find_first_not_of(delim, pos);
        if (start != wstring::npos) {
            wstring::size_type end = dataString.find_first_of(delim, start);
            if ((end != wstring::npos) && (end-start > 0)) {
                value = dataString.substr(start, end-start);
            }
        }
    }

    // Un-escape special chars of vCard 2.1 (";" and "\").
    replaceAll(L"\\;", L";", value);
    replaceAll(L"\\\\", L"\\", value);
    return value;
}



/**
 * Replaces the 'defaultFolder' string with "DEFAULT_FOLDER" inside 'path'.
 * We need to temporary add a "\" char at the end, to avoid replacing 
 * folders names such as "\\Personal Folder\Contacts2" into "DEFAULT_FOLDER2".
 * @param path           the wstring to search (IN/OUT: it can be modified here)
 * @param defaultFolder  the wstring to search for
 */
void replaceDefaultPath(wstring& path, const wstring& defaultFolder) {

    if (path.length() == 0) {
        return;
    }

    wstring dest   = DEFAULT_FOLDER;    dest   += L"\\";
    wstring source = defaultFolder;     source += L"\\";

    path += L"\\";
    replaceAll(source, dest, path);
    path = path.substr(0, path.length()-1);
}
