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
#include "winmaincpp.h"
#include "utils.h"

#include "outlook/defs.h"
#include "outlook/ClientApplication.h"
#include "outlook/ClientTask.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

using namespace std;


/* 
 * Constructor:
 * Dummy initialize class members.
 * Instance the Redemption COM pointer.
*/
ClientTask::ClientTask() : ClientItem() {

    pTask           = NULL;
    pSafeTask       = NULL;

    createSafeTaskInstance();
}

// Destructor
ClientTask::~ClientTask() {
    if (pTask)     { pTask.Release();     }
    if (pSafeTask) { pSafeTask.Release(); }
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
void ClientTask::setCOMPtr(_TaskItemPtr& ptr, const wstring& itemID) {

    pTask = ptr;

    try {
        pSafeTask->Item = pTask;

        pItemProperties = pTask->ItemProperties;
        propertiesCount = pItemProperties->Count;
        if (propertiesCount) {
            pItemProperty = pItemProperties->Item(0);
        }
        // ID passed as parameter
        if (itemID != EMPTY_WSTRING) {
            ID = itemID;
        }
        // ID retrieved from Outlook
        else {
            _bstr_t bstrID = pTask->GetEntryID();
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
        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pTask->GetParent();
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
    itemType        = TASK;
    propertiesIndex = 0;

    // Assume all items of this type have same props!
    //propertyMap.clear();
}


// Here ID is derived from the entryID of the item.
void ClientTask::setCOMPtr(_TaskItemPtr& ptr) {
    return setCOMPtr(ptr, EMPTY_WSTRING);
}


/**
 * Returns a reference to the internal COM pointer.
 */
_TaskItemPtr& ClientTask::getCOMPtr() {
    return pTask;
}



/*
 * Creates an instance for the Redemption COM pointer.
 */
void ClientTask::createSafeTaskInstance() {

    try {
        hr = pSafeTask.CreateInstance(RED_SAFETASK);
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
 *
 * Note: GetRecurrencePattern() MUST NOT be called if the task is 
 * ----- not recurring, otherwise RecurrencePattern will be created!! 
 *      (and so IsRecurring will return true) ;)
 */
void ClientTask::initializeRecPattern() {

    RecurrencePatternPtr rp;

    //
    // This is an item retrieved (not empty).
    //
    if (ID != EMPTY_WSTRING) {
        BOOL isRec = vBoolToBOOL(pTask->GetIsRecurring());

        if (isRec) {
            rp = pTask->GetRecurrencePattern();
            recPattern.setRecurrence();
        }
        else {
            rp = NULL;
            recPattern.clearRecurrence();
        }

        // StartDate format is "yyyyMMdd" so we won't use the change-day.
        recPattern.setStart(getComplexProperty(L"StartDate"));
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
ClientRecurrence* ClientTask::getRecPattern() {

    BOOL isRec = vBoolToBOOL(pTask->GetIsRecurring());
    return (isRec? &recPattern : NULL);
}


/**
 * Used to clear recurrence pattern. 
 * Throws a ClientException if operation fails.
 * Returns 0 if no errors.
 */
int ClientTask::clearRecPattern() {

    try {
        hr = pTask->ClearRecurrencePattern();
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
    setErrorF(0, ERR_OUTLOOK_CLEAR_REC, itemType.c_str(), getSafeItemName(this).c_str());
    throwClientException(getLastErrorMsg());
    return 1;
}



/*
 * Save the current Item.
 * @return: 0 if no errors.
 */
int ClientTask::saveItem() {
    
    // --- Adjustment for reminders ---
    // Task reminders can be saved ONLY inside default folder! (otherwise an error will be promped)
    // -> disable reminder, notify user in the LOG file.
    if (getProperty(L"ReminderSet") == L"1") {
        ClientApplication* ol = ClientApplication::getInstance();
        ClientFolder*  folder = ol->getDefaultFolder(itemType);         // WARNING! this changes the ClientApplication::folder object...
        wstring defFolderPath = folder->getPath();                      // (pay attention if any pointer was linked to it)
        if (parentPath != defFolderPath) {
            LOG.info(INFO_OUTLOOK_REMINDER_RESET, getProperty(L"Subject").c_str());
            setProperty(L"ReminderSet", L"0");
        }
    }

    try {
        // Save recurrence pattern if needed.
        if (recPattern.isRecurring()) {
            if (recPattern.save()) {
                goto error;
            }
        }

        hr = pTask->Save();
        if (FAILED(hr)) {
            goto error;
        }

        // This is just a workaround to fix an Outlook bug:
        // some recurring tasks are not correctly stored in Outlook, so they are recurring but the task's 
        // icon doesn't show the recurring arrows.
        // In this case, an update that removes the recurrence would fail (without errors from Outlook)
        // so we have to save twice the task: the 2nd save will work.
        // An example to create such a corrupted item: create a recurring task, then set the startDate < dueDate = today.
        // You will see 2 tasks created in Outlook, the one due today is corrupted.
        if (recPattern.isRecurring() == 0) {
            clearRecPattern();
            hr = pTask->Save();
            if (FAILED(hr)) {
                goto error;
            }
        }


        // Get the new entry-ID.
        ID = (WCHAR*)pTask->GetEntryID();
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
int ClientTask::deleteItem() {
    
    try {
        hr = pTask->Delete();
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
ClientItem* ClientTask::copyItem() {

    IDispatchPtr  pNew     = NULL;
    _TaskItemPtr  pTaskNew = NULL;
    ClientTask*   cNew     = NULL;

    if (!pTask) {
        goto error;
    }

    try {
        // Copy the COM pointer
        pNew = pTask->Copy();
        if (!pNew) {
            goto error;
        }
        pTaskNew = (_TaskItemPtr)pNew;
        if (!pTaskNew) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Set the new COM pointer to a new allocated ClientTask.
    cNew = new ClientTask();
    cNew->setCOMPtr(pTaskNew);

    return (ClientItem*)cNew;

error:
    return NULL;
}


/**
 * Moves this ClientTask into the passed destination folder.
 * The move operation changes only the item location in Outlook, so
 * the item's ID is preserved.
 *
 * @param   destFolder  the destination ClientFolder to move this object to
 * @return              0 if no errors
 */
//int ClientTask::moveItem(ClientFolder* destFolder) {
//
//    if (!pTask) {
//        goto error;
//    }
//
//    // Get destination folder
//    MAPIFolder* pDestFolder = destFolder->getCOMPtr();
//    if (!pDestFolder) {
//        goto error;
//    }
//
//    // Move item
//    try {
//        pTask->Move(pDestFolder);
//    }
//    catch(_com_error &e) {
//        manageComErrors(e);
//        goto error;
//    }
//
//    parentPath = destFolder->getPath();
//    return 0;
//
//error:
//    LOG.error("Error moving item '%ls'", ID.c_str());
//    return 1;
//}




//
// ------------------------------ Methods to manage item properties -----------------------------
//

/*
 * Return true if the property is protected by Outlook Security patch.
 * Protected properties are listed in safeTaskProps array.
 */
bool ClientTask::isSecureProperty(const wstring& propertyName) {
    
    for (int i=0; safeTaskProps[i]; i++) {
        if (propertyName == safeTaskProps[i]) {
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
bool ClientTask::isComplexProperty(const wstring& propertyName) {

    for (int i=0; complexTaskProps[i]; i++) {
        if (propertyName == complexTaskProps[i]) {
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
const wstring ClientTask::getSafeProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    BSTR tmpVal;

    if (propertyName == L"Body") {
        hr = pSafeTask->get_Body(&tmpVal);
    }
    //else if (propertyName == L"ContactNames") {
    //    hr = pSafeTask->get_ContactNames(&tmpVal);
    //}
    //else if (propertyName == L"Owner") {
    //    hr = pSafeTask->get_Owner(&tmpVal);
    //}

    if (FAILED(hr)) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE, itemType.c_str(), propertyName.c_str());
        throwClientException(getLastErrorMsg());
    }
    if (tmpVal) {
        propertyValue = tmpVal;
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
const wstring ClientTask::getComplexProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    WCHAR tmp[64];
    VARIANT_BOOL vbool;
    DATE date;

    //
    // Date
    //
    if (propertyName == L"DateCompleted") {                     // "yyyyMMdd"     <- **** different from 3.0! ****
        date = pTask->GetDateCompleted();
        if (date && date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, USE_UTC, true);
        }
    }
    else if (propertyName == L"DueDate") {                      // "yyyyMMdd"
        date = pTask->GetDueDate();
        if (date && date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, USE_UTC, true);
        }
    }
    else if (propertyName == L"ReminderTime") {                 // "YYYYMMDDThhmmss" we use local time! (new since 6.5.2)
        date = pTask->GetReminderTime();
        if (date && date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, 0, false);
        }
    }
    else if (propertyName == L"StartDate") {                    // "yyyyMMdd"
        date = pTask->GetStartDate();
        if (date && date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, USE_UTC, true);
        }
    }
    else if (propertyName == L"LastModificationTime") {         // "(double format)"
        date = pTask->GetLastModificationTime();
        swprintf_s(tmp, L"%.12f", date);
        propertyValue = tmp;
    }

    //
    // Boolean
    //
    else if (propertyName == L"Complete") {
        vbool = pTask->GetComplete();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp;   
    }
    else if (propertyName == L"Importance") {
        OlImportance importance = pTask->GetImportance();
        wsprintf(tmp, TEXT("%d"), importance);
        propertyValue = tmp;
    }
    else if (propertyName == L"IsRecurring") {
        // Retrieve buffered value, 'IsRecurring' in Outlook could be wrong
        // (RecurrencePattern is created if we call GetRecurrencePattern)
        wsprintf(tmp, TEXT("%d"), recPattern.isRecurring());
        propertyValue = tmp;  
    }
    else if (propertyName == L"ReminderSet") {
        vbool = pTask->GetReminderSet();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp; 
    }
    else if (propertyName == L"TeamTask") {
        vbool = pTask->GetTeamTask();
        wsprintf(tmp, TEXT("%d"), vBoolToBOOL(vbool));
        propertyValue = tmp; 
    }

    // Separator for Categories in Outlook can be "," or ";".
    // We use only ",".
    else if (propertyName == L"Categories") {
        _bstr_t categories = pTask->GetCategories();
        if (categories.length() > 0) {
            propertyValue = (WCHAR*)categories;
            replaceAll(L";", L",", propertyValue);
        }
    }
    // Get ReminderSoundFile ONLY IF ReminderPlaySound is true
    else if (propertyName == L"ReminderSoundFile") {
        bool playSound = vBoolToBool(pTask->GetReminderPlaySound());
        if (playSound) {
            _bstr_t path = pTask->GetReminderSoundFile();
            if (path.length()) {
                propertyValue = path;
            }
        }
    }

    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_NOT_FOUND, propertyName.c_str(), itemType.c_str());
        throwClientException(getLastErrorMsg());
        return EMPTY_WSTRING;
    }

    return propertyValue;
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
int ClientTask::setComplexProperty(const wstring& propertyName, const wstring& propertyValue) {

    int  ret = 0;
    VARIANT_BOOL vbool;

    // Default values used if propertyValue = empty.
    DATE date     = 0;
    int  intValue = 0;

    int len = propertyValue.length();

    //
    // Date
    //
    if (propertyName == L"DateCompleted") {
        systemTimeToDouble(propertyValue, &date);               // Expected "yyyyMMdd" (UTC also accepted)
        if (!date) date = REFERRED_MAX_DATE;                    // Use REFERRED_MAX_DATE to clear
        hr = pTask->put_DateCompleted(date);
    }

    else if (propertyName == L"DueDate") {
        systemTimeToDouble(propertyValue, &date);               // Expected "yyyyMMdd" (UTC also accepted)
        if (!date) date = REFERRED_MAX_DATE;                    // Use REFERRED_MAX_DATE to clear
        hr = pTask->put_DueDate(date);

        // RecPattern: 'patternStartDate' will be set = 'StartDate' if not empty,
        // otherwise = 'DueDate' if not empty.
        // ['DueDate' is set first, so 'StartDate' is stronger if both exist]
        if (propertyValue != EMPTY_WSTRING) {
            recPattern.setStart(propertyValue);
        }
    }

    else if (propertyName == L"ReminderTime") {
        systemTimeToDouble(propertyValue, &date);               // Expected "YYYYMMDDThhmmssZ"
        if (!date) date = REFERRED_MAX_DATE;                    // Use REFERRED_MAX_DATE to clear
        hr = pTask->put_ReminderTime(date);
    }

    else if (propertyName == L"StartDate") {
        systemTimeToDouble(propertyValue, &date);               // Expected "yyyyMMdd" (UTC also accepted)
        if (!date) date = REFERRED_MAX_DATE;                    // Use REFERRED_MAX_DATE to clear
        hr = pTask->put_StartDate(date);

        // RecPattern: 'patternStartDate' will be set = 'StartDate' if not empty,
        // otherwise = 'DueDate' if not empty.
        // ['DueDate' is set for first, so 'StartDate' is stronger if both exist]
        if (propertyValue != EMPTY_WSTRING) {
            recPattern.setStart(propertyValue);
        }
    }

    //
    // Boolean
    //
    else if (propertyName == L"Complete") {
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pTask->put_Complete(vbool);
    }

    else if (propertyName == L"IsRecurring") {
        BOOL isRec = FALSE;   // as a default
        if (len) {
            isRec = _wtoi(propertyValue.c_str());
        }
        // MUST set the 'recurring' value and link COM Ptr.
        if (isRec) {
            recPattern.setRecurrence();
            recPattern.setCOMPtr(pTask->GetRecurrencePattern());
        }
        else {
            recPattern.clearRecurrence();
        }
    }


    else if (propertyName == L"Importance") {
        // Just to add safe checks (many times PRIORITY is a wrong value)
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        intValue = min(2, intValue);
        intValue = max(0, intValue);
        OlImportance importance = (OlImportance)intValue;
        hr = pTask->put_Importance(importance);
    }
    else if (propertyName == L"ReminderSet") {
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pTask->put_ReminderSet(vbool); 
    }
    else if (propertyName == L"TeamTask") {
        if (len) {
            intValue = _wtoi(propertyValue.c_str());
        }
        vbool = BOOLToVBool(intValue);
        hr = pTask->put_TeamTask(vbool); 
    }


    // Separator for Categories in Outlook can be "," or ";".
    // Nothing to do (both accepted by Outlook).
    else if (propertyName == L"Categories") {
        pTask->PutCategories(propertyValue.c_str());
    }

    // Also set the ReminderPlaySound if ReminderSoundFile exists
    else if (propertyName == L"ReminderSoundFile") {
        if (propertyValue.length() > 0) {
            hr = pTask->put_ReminderPlaySound(VARIANT_TRUE);
            _bstr_t bstrValue = (_bstr_t)propertyValue.c_str();
            hr = pTask->put_ReminderSoundFile(bstrValue);
        }
        else {
            hr = pTask->put_ReminderPlaySound(VARIANT_FALSE);
        }
    }


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
ClientTask::ClientTask(const ClientTask& c) {
    hr              = c.hr;
    ID              = c.ID;
    itemType        = c.itemType;
    parentPath      = c.parentPath;
    pTask           = c.pTask;
    pItemProperties = c.pItemProperties;
    pItemProperty   = c.pItemProperty;
    propertiesIndex = c.propertiesIndex;
    propertiesCount = c.propertiesCount;
    propertyMap     = c.propertyMap;

    createSafeTaskInstance();
    pSafeTask       = c.pSafeTask;
    recPattern      = c.recPattern;
}


/* 
 * Operator =
*/
ClientTask ClientTask::operator=(const ClientTask& c) {

    ClientTask cnew;

    cnew.hr              = c.hr;
    cnew.ID              = c.ID;
    cnew.itemType        = c.itemType;
    cnew.parentPath      = c.parentPath;
    cnew.pTask           = c.pTask;
    cnew.pItemProperties = c.pItemProperties;
    cnew.pItemProperty   = c.pItemProperty;
    cnew.propertiesIndex = c.propertiesIndex;
    cnew.propertiesCount = c.propertiesCount;
    cnew.propertyMap     = c.propertyMap;

    cnew.createSafeTaskInstance();
    cnew.pSafeTask       = c.pSafeTask;
    cnew.recPattern      = c.recPattern;

    return cnew;
}
