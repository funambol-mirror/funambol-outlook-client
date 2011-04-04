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
#include "outlook/ClientTask.h"
#include "outlook/ClientAppointment.h"
#include "outlook/ClientContact.h"
#include "outlook/ClientNote.h"
#include "outlook/ClientAppException.h"
#include "outlook/ClientException.h"
#include "SIFFields.h"
#include "customization.h"
#include "spds/spdsutils.h"

#include "syncml\core\Property.h"
#include "syncml\core\PropParam.h"
#include <string>

using namespace std;

void initWinItems() {
    WinItem::setDefaultValidateFunction(&(DLLCustomization::validateExtraProperty));
}

wstring getEmptyVCard();

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
        else {               
            const char* v = getConfig()->getServerConfig().getNoFieldLevelReplace();
            if (v) {
                StringBuffer values(v);
                if (values.ifind(getConfig()->getSyncSourceConfig(CONTACT_)->getURI()) != StringBuffer::npos) {
                    wstring emptyVCard = getEmptyVCard();
                    WinContact* itemTmp = new WinContact(emptyVCard);        
                    itemTmp->parseMapReset(data, false);
                    itemTmp->setPhotoType(L"JPEG");

                    wstring newData;
                    wstring photo;
                    bool exists = itemTmp->getProperty(L"Photo", photo);
                    if (exists && !photo.empty()) {
                        StringBuffer s; 
                        s.convert(photo.c_str());
                        char* b64tmp = new char[s.length()];
                        int len = b64_decode(b64tmp, s.c_str());
                        b64tmp[len] = 0;
                        char* result = b64EncodeWithSpaces(b64tmp, len);
                        WCHAR* tt = toWideChar(result);
                        itemTmp->setProperty(L"Photo", tt);
                        itemTmp->setPhotoType(L"JPEG");
                        delete [] b64tmp;
                        delete [] result;
                        delete [] tt;
                    }
                    //newData = itemTmp->toString();
                    //delete itemTmp;
                    //item = new WinContact(newData);
                    item = (WinItem*)itemTmp;                    
                } else {
                    item = new WinContact(data);
                }
            } else {
                item = new WinContact(data);
            }
        }
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
 * @param addUserProperties 
 *                       Wether or not to add user properties to the outgoing item
 * @return               the (new allocated) SyncItem object.
 *                       Returned pointer MUST be freed by the caller
 */
