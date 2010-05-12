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
#include "base/stringUtils.h"
#include "base/timeUtils.h"
#include "winmaincpp.h"
#include "utils.h"

#include "outlook/defs.h"
#include "outlook/ClientAppointment.h"
#include "outlook/ClientException.h"
#include "outlook/ClientFolder.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

using namespace std;



/* 
 * Constructor:
 * Initialize class members.
 * Instance the Redemption COM pointer.
*/
ClientAppointment::ClientAppointment() : ClientItem() {

    pAppointment     = NULL;
    pSafeAppointment = NULL;

    createSafeAppointmentInstance();
    memset((void*)&timeZoneInfo, 0, sizeof(timeZoneInfo));
}

// Destructor
ClientAppointment::~ClientAppointment() {
    if (pAppointment)     { pAppointment.Release();     }
    if (pSafeAppointment) { pSafeAppointment.Release(); }
}



/*
 * Set a COM pointer to this object.
 * ------------------------------------
 * This method is used to link the object to the correspondent
 * outlook COM pointer. All class members are overwrited by this call.
 * The method MUST be called before using this object, as the constructor
 * doesn't link the class COM pointer.
 * This method also links Redemption COM pointer to the item.
 * If 'itemID' parameter is an empty string, then ID is derived
 * from the entryID of the outlook item.
 */
void ClientAppointment::setCOMPtr(_AppointmentItemPtr& ptr, const wstring& itemID) {

    pAppointment = ptr;

    try {
        pSafeAppointment->Item = pAppointment;

        pUserProperties = pAppointment->UserProperties;
        userPropertiesCount = pUserProperties->Count;

        pItemProperties = pAppointment->ItemProperties;
        propertiesCount = pItemProperties->Count;

        propertiesCount -= userPropertiesCount;

        if (propertiesCount) {
            pItemProperty = pItemProperties->Item(0);
        }

        // ID passed as parameter
        if (itemID != EMPTY_WSTRING) {
            ID = itemID;
        }
        // ID retrieved from Outlook
        else {
            _bstr_t bstrID = pAppointment->GetEntryID();
            // ID could not exist (if item not saved)
            if (bstrID.GetBSTR()) {
                ID = (WCHAR*)bstrID;
            }
            else {
                ID = EMPTY_WSTRING;
            }
        }
        // Rec Pattern
        initializeRecPattern();

        // Full path of parent folder
        // Outlook returns "%5C" instead of "\" and "%2F" instead of "/"
        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pAppointment->GetParent();
        if (parentFolder) {
            parentPath = (WCHAR*)parentFolder->GetFullFolderPath();
            // "%5C" is kept and used to escape the "\" char
            replaceAll(L"%2F", L"/",     parentPath);
        }
    }

    catch(_com_error &e) {
        manageComErrors(e);
        throwClientFatalException(ERR_OUTLOOK_ITEM_ASSIGN);
    }

    hr              = S_OK;
    itemType        = APPOINTMENT;
    propertiesIndex = 0;

    // Assume all items of this type have same props!
    //propertyMap.clear();
}


// Here ID is derived from the entryID of the item.
void ClientAppointment::setCOMPtr(_AppointmentItemPtr& ptr) {
    return setCOMPtr(ptr, EMPTY_WSTRING);
}


/**
 * Returns a reference to the internal COM pointer.
 */
_AppointmentItemPtr& ClientAppointment::getCOMPtr() {
    return pAppointment;
}



/*
 * Creates an instance for the Redemption COM pointer.
 */
