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
#include "outlook/defs.h"
#include "utils.h"

#include "outlook/ClientFolder.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"

using namespace std;



/**
 * Constructor:
 * initialize all members.
 */
ClientFolder::ClientFolder() {
    hr              = S_OK;
    ID              = EMPTY_WSTRING;
    itemType        = EMPTY_WSTRING;
    name            = EMPTY_WSTRING;
    path            = EMPTY_WSTRING;
    subfoldersIndex = 0;
    subfoldersCount = 0;
    itemsIndex      = 0;
    itemsCount      = 0;
    
    // should be created NULL...
    //pFolder         = NULL;
    //pSubFolders     = NULL;
    //pSubFolder      = NULL;
    //pItems          = NULL;
    //pItem           = NULL;
    //pContact        = NULL;
    //pAppointment    = NULL;
    //pMail           = NULL;
    //pTask           = NULL;
    //pNote           = NULL;

    subFolder       = NULL;
    mail            = NULL;
    contact         = NULL;
    appointment     = NULL;
    task            = NULL;
    note            = NULL;
}



/**
 * Destructor:
 * Delete internal objects
 */
ClientFolder::~ClientFolder() {

    if (subFolder) {
        delete subFolder;
        subFolder = NULL; 
    }
    if (mail) {
        delete mail;
        mail = NULL;
    }
    if (contact) {
        delete contact;
        contact = NULL;
    }
    if (appointment) {
        delete appointment;
        appointment = NULL;
    }
    if (task) {
        delete task;
        task = NULL;
    }
    if (note) {
        delete note;
        note = NULL;
    }

    // Release COM pointers.
    if (pFolder)      pFolder.Release     ();
    if (pSubFolders)  pSubFolders.Release ();
    if (pSubFolder)   pSubFolder.Release  ();
    if (pItems)       pItems.Release      ();
    if (pItem)        pItem.Release       ();
    if (pContact)     pContact.Release    ();
    if (pAppointment) pAppointment.Release();
    if (pMail)        pMail.Release       ();
    if (pTask)        pTask.Release       ();
    if (pNote)        pNote.Release       ();
}


/**
 * Set a COM pointer to this object.
 ************************************
 * This method is used to link the object to the correspondent
 * outlook COM pointer. All class members are overwrited by this call.
 * The method MUST be called before using this object, as the constructor
 * doesn't link the class COM pointer.
 * If 'type' parameter is an empty string, then item type is derived
 * from the defaultItemType of the folder.
 */
void ClientFolder::setCOMPtr(MAPIFolderPtr& f, const wstring& type) {
    
    pFolder = f;

    try {
        pSubFolders = pFolder->GetFolders();
        pItems      = pFolder->GetItems();
        ID  = (WCHAR*)pFolder->GetEntryID();

        _bstr_t bstrName = pFolder->GetName();
        // Folders name could be "" -> NULL is returned!
        if (bstrName.length()) name = (WCHAR*)bstrName;
        else                   name = L"";

        // Outlook returns "%5C" instead of "\" and "%2F" instead of "/"
        path = (WCHAR*)pFolder->GetFullFolderPath();
        // "%5C" is kept and used to escape the "\" char
        replaceAll(L"%2F", L"/",     path);
        
        if (type != EMPTY_WSTRING) {
            itemType = type;
        }
        else {
            OlItemType olType = pFolder->GetDefaultItemType();
            itemType = getItemTypeFromOlType(olType);
        }

        subfoldersCount = pSubFolders->GetCount();
        itemsCount      = pItems->GetCount();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_FOLDER_ASSIGN);
    }

    hr = S_OK;
    subfoldersIndex = 0;
    itemsIndex      = 0;
    pSubFolder      = NULL;
    pItem           = NULL;
    pContact        = NULL;
    pAppointment    = NULL;
    pMail           = NULL;
    pTask           = NULL;
    pNote           = NULL;
}