SyncItem* convertToSyncItem(ClientItem* cItem, const char* dataType, const wstring& defaultFolder, bool addUserProperties) {

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
    wstring propertyValue;

    // Replace "\\Personal Folders\Contacts" with "DEFAULT_FOLDER"
    wstring path = cItem->getParentPath();
    replaceDefaultPath(path, defaultFolder);

    // Internally switch to the correct WinObject.
    // Fill WinItem object (parse data string).
    WinItem * winItem = createWinItem(useSIF, type);

    // "Folder" is retrieved separately in WindowsSyncSource.
    for (int i=0; sifFields[i]; i++) {
        propertyValue = cItem->getProperty(sifFields[i]);
        winItem->setProperty(sifFields[i], propertyValue);
    }

    cItem->createUserPropertyMap();

    if (addUserProperties) {
        std::vector<std::wstring> names = cItem->getUserPropertyNames();
        std::vector<std::wstring>::iterator it = names.begin();
        for (; it != names.end(); it++) {
            if (cItem->getUserProperty(*it, propertyValue)) {
                winItem->setExtraProperty(*it, propertyValue);
            }
        }
    }

    winItem->setProperty(L"Folder", path);

    //
    // Set specific properties for each item-type.
    //
    if (cItem->getType() == CONTACT) {
        ClientApplication* ol = ClientApplication::getInstance();
        int version = _wtoi(ol->getVersion().c_str());
        if (version < 11 || DLLCustomization::neverSendPhotos) {
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

    else if (type == APPOINTMENT) {
        WinEvent * winE = (WinEvent*)winItem;

        // Manage recurrence properties
        ClientAppointment* cApp = (ClientAppointment*)cItem;
        ClientRecurrence*  cRec = NULL;
        if ( cApp && (cRec = cApp->getRecPattern()) ) {
            // getTimezone from appointment
            if (DLLCustomization::sendTimezone) {
                ClientApplication* application = ClientApplication::getInstance();
                TIME_ZONE_INFORMATION* tz = application->getTimezone(cApp);
                winE->setTimezone(tz);
                delete tz;
            }

            // Fill recurrence properties
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

        // Attendees
        if (cApp && DLLCustomization::syncAttendees) {
            list<WinRecipient>* attendees = winE->getRecipients();
            std::map<int, ClientRecipient> attList = cApp->getAttendees();
            std::map<int, ClientRecipient>::iterator it;
            for (it = attList.begin(); it != attList.end(); it++) {
                WinRecipient wr;
                wr.setProperty(L"AttendeeName", (it->second).getName());
                wr.setProperty(L"AttendeeEmail", (it->second).getEmail());

                // Get status doesnt really work yet, but this block is right
                switch ((it->second).getStatus())
                {
                    default:
                    case Redemption::olResponseNone:
                    case Redemption::olResponseNotResponded:
                    case Redemption::olResponseOrganized:
                            wr.setProperty(L"AttendeeStatus", L"NEEDS ACTION");
                        break;
                    case Redemption::olResponseTentative:
                            wr.setProperty(L"AttendeeStatus", L"TENATIVE");
                        break;
                    case Redemption::olResponseAccepted:
                            wr.setProperty(L"AttendeeStatus", L"ACCEPTED");
                        break;
                    case Redemption::olResponseDeclined:
                            wr.setProperty(L"AttendeeStatus", L"DECLINED");
                        break;
                }

                attendees->push_back(wr);
            }
        }
    }

    else if (type == TASK) {
        WinTask * winT = (WinTask*)winItem;

        // Manage recurrence properties
        ClientTask * cTask = (ClientTask*)cItem;
        ClientRecurrence*  cRec = NULL;
        if ( cTask && (cRec = cTask->getRecPattern()) && cTask->getProperty(L"Complete").compare(L"1")) {
            // Fill recurrence properties.
            WinRecurrence* rec = winT->getRecPattern();
            for (int i=0; recurrenceFields[i]; i++) {
                const wstring value = cRec->getProperty(recurrenceFields[i]);
                rec->setProperty(recurrenceFields[i], value);
            }

            // TODO: Recurrence exceptions
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

    if (itemType == CONTACT) {
        if (!DLLCustomization::saveFileAs && winItem->getProperty(L"FileAs", propertyValue)) {
            winItem->removeElement(L"FileAs");
        }
    }

    //
    // WinItem -> ClientItem (set only properties found)
    // "Folder" is retrieved separately in WindowsSyncSource.
    //
    for (int i=0; sifFields[i]; i++) {
        if (winItem->getProperty(sifFields[i], propertyValue)) {
            cItem->setProperty(sifFields[i], propertyValue);
        }
    }

    // Extra properties
    std::vector<std::wstring> names = winItem->getExtraPropertyNames();
    std::vector<std::wstring>::iterator it = names.begin();
    for (; it != names.end(); it++) {
        if (winItem->getExtraProperty(*it, propertyValue)) {
            cItem->setUserProperty(*it, propertyValue);
        }
    }

    //
    // Set specific properties for each item-type.
    //
    if (itemType == APPOINTMENT) {
        WinEvent * winE = ((WinEvent*)winItem);

        // Set recurrence properties
        ClientAppointment* cApp = (ClientAppointment*)cItem;
        ClientRecurrence * cRec = NULL;

        // cRec is not NULL if 'IsRecurring' is set = 1.
        if ( cApp && (cRec = cApp->getRecPattern()) ) {     // cRec is not NULL if 'IsRecurring' is set = 1.

            WinRecurrence * rec = ((WinEvent*)winItem)->getRecPattern();

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
            setRecurrenceExceptions(cItem, cRec, *(winE->getExcludeDates()), *(winE->getIncludeDates()));
        }

        // Attendees
        if (DLLCustomization::syncAttendees) {
            list<WinRecipient>* attendees = ((WinEvent*)winItem)->getRecipients();
            list<WinRecipient>::iterator it;

            std::wstring email;
            std::wstring attendee;

            std::map<int, ClientRecipient> attList = cApp->getAttendees();
            std::map<int, ClientRecipient>::iterator listIt;

            // For all existing attendees
            for (it = attendees->begin(); it != attendees->end(); it++) {
                email = L"";

                if (it->getProperty(L"AttendeeEmail", email)) {
                    bool found = false;
                    for (listIt = attList.begin(); listIt != attList.end(); listIt++) {
                        if (email.compare((listIt->second).getEmail()) == 0) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        it->getNamedEmail(attendee);
                        cApp->addAttendee(ClientRecipient(attendee));
                    }
                }
            }

            std::map<int, ClientRecipient>::reverse_iterator rlistIt;

            // For all existing attendees (backwards, so when we remove indices, the next ones work
            for (rlistIt = attList.rbegin(); rlistIt != attList.rend(); rlistIt++) {

                email = L"";
                bool found = false;

                // For all incoming attendees
                for (it = attendees->begin(); it != attendees->end(); it++) {
                    email = L"";
                    if (it->getProperty(L"AttendeeEmail", email)) {
                        if (email.compare((rlistIt->second).getEmail()) == 0) {
                            found = true;
                            break;
                        }
                    }
                }

                // If there's an existing attendee that is not incoming, remove it
                if (!found) {
                    cApp->removeAttendee(rlistIt->first);
                }
            }
        }
    }

    else if (itemType == TASK) {
        ClientTask* cTask = (ClientTask*)cItem;
        WinRecurrence * wRec = ((WinTask*)winItem)->getRecPattern();

        // cRec is not NULL if 'IsRecurring' is set = 1.
        if (cTask && wRec) {
            ClientRecurrence*  cRec = cTask->getRecPattern();
            if (cRec) {

                // Manage recurring properties.
                for (int i=0; recurrenceFields[i]; i++) {
                    if (wRec->getProperty(recurrenceFields[i], propertyValue)) {
                        replaceAll(L"&lt;",  L"<", propertyValue);
                        replaceAll(L"&gt;",  L">", propertyValue);
                        replaceAll(L"&amp;", L"&", propertyValue);
                        cRec->setProperty(recurrenceFields[i], propertyValue);
                    }
                }

                // TODO: recurrence exceptions
            }
        }
    }

    if (winItem) {
        delete winItem;
        winItem = NULL;
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
        // error...
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

    bool changed = false;

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
            changed = true;

            // Modified exceptions must be converted.
            // --------------------------------------
            LOG.debug(DBG_NORMALIZING_EXCEPTION, getSafeItemName(cItem).c_str(), cEx->formatOriginalDate().c_str());

            //
            // 1. Create new (unlinked) single event.
            //
            // Copy from main appointment (all original props are copied).
            ClientFolder * folder = NULL;
            folder = ClientApplication::getInstance()->getFolderFromPath(cItem->getType(), cItem->getParentPath());
            if (!folder) {
                // TODO failure
            }

            ClientItem * cItemNew = folder->addItem();

            if (cItemNew) {
                // clear the rec pattern...
                cItemNew->setProperty(L"IsRecurring", L"0");
                ((ClientAppointment*)cItemNew)->clearRecPattern();

                // Set exception properties on the new appointment.
                wstring propertyValue = EMPTY_WSTRING;
                for (int j=0; exAppointmentFields[j]; j++) {
                    propertyValue = cEx->getAppProperty(exAppointmentFields[j]);
                    cItemNew->setProperty(exAppointmentFields[j], propertyValue);
                }

                std::vector<std::wstring> attendees = cEx->getAttendees();
                if (attendees.size() == 0) {
                    cEx->inheritAttendees((ClientAppointment*)cItem);
                    attendees = cEx->getAttendees();
                }
                for (size_t x = 0; x < attendees.size(); x++) {
                    ((ClientAppointment*)cItemNew)->addAttendee(ClientRecipient(attendees[x]));
                }

                // Save the new appointment and add its LUID to the new items list.
                if (!cItemNew->saveItem()) {
                    allItems.push_back(cItemNew->getID());
                    allItemsPaths.push_back(cItemNew->getParentPath());
                }
                else {
                    setErrorF(0,ERR_ITEM_SAVE, APPOINTMENT, getSafeItemName(cItemNew).c_str(), cItemNew->getParentPath().c_str());
                    goto error;
                }
            }
            else {
                setErrorF(0, ERR_ITEM_CREATE, APPOINTMENT, cItemNew->getParentPath().c_str());
                goto error;
            }

            //
            // 2. Modify exception -> convert into a 'occurrence deleted'
            //    Must also save the recurring appointment (it's modified)
            //
            cEx->setDeleted(TRUE);
            // Dont save the event yet, we do all work in memory

            //
            // 3. Delete also ALL (existing) occurrences between originalDate and StartDate!
            //    (create an exception 'occurrence deleted' for each occurrence found).
            //
            DATE originalDate = cEx->getOriginalDate();

            DATE startDateNewTime  = NULL;
            systemTimeToDouble(cEx->getStart(), &startDateNewTime, false);

            DATE startDateOriginalTime = (int)startDateNewTime + (DATE)(originalDate - ((DATE)int(originalDate)));
            DATE originalDateNewTime = (int)originalDate + (DATE)(startDateNewTime - ((DATE)int(startDateNewTime)));

            int numExCreated = 0;
            numExCreated += deleteOccurrencesInInterval(startDateOriginalTime, originalDate, cRec);
            if (originalDate != originalDateNewTime) {
                // Event time has changed, check for the exception at the new time
                numExCreated += deleteOccurrencesInInterval(startDateNewTime, originalDateNewTime, cRec);
            }

            /*
            // Dont do this now - it causes problems.  Resolve all exceptions first

            //
            // 4. Save the recurring appointment (if it's modified)
            //
            if (numExCreated > 0) {
                if (cItem->saveItem()) {
                    setErrorF(0, ERR_ITEM_SAVE, L"recurring appointment", getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
                    goto error;
                }
            }

*/
        }
    } // end: for (int i=0; i<exCount; i++)

    if (changed) {
        if (cItem->saveItem()) {
            setErrorF(0, ERR_ITEM_SAVE, L"recurring appointment", getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
            goto error;
        }
    }

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
 * Sets the recurrence exceptions for the item passed (server to client).
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
 * @param cItem        : the ClientItem object to modify (it's an appointment or task)
 * @param cRec         : the ClientRecurrence object
 * @param excludeDates : a list of exclude-dates (occurrences to delete)
 * @param includeDates : a list of include-dates (occurrences to add)
 * @return               0 if exceptions saved with no errors.
 *                       1 if not recurring (nothing done).
 *                      -1 if errors.
 */
int setRecurrenceExceptions(ClientItem * cItem, ClientRecurrence* cRec, list<wstring> &excludeDates, list<wstring> &includeDates) {

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
    const wstring delimstart = L":";
    const wstring delimend = L"\r\n";

    if (pos != wstring::npos) {
        pos += propertyName.length();
        wstring::size_type start = dataString.find_first_not_of(delimstart, pos);
        if (start != wstring::npos) {
            wstring::size_type end = dataString.find_first_of(delimend, start);
            while ((end != wstring::npos)) {
                if (end == dataString.length()) {
                    // Data is bad, quit
                    break;
                }

                // Build up the string
                value += dataString.substr(start, end-start);
                WCHAR nextchar = dataString[end+2];
                if (nextchar == ' ') {
                    // Its a fold, not the end
                    start = end+3;
                    end = dataString.find_first_of(delimend, start);
                } else {
                    end = wstring::npos;
                }
            }
        }
    }

    // Unescape , too, bug in server will be fixed in later releases
    replaceAll(L"\\,", L",", value);

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


ArrayList* getVCalendarProperties(){

    ArrayList* props = new ArrayList();
    Property* p = new Property();
    ArrayList valenums;
    StringBuffer val;

    p->setPropName("BEGIN");
    val = "VCALENDAR";
    valenums.add(val);
	val = "VEVENT";
	valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);

    valenums.clear();

    p->setPropName("END");
    val = "VCALENDAR";
    valenums.add(val);
    val = "VEVENT";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);

    valenums.clear();
    
    p->setPropName("VERSION");
    val = "1.0";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();
	
	p->setPropName("CLASS");
    val = "PUBLIC";
    valenums.add(val);
	val = "PRIVATE";
    valenums.add(val);
	val = "CONFIDENTIAL";
    valenums.add(val);	
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();	
	delete p; p = NULL;    
    p = new Property();    

    p->setPropName("X-FUNAMBOL-ALLDAY");
    props->add(*p);
    p->setPropName("DESCRIPTION");
    props->add(*p);
    p->setPropName("X-MICROSOFT-CDO-BUSYSTATUS");
    props->add(*p);
    p->setPropName("CATEGORIES");
    props->add(*p);
    p->setPropName("DTEND");
    props->add(*p);
    p->setPropName("LOCATION");
    props->add(*p);
    p->setPropName("STATUS");
    props->add(*p);
    p->setPropName("SUMMARY");
    props->add(*p);
    p->setPropName("X-FUNAMBOL-BILLINGINFO");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-COMPANIES");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-MILEAGE");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-NOAGING");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-FOLDER");
    props->add(*p);
    p->setPropName("X-MICROSOFT-CDO-REPLYTIME");
    props->add(*p);
    p->setPropName("AALARM");
    props->add(*p);
	p->setPropName("PRIORITY"); 
    props->add(*p);
    p->setPropName("DTSTART");
    props->add(*p);
    p->setPropName("RRULE");
    props->add(*p);
    p->setPropName("EXDATE");
    props->add(*p);
    p->setPropName("RDATE");
    props->add(*p);
    p->setPropName("TZ");
    props->add(*p);
    p->setPropName("DAYLIGHT");
    props->add(*p);

    delete p; p = NULL;

    return props;
}

ArrayList* getVCardProperties(){

    ArrayList* props = new ArrayList();
    Property* p = new Property();
    PropParam* pp = new PropParam();
    ArrayList pparams;
    ArrayList valenums;
    StringBuffer val;


    p->setPropName("BEGIN");
    val = "VCARD";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);

    valenums.clear();

    p->setPropName("END");
    val = "VCARD";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);

    valenums.clear();
    
    p->setPropName("VERSION");
    val = "2.1";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();

	p->setPropName("CLASS");
    val = "PUBLIC";
    valenums.add(val);
	val = "PRIVATE";
    valenums.add(val);
	val = "CONFIDENTIAL";
    valenums.add(val);	
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();	
	
	delete p; p = NULL;
    delete pp; pp = NULL;
    p = new Property();
    pp = new PropParam();

    p->setPropName("X-ANNIVERSARY");
    props->add(*p);
    
    p->setPropName("BDAY");
    props->add(*p);
    
    p->setPropName("NOTE");
    props->add(*p);

	p->setPropName("FN");
    props->add(*p);

    // new ctcaps...

    p->setPropName("TEL");
    pp->setParamName("TYPE");
    valenums.add(StringBuffer("VOICE,WORK"));
    valenums.add(StringBuffer("WORK,FAX"));
    valenums.add(StringBuffer("CAR,VOICE"));
    valenums.add(StringBuffer("WORK,PREF"));
    valenums.add(StringBuffer("HOME,FAX"));
    valenums.add(StringBuffer("VOICE,HOME"));
    valenums.add(StringBuffer("PREF,VOICE"));
    valenums.add(StringBuffer("CELL"));
    valenums.add(StringBuffer("PAGER"));
    valenums.add(StringBuffer("FAX"));
    valenums.add(StringBuffer("VOICE"));
    valenums.add(StringBuffer("X-FUNAMBOL-TELEX"));
    valenums.add(StringBuffer("X-FUNAMBOL-RADIO"));
    valenums.add(StringBuffer("X-FUNAMBOL-CALLBACK"));    
    pp->setValEnums(&valenums);
    pparams.add(*pp);
    p->setPropParams(&pparams);
    props->add(*p);
	
	pparams.clear();
    delete p; p = NULL;
    delete pp; pp = NULL;
    p = new Property();
    pp = new PropParam();
    valenums.clear();

    p->setPropName("CATEGORIES");
    props->add(*p);

    p->setPropName("X-FUNAMBOL-CHILDREN");
    props->add(*p);

    p->setPropName("ORG");
    props->add(*p);

	p->setPropName("ROLE");
    props->add(*p);

    //p->setPropName("X-FUNAMBOL-CUSTOMERID");
    //props->add(*p);    

    p->setPropName("ADR");
    pp->setParamName("TYPE");
    valenums.add(StringBuffer("HOME"));
    valenums.add(StringBuffer("WORK"));
	pp->setValEnums(&valenums);
    pparams.add(*pp);
    p->setPropParams(&pparams);
    props->add(*p);

    pparams.clear();
    delete p; p = NULL;
    delete pp; pp = NULL;
    p = new Property();
    pp = new PropParam();
    valenums.clear();

    p->setPropName("URL");
    pp->setParamName("TYPE");
    valenums.add(StringBuffer("HOME"));
    pp->setValEnums(&valenums);
    pparams.add(*pp);
    p->setPropParams(&pparams);
    props->add(*p);

    pparams.clear();
    delete p; p = NULL;
    delete pp; pp = NULL;
    p = new Property();
    pp = new PropParam();
    valenums.clear();

    p->setPropName("EMAIL");
    pp->setParamName("TYPE");
    valenums.add(StringBuffer("INTERNET"));    
    valenums.add(StringBuffer("INTERNET,HOME"));
    valenums.add(StringBuffer("INTERNET,WORK"));
    valenums.add(StringBuffer("INTERNET,HOME,X-FUNAMBOL-INSTANTMESSENGER"));
    pp->setValEnums(&valenums);    
    pparams.add(*pp);
    p->setPropParams(&pparams);
    props->add(*p);

    pparams.clear();
    delete p; p = NULL;
    delete pp; pp = NULL;
    p = new Property();
    pp = new PropParam();
    valenums.clear();

    p->setPropName("TITLE");
    props->add(*p);

    p->setPropName("N");
    props->add(*p);

    p->setPropName("X-MANAGER");
    props->add(*p);
    
    p->setPropName("NICKNAME");
    props->add(*p);
    
    p->setPropName("PHOTO");
    props->add(*p);

	p->setPropName("PRIORITY");
    props->add(*p);

    p->setPropName("X-SPOUSE");
    props->add(*p);   
	
	p->setPropName("X-FUNAMBOL-BILLINGINFO");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-COMPANIES");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-FOLDER");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-GENDER");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-HOBBIES");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-INITIALS");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-LANGUAGES");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-MILEAGE");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-SUBJECT");
    props->add(*p);

	p->setPropName("X-FUNAMBOL-ORGANIZATIONALID");
    props->add(*p);

    p->setPropName("X-FUNAMBOL-YOMICOMPANYNAME");
    props->add(*p);

    p->setPropName("X-FUNAMBOL-YOMIFIRSTNAME");
    props->add(*p);

    p->setPropName("X-FUNAMBOL-YOMILASTNAME");
    props->add(*p);

    delete p; p = NULL;
    delete pp; pp = NULL;

    return props;
}