void ClientAppointment::createSafeAppointmentInstance() {

    try {
        hr = pSafeAppointment.CreateInstance(RED_SAFEAPPOINTMENT);
        if (FAILED(hr)) {
            throwClientFatalException(ERR_OUTLOOK_SAFEITEM);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientFatalException(ERR_OUTLOOK_SAFEITEM);
    }
}


/*
 * Initialize Recurrence Pattern object.
 *   - initialize members
 *   - link COM Ptr (if exists)
 *   - set infos for the change-day of rec pattern
 */
void ClientAppointment::initializeRecPattern() {

    RecurrencePatternPtr rp;

    //
    // This is an item retrieved (not empty).
    //
    if (ID != EMPTY_WSTRING) {
        BOOL isRec = vBoolToBOOL(pAppointment->GetIsRecurring());
        
        //rp = pAppointment->GetRecurrencePattern();
        //if (isRec)  recPattern.setRecurrence  ();
        //else        recPattern.clearRecurrence();

        if (isRec) {
            rp = pAppointment->GetRecurrencePattern();
            recPattern.setRecurrence();
        }
        else {
            rp = NULL;
            recPattern.clearRecurrence();
        }

        // This is used for the change-day option
        recPattern.setStart(getComplexProperty(L"Start"));
        recPattern.setIsAllDay(isAllDayEvent());
    }

    //
    // This is a new empty item (add): rec pattern not exist yet!!
    // Note: calling 'getRecPattern()' will lead to errors...
    //
    else {
        rp = NULL;
        recPattern.clearRecurrence();
    }

    //
    // Link the COM Ptr (NULL if new item)
    //
    recPattern.setCOMPtr(rp);
}



/**
 * Returns a pointer to the internal ClientRecurrence object 
 * Returns NULL if it's not recurring.
 */
ClientRecurrence* ClientAppointment::getRecPattern() {

    bool isRec = recPattern.isRecurring();
    return (isRec? &recPattern : NULL);
}


/**
 * Used to clear recurrence pattern. 
 * Also removes any appointment exception (if exists).
 * Throws a ClientException if operation fails.
 * Returns 0 if no errors.
 */
int ClientAppointment::clearRecPattern() {

    try {
        hr = pAppointment->ClearRecurrencePattern();
        if (FAILED(hr)) {
            goto error;
        }
        recPattern.clearRecurrence();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    return 0;

error: 
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_CLEAR_REC, itemType.c_str(), getSafeItemName(this).c_str());
    throwClientException(getLastErrorMsg());
    return 1;
}




/*
 * Save the current appointment item.
 *
 * Notes:
 * - exceptions can be saved only if the appointment is already saved
 * - saving the recurrence pattern results in losing all exceptions (if any),
 *   so we must save the appointment before saving exceptions
 *
 * @return: 0 if no errors.
 */
int ClientAppointment::saveItem() {
    
    try {                                     

        // 1. Save recurrence pattern if needed.
        if (recPattern.isRecurring()) {                        
            if (recPattern.save()) {
                goto error;
            }           
        }                 
       
        // 2. Save the appointment item.
        hr = pAppointment->Save();
        if (FAILED(hr)) {
            goto error;
        }

        // 3. If the appointment is recurring set the timezone
        if (recPattern.isRecurring() && recPattern.getHasTimezone()) {
            ClientApplication* cApp = ClientApplication::getInstance();
            bool isTheSame = cApp->isTheSameTimezoneRule(getRecurringTimezone());
            if (!isTheSame) {
                cApp->setTimezone(this);
                cApp->setStartAndEnd(this, localStartDate, localEndDate);
            }
        }  
 
                                

    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    try {
        // 3. Get the new entry-ID.
        // *** Note ***
        // GetEntryID() is catched apart because it may happen that Outlook throws a
        // wrong COM exception (it happens if ALL occurrences of a recurring event
        // are exceptions...) but the ID is correclty returned.   Microsoft bug?
        // So we catch silently here any exception, exit only if ID is empty.
        // *** Note ***
        // This is done BEFORE saveAllExceptions() because a funny user could delete
        // all occurrences as exceptions, and so the ID could be not available...
        ID = (WCHAR*)pAppointment->GetEntryID();
    }
    catch(_com_error &e) {

        if (!ID.length()) {
            LOG.error("Error saving item: ID of item saved is empty!");
            manageComErrors(e);
            goto error;
        }
    }

    try {
        // 4. Save exceptions (MUST: after saving the item).
        if (recPattern.saveAllExceptions()) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    return 0;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_SAVE, itemType.c_str());
    throwClientException(getLastErrorMsg());
    return 1;
}


/*
 * Delete the current Item.
 * @return: 0 if no errors.
 */
int ClientAppointment::deleteItem() {
    
    try {
        hr = pAppointment->Delete();
        if (FAILED(hr)) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    return 0;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_DELETE, itemType.c_str(), ID.c_str());
    throwClientException(getLastErrorMsg());
    return 1;
}



/**
 * Returns a (new allocated) copy of this item.
 * @return : a pointer to the new ClientItem object (must bee deleted by the caller).
 *           NULL in case of errors.
 */
ClientItem* ClientAppointment::copyItem() {

    IDispatchPtr        pNew    = NULL;
    _AppointmentItemPtr pAppNew = NULL;
    ClientAppointment*  cNew    = NULL;

    if (!pAppointment) {
        goto error;
    }

    try {
        // Copy the COM pointer
        pNew = pAppointment->Copy();
        if (!pNew) {
            goto error;
        }
        pAppNew = (_AppointmentItemPtr)pNew;
        if (!pAppNew) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Set the new COM pointer to a new allocated ClientAppointment.
    cNew = new ClientAppointment();
    cNew->setCOMPtr(pAppNew);

    if (cNew->getRecPattern()) {
        cNew->getRecPattern()->read();
    }

    return (ClientItem*)cNew;

error:
    return NULL;
}



/**
 * Moves this ClientAppointment into the passed destination folder.
 * The move operation changes only the item location in Outlook, so
 * the item's ID is preserved.
 *
 * @param   destFolder  the destination ClientFolder to move this object to
 * @return              0 if no errors
 */
int ClientAppointment::moveItem(ClientFolder* destFolder) {

    if (!pAppointment) {
        goto error;
    }

    // Get destination folder
    MAPIFolder* pDestFolder = destFolder->getCOMPtr();
    if (!pDestFolder) {
        goto error;
    }

    // Move item
    try {
        this->setCOMPtr((_AppointmentItemPtr)pAppointment->Move(pDestFolder));
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    parentPath = destFolder->getPath();
    return 0;

error:
    LOG.error("Error moving item '%ls'", getSafeItemName(this));
    return 1;
}




// Always retrieve the updated property.
bool ClientAppointment::isAllDayEvent() {
    VARIANT_BOOL allday = pAppointment->GetAllDayEvent();
    return vBoolToBool(allday);
}


//
// ------------------------------ Methods to manage item properties -----------------------------
//

/*
 * Return true if the property is protected by Outlook Security patch.
 * Protected properties are listed in safeCalendarProps array.
 */
bool ClientAppointment::isSecureProperty(const wstring& propertyName) {
    
    for (int i=0; safeCalendarProps[i]; i++) {
        if (propertyName == safeCalendarProps[i]) {
            return true;
        }
    }
    return false;
}

/*
 * Return true if the property needs some specific conversion of data.
 * Complex properties are listed in complexCalendarProps array.
 * Recurrence properties are listed in recurrenceProps array.
 */
bool ClientAppointment::isComplexProperty(const wstring& propertyName) {
    
    for (int i=0; complexCalendarProps[i]; i++) {
        if (propertyName == complexCalendarProps[i]) {
            return true;
        }
    }
    for (int i=0; recurrenceProps[i]; i++) {
        if (propertyName == recurrenceProps[i]) {
            return true;
        }
    }
    return false;
}



// ------------------------------- GET PROPERTY -------------------------------
/*
 * Return the Item property value from the property name.
 * Use appropriate Redemption function for each property.
 * Note: this method uses the redemption library.
 *
 * @param propertyName  : the name of the property
 * @return              : the value retrieved (from Outlook)
 */
const wstring ClientAppointment::getSafeProperty(const wstring& propertyName) {
    
    wstring propertyValue = EMPTY_WSTRING;
    BSTR tmpVal;

    if (propertyName == L"Body") {
        hr = pSafeAppointment->get_Body(&tmpVal);
    }
    //else if (propertyName == L"OptionalAttendees") {             // not used since 6.6.0
    //    hr = pSafeAppointment->get_OptionalAttendees(&tmpVal);
    //}

    if (FAILED(hr)) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE, itemType.c_str(), propertyName.c_str());
        throwClientException(getLastErrorMsg());
    }
    if (tmpVal) {
        propertyValue = tmpVal;

        // Manage error in API to read item's body: one newline is always appended.
        if (propertyName == L"Body") {
            removeLastNewLine(propertyValue);
        }
    }

    return propertyValue;
}




/*
 * Return the Item property value from the property name.
 * Here are processed properties that need specific conversion of 
 * the property value. Also recurrence properties are retrieved here.
 *
 * @param propertyName  : the name of the property
 * @return              : the value retrieved (from Outlook)
 */
const wstring ClientAppointment::getComplexProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    WCHAR tmp[64];
    VARIANT_BOOL vbool;
    DATE date;

    //
    // Date
    //
    if (propertyName == L"Start") {                             // All day: "yyyyMMdd"
        date = pAppointment->GetStart();                        // else   : "YYYYMMDDThhmmssZ"
        doubleToSystemTime(propertyValue, date, USE_UTC, isAllDayEvent());
    }
    else if (propertyName == L"End") {                          // All day: "yyyyMMdd"
        date = pAppointment->GetEnd();                          // else   : "YYYYMMDDThhmmssZ"
        doubleToSystemTime(propertyValue, date, USE_UTC, isAllDayEvent());
    }
    else if (propertyName == L"ReplyTime") {                    // "YYYYMMDDThhmmssZ"
        date = pAppointment->GetReplyTime();
        if (date && date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, USE_UTC);
        }
        else {
            propertyValue = EMPTY_WSTRING;
        }
    }
    else if (propertyName == L"LastModificationTime") {         // "(double format)"
        date = pAppointment->GetLastModificationTime();
        swprintf_s(tmp, L"%.12f", date);
        propertyValue = tmp;
    }

    //
    // Boolean
    //
    else if (propertyName == L"AllDayEvent") {
        wsprintf(tmp, TEXT("%d"), isAllDayEvent());
        propertyValue = tmp;
    }
    else if (propertyName == L"IsRecurring") {
        vbool = pAppointment->GetIsRecurring();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp;    
    }
    else if (propertyName == L"NoAging") {
        vbool = pAppointment->GetNoAging();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp;   
    }
    else if (propertyName == L"ReminderSet") {
        vbool = pAppointment->GetReminderSet();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp;   
    }
    //else if (propertyName == L"UnRead") {         // not used since 6.6.0
    //    vbool = pAppointment->GetUnRead();
    //    wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
    //    propertyValue = tmp;
    //}

    // Separator for Categories in Outlook can be "," or ";".
    // We use only ",".
    else if (propertyName == L"Categories") {
        _bstr_t categories = pAppointment->GetCategories();
        if (categories.length() > 0) {
            propertyValue = (WCHAR*)categories;
            replaceAll(L";", L",", propertyValue);
        }
    }

    // Get ReminderSoundFile ONLY IF ReminderPlaySound is true
    else if (propertyName == L"ReminderSoundFile") {
        bool playSound = vBoolToBool(pAppointment->GetReminderPlaySound());
        if (playSound) {
            _bstr_t path = pAppointment->GetReminderSoundFile();
            if (path.length()) {
                propertyValue = path;
            }
        }
    }
/*
    // Get the BusyStatus. It should be a simple property but in the receiving we must check
    // if its value is a numeric one or a literal one (it must be 1, 2 not FREE, TENTATIVE...)
    else if (propertyName == L"BusyStatus") {
        OlBusyStatus value = pAppointment->GetBusyStatus();   
        wchar_t tmp[10];
        wsprintf(tmp, TEXT("%i"), value);                
        propertyValue = tmp;
    }
*/
    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_NOT_FOUND, propertyName.c_str(), itemType.c_str());
        throwClientException(getLastErrorMsg());
        return EMPTY_WSTRING;
    }

    return propertyValue;
}