/// Here itemType is derived from the defaultItemType of the folder.
void ClientFolder::setCOMPtr(MAPIFolderPtr& f) {
    return setCOMPtr(f, EMPTY_WSTRING);
}

/**
 * Returns a reference to the internal COM pointer.
 */
MAPIFolderPtr& ClientFolder::getCOMPtr() {
    return pFolder;
}


//
// These members are stored inside the object, as they cannot change.
//
const wstring& ClientFolder::getID() {
    return ID;
}

const wstring& ClientFolder::getType() {
    return itemType;
}

const wstring& ClientFolder::getName() {
    return name;
}

const wstring& ClientFolder::getPath() {
    return path;
}



//
// These members are always retrieved from Outlook, as they 
// could change, due to save/delete item operations.
//
const int ClientFolder::getSubfoldersCount() {
   
    try {
        subfoldersCount = pSubFolders->GetCount();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_COUNT, name.c_str());
        throwClientException(getLastErrorMsg());
    }

    return subfoldersCount;
}


const int ClientFolder::getItemsCount() {
   try {
        itemsCount = pItems->GetCount();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEMS_COUNT, name.c_str());
        throwClientException(getLastErrorMsg());
    }

    return itemsCount;
}


//
// Internal iterators for subfolders/items.
//
const int ClientFolder::getSubfoldersIndex() {
    return subfoldersIndex;
}

const int ClientFolder::getItemsIndex(){
    return itemsIndex;
}






//
// -------------------------------- Methods to retrieve a folder object --------------------------------------------
//

/**
 * Returns the first subfolder of this folder.
 * If subfolder not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
 */