ArrayList* getVTodoProperties(){

    ArrayList* props = new ArrayList();
    Property* p = new Property();
    ArrayList valenums;
    StringBuffer val;
	
	p->setPropName("BEGIN");
    val = "VCALENDAR";
    valenums.add(val);
	val = "VTODO";
	valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);   

    valenums.clear();

    p->setPropName("END");
    val = "VTODO";
    valenums.add(val);
	val = "VCALENDAR";
    valenums.add(val);	
    p->setValEnums(&valenums);
    props->add(*p);

    valenums.clear();
    
    p->setPropName("VERSION");
    val = "1.0";
    valenums.add(val);
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();

	p->setPropName("CLASS");
    val = "PUBLIC";
    valenums.add(val);
	val = "PRIVATE";
    valenums.add(val);
	val = "CONFIDENTIAL";
    valenums.add(val);	
    p->setValEnums(&valenums);
    props->add(*p);
	
	valenums.clear();	
	
	delete p; p = NULL;
    p = new Property();
    
    p->setPropName("X-FUNAMBOL-ALLDAY");
    props->add(*p);
    p->setPropName("X-FUNAMBOL-FOLDER");
    props->add(*p);
    p->setPropName("DESCRIPTION");
    props->add(*p);
    p->setPropName("CATEGORIES");
    props->add(*p);
    p->setPropName("STATUS");
    props->add(*p);
    p->setPropName("COMPLETED");
    props->add(*p);
    p->setPropName("DUE");
    props->add(*p);
    p->setPropName("PRIORITY");
    props->add(*p);
    //p->setPropName("X-FUNAMBOL-AALARMOPTIONS");
    //props->add(*p);
    p->setPropName("AALARM");
    props->add(*p);
    p->setPropName("CLASS");
    props->add(*p);
    p->setPropName("DTSTART");
    props->add(*p);
    p->setPropName("SUMMARY");
    props->add(*p);
    p->setPropName("X-FUNAMBOL-TEAMTASK");
    props->add(*p);
    p->setPropName("RRULE");
    props->add(*p);
	p->setPropName("PERCENT-COMPLETE");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-ACTUALWORK");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-BILLINGINFO");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-COMPANIES");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-MILEAGE");
    props->add(*p);
	p->setPropName("X-FUNAMBOL-TOTALWORK");
    props->add(*p);

    delete p; p = NULL;

    return props;
}