DATE ClientAppointment::getCreationTime() {

    try {
        if (pAppointment) {
            return pAppointment->GetCreationTime();
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
    }
    return NULL;
}



// ------------------------------- SET PROPERTY -------------------------------
/*
 * Set the Item property value for the specific property name.
 * Here are processed properties that need specific conversion of the property value. 
 * Also recurrence properties are set from here.
 *
 * @param propertyName  : the name of the property
 * @param propertyValue : the value to store
 * @return              : 0 if no errors, 1 if errors
 */
int ClientAppointment::setComplexProperty(const wstring& propertyName, const wstring& propertyValue) {
    
    int  ret = 0;
    VARIANT_BOOL vbool;

    // Default values used if propertyValue = empty.
    DATE date     = 0;
    int  intValue = 0;

    int len = propertyValue.length();

    //
    // Date
    //
    if (propertyName == L"Start") {
        if (!len) {
            // this is required!
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_REQUIRED, L"Start", itemType.c_str());
            return 1;
        }
        systemTimeToDouble(propertyValue, &date);
        hr = pAppointment->put_Start(date);

        // This is used for the change-day option
        recPattern.setStart(propertyValue);
        localStartDate = date;
    }

    else if (propertyName == L"End") {
        if (!len) {
            // this is required!
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_REQUIRED, L"End", itemType.c_str());
            return 1;
        } 
        systemTimeToDouble(propertyValue, &date);
        hr = pAppointment->put_End(date);

        // This is used for the EndTime calculation when using apppointment
        recPattern.setEnd(propertyValue);
        localEndDate = date;
    }

    else if (propertyName == L"ReplyTime") {
        date = 0;
        if (len) {
            systemTimeToDouble(propertyValue, &date);
        }
        hr = pAppointment->put_ReplyTime(date);                       // Use '0' to clear
    }

    //
    // Boolean
    //
    else if (propertyName == L"AllDayEvent") {
        intValue = 1;   // as a default
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pAppointment->put_AllDayEvent(vbool);

        // Rec pattern must know if it's an allday!
        recPattern.setIsAllDay(intValue);
    }

    else if (propertyName == L"IsRecurring") {
        BOOL isRec = FALSE;   // as a default
        if (len) {
            isRec = _wtoi(propertyValue.c_str());
        }
        // MUST set the 'recurring' value and link COM Ptr.
        if (isRec) {
            recPattern.setRecurrence();
            recPattern.setCOMPtr(pAppointment->GetRecurrencePattern());
        }
        else {
            recPattern.clearRecurrence();
        }
    }

    else if (propertyName == L"NoAging") {
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pAppointment->put_NoAging(vbool);
    }

    else if (propertyName == L"ReminderSet") {
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pAppointment->put_ReminderSet(vbool); 
    }

    //else if (propertyName == L"UnRead") {         // not used since 6.6.0
    //    if (len) {
    //        intValue = _wtoi(propertyValue.c_str());
    //    }
    //    vbool = BOOLToVBool(intValue);
    //    hr = pAppointment->put_UnRead(vbool); 
    //}

    // Separator for Categories in Outlook can be "," or ";".
    // Nothing to do (both accepted by Outlook).
    else if (propertyName == L"Categories") {
        pAppointment->PutCategories(propertyValue.c_str());
    }

    // Also set the ReminderPlaySound if ReminderSoundFile exists
    else if (propertyName == L"ReminderSoundFile") {
        if (propertyValue.length() > 0) {
            hr = pAppointment->put_ReminderPlaySound(VARIANT_TRUE);
            _bstr_t bstrValue = (_bstr_t)propertyValue.c_str();
            hr = pAppointment->put_ReminderSoundFile(bstrValue);
        }
        else {
            hr = pAppointment->put_ReminderPlaySound(VARIANT_FALSE);
        }
    }