ClientFolder* ClientFolder::getFirstSubfolder() {

    if (!subfoldersCount) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, 0, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pSubFolder = pSubFolders->GetFirst();
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    subfoldersIndex = 0;

    // If first use, creates a new folder object for the unique internal subFolder
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, 0, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the next subfolder of this folder.
 * If subfolder not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientFolder* ClientFolder::getNextSubfolder() {

    if (!subfoldersCount || subfoldersIndex+1 >= subfoldersCount) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersIndex+1, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pSubFolder = pSubFolders->GetNext();
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    subfoldersIndex ++;

    // It should never happen...
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersIndex+1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the previous subfolder of this folder.
 * If subfolder not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientFolder* ClientFolder::getPreviousSubfolder() {

    if (!subfoldersCount || subfoldersIndex-1 < 0) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersIndex-1, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pSubFolder = pSubFolders->GetPrevious();
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    subfoldersIndex --;

    // It should never happen...
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersIndex-1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the last subfolder of this folder.
 * If subfolder not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientFolder* ClientFolder::getLastSubfolder() {

    if (!subfoldersCount) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersCount-1, name.c_str());
        return NULL;
    }

    // Get the COM pointer from Outlook.
    try {
        pSubFolder = pSubFolders->GetLast();
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    subfoldersIndex = subfoldersCount - 1;

    // Set the COM pointer to the internal folder (overwrite past values)
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, subfoldersCount-1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}


/**
 * Returns the subfolder from its index.
 * If subfolder not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
 * @note   'index + 1' is used, as first outlook folder has index = 1.
*/
ClientFolder* ClientFolder::getSubfolder(const int index) {

    if (!subfoldersCount || 
        index >= subfoldersCount || 
        index < 0) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, index, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pSubFolder = pSubFolders->Item(index+1);        // Index
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    subfoldersIndex = index;

    // If first use, creates a new internal folder object
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NOT_FOUND, index, name.c_str());
    //throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the subfolder from its name.
 * If subfolder not found returns NULL.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientFolder* ClientFolder::getSubfolderFromName(const wstring& subName) {

    if (!subfoldersCount || subName == EMPTY_WSTRING) {
        LOG.debug(ERR_OUTLOOK_SUBFOLDER_NAME, subName.c_str(), name.c_str());
        return NULL;
    }

    // Search subfolder with the specified name
    int index;
    for (index=0; index<subfoldersCount; index++) {
        
        // TBD: replace with direct access to COM Ptr.... (faster)
        subFolder = getSubfolder(index);
        if (!subFolder) {
            goto error;
        }

        // Convert strings to lower case: Outlook folders are case insensitive.
        wstring subName1 = subName;
        wstring name1 = subFolder->getName();
        toLowerCase(subName1);
        toLowerCase(name1);
        if (name1 == subName1) {
            return subFolder;
        }
    }
    
error:
    // if not found
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_NAME, subName.c_str(), name.c_str());
    //throwClientException(getLastErrorMsg());
    return NULL;
}





/**
 * Adds a new Subfolder for this folder, then a pointer is returned.
 * 'type' param is required because
 * the subfolder could not be of the same type of the parent folder.
 * In case of errors throws a ClientException.
 * @return  the ClientFolder* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
 */
ClientFolder* ClientFolder::addSubFolder(const wstring& subName, const wstring& type) {

    OlDefaultFolders folderType = getDefaultFolderType(type);

    // Get the COM pointer from Outlook for a new item.
    try {
        pSubFolder = pSubFolders->Add(subName.c_str(), folderType);
        if (!pSubFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    
    subfoldersCount ++;


    // If first use, creates a new internal folder object
    if (!subFolder) {
        subFolder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    subFolder->setCOMPtr(pSubFolder, type);

    return subFolder;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_SUBFOLDER_CREATE, subName.c_str(), type.c_str(), name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}




//
// ------------------------------- Methods to retrieve an item object ---------------------------------------------
//
/**
 * Returns the first item of this folder.
 * The pointer returned is a generic ClientItem, but it is casted
 * from a specific Item based on the folder item type (contact / task / ...)
 * If item not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientItem* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientItem* ClientFolder::getFirstItem() {
    
    if (!itemsCount) {
        LOG.debug(ERR_OUTLOOK_ITEM_NOT_FOUND, 0, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pItem = pItems->GetFirst();
        if (!pItem) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    itemsIndex = 0;

    // Here swich for item type and link the proper internal item (contact / task /...)
    ClientItem* item = setInternalItem(pItem);

    return item;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_NOT_FOUND, 0, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}


/**
 * Returns the next item of this folder.
 * The pointer returned is a generic ClientItem, but it is casted
 * from a specific Item based on the folder item type (contact / task / ...)
 * If item not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientItem* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientItem* ClientFolder::getNextItem() {

    if (!itemsCount || itemsIndex+1 >= itemsCount) {
        LOG.debug(ERR_OUTLOOK_ITEM_NOT_FOUND, itemsIndex+1, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pItem = pItems->GetNext(); 
        if (!pItem) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    itemsIndex ++;

    // Here swich for item type and link the proper internal item (contact / task /...)
    ClientItem* item = setInternalItem(pItem);

    return item;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_NOT_FOUND, itemsIndex+1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the previous item of this folder.
 * The pointer returned is a generic ClientItem, but it is casted
 * from a specific Item based on the folder item type (contact / task / ...)
 * If item not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientItem* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientItem* ClientFolder::getPreviousItem() {

    if (!itemsCount || itemsIndex-1 < 0) {
        LOG.debug(ERR_OUTLOOK_ITEM_NOT_FOUND, itemsIndex-1, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pItem = pItems->GetPrevious(); 
        if (!pItem) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    itemsIndex --;

    // Here swich for item type and link the proper internal item (contact / task /...)
    ClientItem* item = setInternalItem(pItem);

    return item;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_NOT_FOUND, itemsIndex-1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Returns the last item of this folder.
 * The pointer returned is a generic ClientItem, but it is casted
 * from a specific Item based on the folder item type (contact / task / ...)
 * If item not found returns NULL. In case of errors throws a ClientException.
 * @return  the ClientItem* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientItem* ClientFolder::getLastItem() {

    if (!itemsCount) {
        LOG.debug(ERR_OUTLOOK_ITEM_NOT_FOUND, itemsCount-1, name.c_str());
        return NULL;
    }
    
    // Get the COM pointer from Outlook.
    try {
        pItem = pItems->GetLast(); 
        if (!pItem) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    itemsIndex = itemsCount - 1;

    // Here swich for item type and link the proper internal item (contact / task /...)
    ClientItem* item = setInternalItem(pItem);

    return item;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_NOT_FOUND, itemsCount-1, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}



/**
 * Adds a new Item for this folder, then a pointer is returned.
 * The pointer returned is a generic ClientItem, but it is casted
 * from a specific Item based on the folder item type (contact / task / ...)
 * In case of errors throws a ClientException.
 * @return  the ClientItem* pointer returned is a reference to the internal object.
 *         (internal objects are deleted in the destructor)
*/
ClientItem* ClientFolder::addItem() {

    // Get the COM pointer from Outlook for a new item.
    try {
        pItem = pItems->Add();
        if (!pItem) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }
    
    itemsCount ++;

    // Here swich for item type and link the proper internal item (contact / task /...)
    ClientItem* item = setInternalItem(pItem);

    return item;

error:
    setErrorF(getLastErrorCode(), ERR_OUTLOOK_ITEM_CREATE, itemsCount, name.c_str());
    throwClientException(getLastErrorMsg());
    return NULL;
}




/**
 * Set the appropriate internal item, based on the item type.
 * If the internal item is NULL, it is created new. Then the item is
 * linked with the COM pointer passed.
 * @param pItem   the generic Item COM pointer to link the item object         <-- *** NOW class member / remove parameter...
 * @return        a ClientItem pointer to the internal object updated
 *                (NULL if is a bad item for the item-type)
 */
ClientItem* ClientFolder::setInternalItem(IDispatchPtr& pItem) {
 
    ClientItem* item;

    //
    // Switch itemType:
    // ----------------
    // - verify if COM pointer is NULL after casting -> it's a bad item -> return NULL
    // - if first use, creates a new internal object
    // - link the COM pointer to the internal object
    //
    if (itemType == APPOINTMENT) {                           // APPOINTMENT ITEM
        pAppointment = (_AppointmentItemPtr)pItem;
        if (!pAppointment) goto badItem;

        if (!appointment) {
            appointment = new ClientAppointment();
        }
        appointment->setCOMPtr(pAppointment);
        item = (ClientItem*)appointment;
    }

    else if (itemType == CONTACT) {                          // CONTACT ITEM
        pContact = (_ContactItemPtr)pItem;
        if (!pContact) goto badItem;

        if (!contact) {
            contact = new ClientContact();
        }
        contact->setCOMPtr(pContact);
        item = (ClientItem*)contact;
    }

    else if (itemType == TASK) {                            // TASK ITEM
        pTask = (_TaskItemPtr)pItem;
        if (!pTask) goto badItem;

        if (!task) {
            task = new ClientTask();
        }
        task->setCOMPtr(pTask);
        item = (ClientItem*)task;
    }

    else if (itemType == NOTE) {                            // NOTE ITEM
        pNote = (_NoteItemPtr)pItem;
        if (!pNote) goto badItem;

        if (!note) {
            note = new ClientNote();
        }
        note->setCOMPtr(pNote);
        item = (ClientItem*)note;
    }

    else if (itemType == MAIL) {                            // MAIL ITEM
        pMail = (_MailItemPtr)pItem;
        if (!pMail) goto badItem;
        
        if (!mail) {
            mail = new ClientMail();
        }
        mail->setCOMPtr(pMail);
        item = (ClientItem*)mail;
    }

    else {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_BAD_ITEMTYPE, itemType.c_str());
        throwClientException(getLastErrorMsg());
    }

    return item;

badItem:
    _DistListItemPtr isDList = (_DistListItemPtr)pItem;
    if (isDList && itemType == CONTACT) {
        // Can happen often (contacts distribution_list) -> no throw.
        LOG.debug(DBG_OUTLOOK_DLIST_ITEM, itemType.c_str(), name.c_str(), itemsIndex);
        isDList.Release();
        return NULL;
    }
    else if (itemType == NOTE) {
        _MailItemPtr isMail = (_MailItemPtr)pItem;
        if (isMail) {
            // Notes synced from WM with ActiveSync, can be MailItems if the
            // note is not a text note (drawing / sound ...)
            LOG.debug(DBG_OUTLOOK_BAD_NOTE_ITEM, name.c_str(), itemsIndex);
            isMail.Release();
        }
        else {
            LOG.error(ERR_OUTLOOK_BAD_ITEM, itemsIndex, name.c_str(), itemType.c_str());
            setErrorF(getLastErrorCode(), "%s", ERR_OUTLOOK_BAD_ITEM_MSG);
        }
        return NULL;
    }
    else {
        // Can happen if Outlook stuck / not reponding... no throw.
        LOG.error(ERR_OUTLOOK_BAD_ITEM, itemsIndex, name.c_str(), itemType.c_str());
        setErrorF(getLastErrorCode(), "%s", ERR_OUTLOOK_BAD_ITEM_MSG);
        return NULL;
    }
}





/**
 * Copy Constructor
 */
ClientFolder::ClientFolder(ClientFolder& f) {

    hr              = f.hr;
    ID              = f.ID;
    itemType        = f.itemType;
    name            = f.name;
    path            = f.path;
    subfoldersIndex = f.subfoldersIndex;
    subfoldersCount = f.subfoldersCount;
    itemsIndex      = f.itemsIndex;
    itemsCount      = f.itemsCount;
    pFolder         = f.pFolder;
    pSubFolders     = f.pSubFolders;
    pSubFolder      = f.pSubFolder;
    pItems          = f.pItems;
    pItem           = f.pItem;

    // Copy internal objects (if pointers not null)
    if (f.subFolder)    subFolder = new ClientFolder(*(f.subFolder));
    else                subFolder = NULL;

    if (f.mail)         mail = new ClientMail(*(f.mail));
    else                mail = NULL;

    if (f.contact)      contact = new ClientContact(*(f.contact));
    else                contact = NULL;

    if (f.appointment)  appointment = new ClientAppointment(*(f.appointment));
    else                appointment = NULL;

    if (f.task)         task = new ClientTask(*(f.task));
    else                task = NULL;

    if (f.note)         note = new ClientNote(*(f.note));
    else                note = NULL;
}


/**
 * Operator =
 */
ClientFolder ClientFolder::operator=(ClientFolder& f) {

    ClientFolder fnew;

    fnew.hr              = f.hr;
    fnew.ID              = f.ID;
    fnew.itemType        = f.itemType;
    fnew.name            = f.name;
    fnew.path            = f.path;
    fnew.subfoldersIndex = f.subfoldersIndex;
    fnew.subfoldersCount = f.subfoldersCount;
    fnew.itemsIndex      = f.itemsIndex;
    fnew.itemsCount      = f.itemsCount;
    fnew.pFolder         = f.pFolder;
    fnew.pSubFolders     = f.pSubFolders;
    fnew.pSubFolder      = f.pSubFolder;
    fnew.pItems          = f.pItems;
    fnew.pItem           = f.pItem;

    // Copy internal objects (if pointers not null)
    if (f.subFolder)    fnew.subFolder = new ClientFolder(*(f.subFolder));
    else                fnew.subFolder = NULL;

    if (f.mail)         fnew.mail = new ClientMail(*(f.mail));
    else                fnew.mail = NULL;

    if (f.contact)      fnew.contact = new ClientContact(*(f.contact));
    else                fnew.contact = NULL;

    if (f.appointment)  fnew.appointment = new ClientAppointment(*(f.appointment));
    else                fnew.appointment = NULL;

    if (f.task)         fnew.task = new ClientTask(*(f.task));
    else                fnew.task = NULL;

    if (f.note)         fnew.note = new ClientNote(*(f.note));
    else                fnew.note = NULL;

    return fnew;
}