ArrayList* getNoteProperties(){

    ArrayList* p = new ArrayList();
	
    //adding sif
    Property subject,body;
    subject.setPropName("Subject");
    body.setPropName("Body");
    p->add(subject);
    p->add(body);
   
    return p;
}

ArrayList* getVNoteProperties() {
	
	ArrayList* p = new ArrayList();

    Property pval;
	Property subject, body;

    subject.setPropName("SUBJECT");
    body.setPropName("BODY");
    ArrayList valenums;
    StringBuffer val;

    pval.setPropName("BEGIN");
    val = "VNOTE";
    valenums.add(val);
    pval.setValEnums(&valenums);
    p->add(pval);

    valenums.clear();

    pval.setPropName("END");
    val = "VNOTE";
    valenums.add(val);
    pval.setValEnums(&valenums);
    p->add(pval);

    valenums.clear();

    pval.setPropName("VERSION");
    val = "1.1";
    valenums.add(val);
    pval.setValEnums(&valenums);
    p->add(pval);
    p->add(subject);
    p->add(body);
   // s->addCtCap(p, "text/x-vnote", "1.1");
    
	return p;

}

wstring getEmptyVCard() {

    wstring emptyVCard;
    emptyVCard.append(L"BEGIN:VCARD\r\n\r\n");
    emptyVCard.append(L"VERSION:2.1\r\n");
    emptyVCard.append(L"N:;;;;\r\n");
    emptyVCard.append(L"BDAY:\r\n");
    emptyVCard.append(L"NOTE:\r\n");
    emptyVCard.append(L"TEL;WORK;FAX:\r\n");
    emptyVCard.append(L"TEL;VOICE;WORK:\r\n");
    emptyVCard.append(L"TEL;VOICE;WORK:\r\n");
    emptyVCard.append(L"TEL;CAR;VOICE:\r\n");
    emptyVCard.append(L"CATEGORIES:\r\n");
    emptyVCard.append(L"TEL;WORK;PREF:\r\n");
    emptyVCard.append(L"FN:\r\n");
    emptyVCard.append(L"EMAIL;INTERNET:\r\n");
    emptyVCard.append(L"EMAIL;INTERNET;HOME:\r\n");
    emptyVCard.append(L"EMAIL;INTERNET;WORK:\r\n");
    emptyVCard.append(L"TITLE:\r\n");
    emptyVCard.append(L"TEL;VOICE;HOME:\r\n");
    emptyVCard.append(L"TEL;VOICE;HOME:\r\n");
    emptyVCard.append(L"TEL;HOME;FAX:\r\n");
    emptyVCard.append(L"URL;HOME:\r\n");
    emptyVCard.append(L"PRIORITY:1\r\n");
    emptyVCard.append(L"TEL;CELL:\r\n");
    emptyVCard.append(L"NICKNAME:\r\n");
    emptyVCard.append(L"TEL;FAX:\r\n");
    emptyVCard.append(L"TEL;VOICE:\r\n");
    emptyVCard.append(L"TEL;PAGER:\r\n");
    emptyVCard.append(L"TEL;PREF;VOICE:\r\n");
    emptyVCard.append(L"ROLE:\r\n");
    emptyVCard.append(L"CLASS:PUBLIC\r\n");
    emptyVCard.append(L"URL:\r\n");
    emptyVCard.append(L"ORG:;;\r\n");
    emptyVCard.append(L"ADR;HOME:;;;;;;\r\n");
    emptyVCard.append(L"ADR:;;;;;;\r\n");
    emptyVCard.append(L"ADR;WORK:;;;;;;\r\n");
    emptyVCard.append(L"PHOTO:\r\n");
    emptyVCard.append(L"X-ANNIVERSARY:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-BILLINGINFO:\r\n");
    emptyVCard.append(L"TEL;X-FUNAMBOL-CALLBACK:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-CHILDREN:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-COMPANIES:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-FOLDER:DEFAULT_FOLDER\r\n");
    emptyVCard.append(L"X-FUNAMBOL-HOBBIES:\r\n");
    emptyVCard.append(L"EMAIL;INTERNET;HOME;X-FUNAMBOL-INSTANTMESSENGER:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-INITIALS:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-LANGUAGES:\r\n");
    emptyVCard.append(L"X-MANAGER:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-MILEAGE:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-ORGANIZATIONALID:\r\n");
    emptyVCard.append(L"TEL;X-FUNAMBOL-RADIO:\r\n");
    emptyVCard.append(L"X-SPOUSE:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-SUBJECT:\r\n");
    emptyVCard.append(L"TEL;X-FUNAMBOL-TELEX:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-YOMICOMPANYNAME:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-YOMIFIRSTNAME:\r\n");
    emptyVCard.append(L"X-FUNAMBOL-YOMILASTNAME:\r\n");
    emptyVCard.append(L"END:VCARD");
    return emptyVCard;
}