/*
    // Set the busy status. Check if the value is one the possible in a digit format.
    // otherwise convert it
    // possible values are olBusy(2), olFree(0), olOutOfOffice(3), or olTentative(1). 
    else if (propertyName == L"BusyStatus") {
        int value = 0; // FREE        
        if (propertyValue.length() > 0) {
            if (propertyValue == L"BUSY" || propertyValue == L"2") {
                value = 2;
            } else if (propertyValue == L"TENTATIVE" || propertyValue == L"1") {
                value = 1;
            } else if (propertyValue == L"OOF" || propertyValue == L"3") { // Out of office
                value = 3;
            }            
        }        
        pAppointment->PutBusyStatus((OlBusyStatus)value);                    
    }
*/
    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_NOT_FOUND, propertyName.c_str(), itemType.c_str());
        throwClientException(getLastErrorMsg());
        return 1;
    }

    if (FAILED(hr)) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_SET, propertyName.c_str(), propertyValue.c_str(), itemType.c_str());
        throwClientException(getLastErrorMsg());
        ret = 1;
    }
    return ret;
}








/* 
 * Copy Constructor
 */
ClientAppointment::ClientAppointment(const ClientAppointment& c) {

    hr              = c.hr;
    ID              = c.ID;
    itemType        = c.itemType;
    parentPath      = c.parentPath;
    pAppointment    = c.pAppointment;
    pItemProperties = c.pItemProperties;
    pItemProperty   = c.pItemProperty;
    propertiesIndex = c.propertiesIndex;
    propertiesCount = c.propertiesCount;
    propertyMap     = c.propertyMap;

    createSafeAppointmentInstance();
    pSafeAppointment= c.pSafeAppointment;
    recPattern      = c.recPattern;
    timeZoneInfo    = c.timeZoneInfo;

    
    userPropertyMap     = c.userPropertyMap;
    userPropertiesCount = c.userPropertiesCount;
}


/* 
 * Operator =
*/
ClientAppointment ClientAppointment::operator=(const ClientAppointment& c) {

    ClientAppointment cnew;

    cnew.hr              = c.hr;
    cnew.ID              = c.ID;
    cnew.itemType        = c.itemType;
    cnew.parentPath      = c.parentPath;
    cnew.pAppointment    = c.pAppointment;
    cnew.pItemProperties = c.pItemProperties;
    cnew.pItemProperty   = c.pItemProperty;
    cnew.propertiesIndex = c.propertiesIndex;
    cnew.propertiesCount = c.propertiesCount;
    cnew.propertyMap     = c.propertyMap;

    cnew.createSafeAppointmentInstance();
    cnew.pSafeAppointment= c.pSafeAppointment;
    cnew.recPattern      = c.recPattern;

    

    cnew.userPropertyMap     = c.userPropertyMap;
    cnew.userPropertiesCount = c.userPropertiesCount;

    return cnew;
}


//void ClientAppointment::test() {
//
//    HRESULT hres;
//
//
//    try {
//        
//        hres = pAppointment->put_AllDayEvent(VARIANT_TRUE);
//        hres = pAppointment->put_Start(39010.000000);
//        hres = pAppointment->put_End  (39011.000000);
//        //hres = pAppointment->ClearRecurrencePattern();
//        pAppointment->PutSubject(L"Test");
//        pAppointment->PutBusyStatus(olBusy);
//        pAppointment->PutImportance(olImportanceLow);
//        pAppointment->PutMeetingStatus(olNonMeeting);
//        pAppointment->PutReminderMinutesBeforeStart(0);
//        pAppointment->PutReminderSet(VARIANT_FALSE);
//        pAppointment->PutSensitivity(olNormal);
//
//
//        //pAppointment->Display();
//
//        hres = pAppointment->Save();
//
//
//        DATE st, en;
//        VARIANT_BOOL ad;
//        _bstr_t sj;
//        hres = pAppointment->get_Start(&st);
//        hres = pAppointment->get_End  (&en);
//        RecurrencePatternPtr p = pAppointment->GetRecurrencePattern();
//        hres = pAppointment->get_AllDayEvent(&ad);
//        sj = pAppointment->GetSubject();
//        LOG.debug("test done");
//    }
//
//    catch(_com_error &e) {
//        manageComErrors(e);
//    }
//}


void ClientAppointment::clearAttendees()
{
    Redemption::ISafeRecipientsPtr pRecipients = pSafeAppointment->GetRecipients();
    int countRecipient = pRecipients->GetCount();
    for(int i = countRecipient;i>0;i--) {
        pRecipients->Remove(i);
    }
}

bool ClientAppointment::addAttendee(const ClientRecipient & recipient)
{
    bool success = FALSE;

    Redemption::ISafeRecipientsPtr temp = pSafeAppointment->GetRecipients();
    Redemption::ISafeRecipientPtr rec;

    if (temp) {
        rec = temp->Add(recipient.getNamedEmail().c_str());
        success = rec->Resolve(false) == TRUE;
    }

    return success;
}

int ClientAppointment::getNumAttendees() {

    Redemption::ISafeRecipientsPtr pRecipients = pSafeAppointment->GetRecipients();
    int countRecipients = pRecipients->GetCount();
    return countRecipients;
}

ClientRecipient ClientAppointment::getAttendee(int index)
{
    Redemption::ISafeRecipientsPtr pRecipients = pSafeAppointment->GetRecipients();
    int countRecipients = pRecipients->GetCount();
    if (index >= countRecipients) {
        throwClientException("Invalid recipient index");
    }
    return ClientRecipient(pRecipients->Item(index));
}

bool ClientAppointment::removeAttendee(int index)
{
    Redemption::ISafeRecipientsPtr pRecipients = pSafeAppointment->GetRecipients();
    int countRecipients = pRecipients->GetCount();
    if (index < countRecipients) {
        pRecipients->Remove(index);
        return true;
    }
    return false;
}

std::map<int, ClientRecipient> ClientAppointment::getAttendees()
{
    std::map<int, ClientRecipient> attendees;

    Redemption::ISafeRecipientsPtr pRecipients = pSafeAppointment->GetRecipients();
    int countRecipient = pRecipients->GetCount();

    for(int i = countRecipient;i>0;i--) {
        attendees[i] = ClientRecipient(pRecipients->Item(i));
    }

    return attendees;
}

