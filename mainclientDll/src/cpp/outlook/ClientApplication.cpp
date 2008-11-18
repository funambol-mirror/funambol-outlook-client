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
#include "outlook/ClientAppointment.h"
#include "outlook/ClientContact.h"
#include "outlook/ClientMail.h"
#include "outlook/ClientNote.h"
#include "outlook/ClientTask.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"

using namespace std;


// Init static pointer.
ClientApplication* ClientApplication::pinstance = NULL;



// Class Methods:
//------------------------------------------------------------------------------------------

/**
 * Method to create the sole instance of ClientApplication
 */
ClientApplication* ClientApplication::getInstance() {

    if (pinstance == NULL) {
        pinstance = new ClientApplication;
    }
    return pinstance;
}

/**
 * Returns true if static instance is not NULL.
 */
bool ClientApplication::isInstantiated() {
    return (pinstance ? true : false);
}


/**
 * Constructor.
 * Creates a new instance of Outlook application then logs in.
 * Initializes version & programName.
 */
ClientApplication::ClientApplication() {
    hr = S_OK;

    try {
        // Init COM library for current thread. 
        LOG.debug("Initialize COM library");
        hr = CoInitialize(NULL);
        //hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            throwClientFatalException(ERR_COM_INITIALIZE);
            return; 
        }
        if (hr == S_FALSE) {
            LOG.debug("Warning: COM library already opened for this thread.");
        }

        // Instantiate Outlook
        LOG.debug("Create %ls instance...", OL_APPLICATION);
        hr = pApp.CreateInstance(OL_APPLICATION);
        if (FAILED(hr)) {
            throwClientFatalException(ERR_OUTLOOK_OPEN);
            return;
        }

        // "MAPI" = the only available message store.
        pMAPI = pApp->GetNamespace(MAPI);		

        // To Logon Outlook (if Outlook closed, it will be opened in bkground)
        LOG.debug("Logon to Outlook MAPI: default profile, show-dialog = %s, new-session = %s", (OL_SHOW_DIALOG)? "true":"false", (OL_NEW_SESSION)? "true":"false");
        pMAPI->Logon(OL_PROFILE, OL_PASSWORD, OL_SHOW_DIALOG, OL_NEW_SESSION);
        version = (WCHAR*)pApp->GetVersion();

        // IMAPIUtils should be instantiated, to be able to call 'cleanUp()' from the destructor.
        // Outlook 2002 might have a problem properly closing if there is an outstanding reference. 
        // Calling cleanUp method ensures that Redemption cleans up its internal references to 
        // all Extended MAPI objects.
        createSafeInstances();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        // Fatal exception, so we will exit the thread.
        throwClientFatalException(ERR_OUTLOOK_OPEN);
        return;
    }
    // *** To catch unexpected exceptions... ***
    catch(...) {
        throwClientFatalException(ERR_OUTLOOK_OPEN);
        return;
    }

    programName = getNameFromVersion(version);

    pFolder     = NULL;
    folder      = NULL;
    mail        = NULL;
    contact     = NULL;
    appointment = NULL;
    task        = NULL;
    note        = NULL;

    LOG.info(INFO_OUTLOOK_OPENED, programName.c_str());
}


/**
 * Destructor.
 * Log off and clean up shared objects,
 * delete internal objects.
 */
ClientApplication::~ClientApplication() {

    // Internal objects:
    if (folder) {
        delete folder;
        folder = NULL; 
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

    pinstance = NULL;



    // Clean up Redemption objects.
    hr = cleanUp();
    if (FAILED(hr)) {
        throwClientException(ERR_OUTLOOK_CLEANUP);
    }

    // Logoff (MUST be the same thread that logged in!)
    try {
        hr = pMAPI->Logoff();
        if (FAILED(hr)) {
            throwClientException(ERR_OUTLOOK_LOGOFF);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_LOGOFF);
    }

    try {
        // Release COM pointers.
        if (rdoSession) rdoSession.Release();
        if (pFolder)    pFolder.Release   ();
        if (pMAPI)      pMAPI.Release     ();
        if (pRedUtils){
            // ***** TODO: investigate on IMAPIUtils 
            if (getNameFromVersion(version) == OUTLOOK_2003) {
                // Outlook2003 can have issues if releasing this library: sometimes it's impossible
                // to create a new instance of Outlook.application in a new thread... need more investigation.
                LOG.debug("Detaching IMAPIUtils object...");
                pRedUtils.Detach();
            }
            else {
                // On other systems with olk2002 and olk2007, IMAPIUtils must be correctly released at this point
                LOG.debug("Releasing IMAPIUtils object...");
                pRedUtils.Release();
            }
        }
        if (pApp) pApp.Release();
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_RELEASE_COMOBJECTS);
    }
    catch(...) {
        throwClientException(ERR_OUTLOOK_RELEASE_COMOBJECTS);
    }

    LOG.info(INFO_OUTLOOK_CLOSED);
}




const wstring& ClientApplication::getVersion() {
    return version;
}

const wstring& ClientApplication::getName() {
    return programName;
}



/**
 * Creates instances for Redemption COM pointers:
 * - MAPIUtils  (used for notes body)
 * - RDOSession (used for EX->SMTP addresses)
 */
void ClientApplication::createSafeInstances() {

    //
    // Open and link Redemption MAPIUtils pointer: MUST be allocated 
    // only once here to avoid malfunctions of MAPIUtils.
    //
    LOG.debug("Creating Redemption.MAPIUtils instance...");
    try {
        pRedUtils.CreateInstance(L"Redemption.MAPIUtils");
        pRedUtils->MAPIOBJECT = pMAPI->Session->MAPIOBJECT;
        if (!pRedUtils) {
            throwClientFatalException(ERR_OUTLOOK_MAPIUTILS);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientFatalException(ERR_OUTLOOK_MAPIUTILS);
    }

    //
    // Open Redemption RDO Session and link to MAPI Object.
    //
    LOG.debug("Creating Redemption.RDOSession instance...");
    try {
        rdoSession.CreateInstance(L"Redemption.RDOSession");
        rdoSession->MAPIOBJECT = pMAPI->Session->MAPIOBJECT;
        if (!rdoSession) {
            throwClientFatalException(ERR_OUTLOOK_RDOSESSION);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientFatalException(ERR_OUTLOOK_RDOSESSION);
    }
}






//
// -------------------------- Methods to retrieve a folder object -----------------------------
//

/**
 * Returns the default ClientFolder for the specific item type.
 * @note
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is fred in the destructor)
*/
ClientFolder* ClientApplication::getDefaultFolder(const wstring& itemType) {

    OlDefaultFolders folderType;
    folderType = getDefaultFolderType(itemType);

    // Get the COM pointer from Outlook.
    try {
        pFolder = pMAPI->GetDefaultFolder(folderType);
        if (!pFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // If first use, creates a new folder object for the unique internal folder
    if (!folder) {
        folder = new ClientFolder();
    }

    // Set the COM pointer to the internal folder (overwrite past values)
    folder->setCOMPtr(pFolder, itemType);

    return folder;

error:
    sprintf(lastErrorMsg, ERR_OUTLOOK_DEFFOLDER_NOT_FOUND, itemType.c_str());
    throwClientFatalException(lastErrorMsg);
    return NULL;
}


/**
 * Returns the ClientFolder from its entryID.
 * Note:
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is fred in the destructor)
*/
ClientFolder* ClientApplication::getFolderFromID(const wstring& folderID) {

    // Get the COM pointer from Outlook.
    try {
        pFolder = pMAPI->GetFolderFromID(folderID.c_str());
        if (!pFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // If first use, creates a new folder object for the unique internal folder
    if (!folder) {
        folder = new ClientFolder();
    }

    // Set the COM pointer to the internal folder (overwrite past values)
    folder->setCOMPtr(pFolder);

    return folder;

error:
    sprintf(lastErrorMsg, ERR_OUTLOOK_IDFOLDER_NOT_FOUND, folderID.c_str());
    throwClientFatalException(lastErrorMsg);
    return NULL;
}



/**
 * Returns the ClientFolder manually selected by the user.
 * If 'itemType' is not empty string, verifies if folder selected is 
 * correct for the item type passed.
 * Note:
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is fred in the destructor)
*/
ClientFolder* ClientApplication::pickFolder(const wstring& itemType) {

    bool correctFolderSelected = false;
    char msg[512];
    OlItemType olType;

    try {
        // Cycle until correct folder selected
        while (!correctFolderSelected) {

            pFolder = pMAPI->PickFolder();
            if (!pFolder) {
                goto error;
            }

            if (itemType != EMPTY_WSTRING) {
                olType = getOlItemType(itemType);
                if (pFolder->GetDefaultItemType() != olType) {
                    // retry...
                    sprintf(msg, ERR_OUTLOOK_BAD_FOLDER_TYPE, itemType.c_str());
                    safeMessageBox(msg);
                    continue;
                }
                else  correctFolderSelected = true;
            }
            else  correctFolderSelected = true;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // If first use, creates a new folder object for the unique internal folder
    if (!folder) {
        folder = new ClientFolder();
    }

    // Set the COM pointer to the internal folder (overwrite past values)
    folder->setCOMPtr(pFolder, itemType);

    return folder;

error:
    // not necessary here...
    //throwClientException(INFO_OUTLOOK_FOLDER_NOT_SELECTED);
    LOG.debug(DBG_OUTLOOK_FOLDER_NOT_SELECTED);
    return NULL;
}



/**
 * Returns the ClientFolder manually selected by the user.
 * No item type verification is performed.
 * Note:
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is fred in the destructor)
*/
ClientFolder* ClientApplication::pickFolder() {
    return pickFolder(EMPTY_WSTRING);
}





/**
 * Returns the ClientFolder from the full folder path (eg "\\Personal Folders\Contacts").
 * If empty (or "\\" or "/") path passed, the default folder will be returned.
 * If correspondent folder does not exist, it will be created.
 * @note  the pointer returned is a reference to a internal object
 *        (internal objects are fred in the destructor)
*/
ClientFolder* ClientApplication::getFolderFromPath(const wstring& itemType, const wstring& path) {

    folder = getDefaultFolder(itemType);

    //
    // For compatibility: if nothing passed we use the default folder
    //
    if (path == L"/" || path == L"\\" || path == L"") {
        return folder;
    }


    // Replace "\\" with "%5C" which is not a valid sequence (skip 1st char).
    // This is done because "\" is the separator used to select the folder, so it's not good.
    wstring path1 = path;
    replaceAll(L"\\\\", L"%5C", path1, 1);


    // 
    // Search for specific subfolder.
    // ==============================
    //
    wstring name, subName;
    ClientFolder *f, *sf;
    f = folder;

    // parse the path to get subfolder names
    wstring::size_type start, end;
    const wstring delim = L"\\";

    //
    // First token: select root folder (e.g. "Personal Folders")
    // ------------
    start = path1.find_first_not_of(delim);
    if (start != wstring::npos) {
        // end of first name found
        end = path1.find_first_of(delim, start);
        if (end == wstring::npos) {
            end = path1.length();
        }
        name = path1.substr(start, end-start);

        f = getRootFolderFromName(name);
        
        // If folder doesn't exists -> try get the default personal folder...
        // If neither default root  -> error
        if (!f) {
            LOG.info("%s - Continue with default root folder.", lastErrorMsg);
            f = getDefaultRootFolder();
            if (!f) {
                sprintf(lastErrorMsg, ERR_OUTLOOK_NO_ROOTFOLDER);
                throwClientException(lastErrorMsg);
                return NULL;
            }
        }

        // begin of next token
        start = path1.find_first_not_of(delim, end);


        //
        // Next tokens: select folder as subfolder
        // ------------
        while (start != wstring::npos) {
            // end of a name found
            end = path1.find_first_of(delim, start);
            if (end == wstring::npos) {
                end = path1.length();
            }
            subName = path1.substr(start, end-start);
            replaceAll(L"%5C", L"\\", subName);             // Convert back "%5C" to "\".

            sf = f->getSubfolderFromName(subName);

            // If subfolder doesn't exists -> create the new folder
            // ----------------------------------------------------
            if (!sf) {
                sf = f->addSubFolder(subName, itemType);
            }

            // begin of next token
            start = path1.find_first_not_of(delim, end);

            // point recursively to subfolder
            f = sf;
        }
    }


    // Safety check: item type MUST correspond (Outlook doesn't care about it)
    if (f->getType() != itemType) {
        // non-blocking error
        sprintf(lastErrorMsg, ERR_OUTLOOK_PATH_TYPE_MISMATCH, f->getPath().c_str(), itemType.c_str());
        LOG.error(lastErrorMsg);
        return NULL;
    }

    return f;
}



/**
 * Returns the default root folder (index = 0). Root folders are the Outlook data
 * files folders (this should be "Personal Folder").
 */
ClientFolder* ClientApplication::getDefaultRootFolder() {
    return getRootFolder(0);
}



/**
 * Returns the root folder from its index. Root folders are the Outlook data
 * files folders (e.g. "Personal Folder"). If folder not found returns NULL.
 * Note:
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is freed in the destructor)
 * 'index + 1' is used, as first outlook folder has index = 1.
 */
ClientFolder* ClientApplication::getRootFolder(const int index) {

    try {
        // Get number of root folders (usually = 1)
        long rootFoldersCount = pMAPI->GetFolders()->GetCount();
        if (!rootFoldersCount || 
            index >= rootFoldersCount || 
            index < 0) {
            goto error;
        }
    
        // Get the COM pointer from Outlook.
        pFolder = pMAPI->GetFolders()->Item(index+1);        // Index
        if (!pFolder) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // If first use, creates a new internal folder object
    if (!folder) {
        folder = new ClientFolder();
    }
    // Set the COM pointer to the internal folder (overwrite past values)
    folder->setCOMPtr(pFolder);

    return folder;

error:
    // non-blocking error
    sprintf(lastErrorMsg, ERR_OUTLOOK_ROOTFOLDER_NOT_FOUND, index);
    LOG.info(lastErrorMsg);
    return NULL;
}



/**
 * Returns the root folder from its name. Root folders are the Outlook data
 * files folders (e.g. "Personal Folder"). If folder not found returns NULL.
 * Note:
 * the pointer returned is a reference to the internal ClientFolder.
 * (the internal object is fred in the destructor)
 */
ClientFolder* ClientApplication::getRootFolderFromName(const wstring& folderName) {

    long rootFoldersCount;

    try {
        // Get number of root folders (usually = 1)
        rootFoldersCount = pMAPI->GetFolders()->GetCount();
        if (!rootFoldersCount || folderName == EMPTY_WSTRING) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Search root folder with the specified name
    for (int index=0; index < rootFoldersCount; index++) {
        
        // TBD: replace with direct access to COM Ptr.... (much faster)
        folder = getRootFolder(index);

        if (!folder) {
            goto error;
        }
        if (folder->getName() == folderName) {
            return folder;
        }
    }

// Not found
error:
    // non-blocking error
    sprintf(lastErrorMsg, ERR_OUTLOOK_ROOTFOLDER_NAME, folderName.c_str());
    LOG.debug(lastErrorMsg);
    return NULL;
}




//
// -------------------------- Methods to retrieve an item object -----------------------------
//

/**
 * Returns the ClientItem from its entryID.
 * The object returned is the specific ClientItem based on 'itemType'
 * (i.e. if itemType is CONTACT, will return an ClientContact object)
 * Returns NULL if the itemID corresponds to a bad item for the item-type.
 *
 * Note:
 * the pointer returned is a reference to the internal ClientItem.
 * (the internal object is fred in the destructor)
*/
ClientItem* ClientApplication::getItemFromID(const wstring& itemID, const wstring& itemType) {

    _bstr_t id = itemID.c_str();
    ClientItem* item;

    try {
        if (itemType == APPOINTMENT) {                                  // APPOINTMENT ITEM
            _AppointmentItemPtr pAppointment = pMAPI->GetItemFromID(id);
            if (!pAppointment) return NULL;
            
            // If first use, create a new internal object
            if (!appointment) {
                appointment = new ClientAppointment();
            }
            // Set the COM pointer to the internal object
            appointment->setCOMPtr(pAppointment, itemID);
            item = (ClientItem*)appointment;
        }

        else if (itemType == CONTACT) {                                  // CONTACT ITEM
            _ContactItemPtr pContact = pMAPI->GetItemFromID(id);
            if (!pContact) return NULL;
            
            // If first use, create a new internal object
            if (!contact) {
                contact = new ClientContact();
            }
            // Set the COM pointer to the internal object
            contact->setCOMPtr(pContact, itemID);
            item = (ClientItem*)contact;
        }

        else if (itemType == TASK) {                                     // TASK ITEM
            _TaskItemPtr pTask = pMAPI->GetItemFromID(id);
            if (!pTask) return NULL;

            // If first use, create a new internal object
            if (!task) {
                task = new ClientTask();
            }
            // Set the COM pointer to the internal object
            task->setCOMPtr(pTask, itemID);
            item = (ClientItem*)task;
        }

        else if(itemType == NOTE) {                                     // NOTE ITEM
            _NoteItemPtr pNote = pMAPI->GetItemFromID(id);
            if (!pNote) return NULL;

            // If first use, create a new internal object
            if (!note) {
                note = new ClientNote();
            }
            // Set the COM pointer to the internal object
            note->setCOMPtr(pNote, itemID);
            item = (ClientItem*)note;
        }

        else if (itemType == MAIL) {                                    // MAIL ITEM
            _MailItemPtr pMail = pMAPI->GetItemFromID(id);
            if (!pMail) return NULL;
            
            // If first use, create a new internal object
            if (!mail) {
                mail = new ClientMail();
            }
            // Set the COM pointer to the internal object
            mail->setCOMPtr(pMail, itemID);
            item = (ClientItem*)mail;
        }

        else {
            sprintf(lastErrorMsg, ERR_OUTLOOK_BAD_ITEMTYPE, itemType.c_str());
            throwClientException(lastErrorMsg);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        sprintf(lastErrorMsg, ERR_OUTLOOK_IDITEM_NOT_FOUND, itemID.c_str());
        throwClientException(lastErrorMsg);
    }

    return item;
}








//
// ---------------------------- Utility Methods ------------------------------
//

/**
 * To release shared session of Outlook. 
 * This function avoids Outlook being instable after usage of Redemption.
 * Release: 
 * - RDOSession (used for EX->SMTP addresses)
 * - MAPIUtils  (used for notes body)
 */
HRESULT ClientApplication::cleanUp() {
    HRESULT ret = S_OK;

    try {
        if (pRedUtils) {
            ret = pRedUtils->Cleanup();
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_CLEANUP);
    }
    return ret;
}



/**
 * Utility to convert an Exchange mail address into a SMTP address.
 * @note this method uses the redemption library (RDOSession object).
 *       It is placed here because the RDOSession object needs to be linked to Outlook MAPI.
 * 
 * @param EXAddress : the EX address to be converted
 * @return          : the SMTP address if found (else empty string)
 */
wstring ClientApplication::getSMTPfromEX(const wstring& EXAddress) {
    
    Redemption::IRDOAddressListPtr   rdoAddrList;
    Redemption::IRDOAddressEntryPtr  rdoAddrEntry;
    wstring SMTPAddr = EMPTY_WSTRING;

    // RDOSession initialized if it's the first time.
    if (!rdoSession) {
        createSafeInstances();
    }
    if (!rdoSession) {
        throwClientException(ERR_OUTLOOK_RDOSESSION);
        return EMPTY_WSTRING;
    }

    // Find correspondent entry
    try {
        // Get the Global Address list.
        rdoAddrList = rdoSession->GetAddressBook()->GetGAL();
        if (rdoAddrList) {
            rdoAddrEntry = rdoAddrList->ResolveName(EXAddress.c_str());
            _bstr_t tmp = rdoAddrEntry->GetSMTPAddress();
            if (tmp.length() > 0) {
                SMTPAddr = tmp;
           } 
        }
    }
    catch (_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_RDOSESSION_ADDRESS);
        return EMPTY_WSTRING;
    }

    return SMTPAddr;
}


/**
 * Utility to get body of a specified item (used for notes body which is protected).
 * @note: this method uses the redemption library (MAPIUtils object).
 *        It is placed here because the MAPIUtils object needs to be linked to Outlook MAPI.
 * 
 * @param itemID : the ID of item to search
 * @return       : the value of 'body' property
 */
wstring ClientApplication::getBodyFromID(const wstring& itemID) {

    Redemption::IMessageItemPtr  pMessage;
    wstring body = EMPTY_WSTRING;
    _bstr_t bstrID = (_bstr_t)itemID.c_str();
    _variant_t var;

    // MAPIUtils initialized if it's the first time.
    if (!pRedUtils) {
        createSafeInstances();
    }
    if (!pRedUtils) {
        throwClientException(ERR_OUTLOOK_MAPIUTILS);
        return EMPTY_WSTRING;
    }
    
    // Retrieve the safe body from Redemption message.
    try {
        pMessage = pRedUtils->GetItemFromID(bstrID, var);
        _bstr_t bstrBody = pMessage->GetBody();
        if (bstrBody.length() > 0) {
            body = (WCHAR*)bstrBody;
        }
    }
    catch (_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_MAPIUTILS_BODY);
        return EMPTY_WSTRING;
    }

    return body;
}


/**
 * Utility to retrieve the userName of current Outlook profile used.
 * In case of errors, or not yet logged on Outlook, throws a ClientException.
 * @note this method uses the redemption library (RDOSession object).
 */
wstring ClientApplication::getCurrentProfileName() {

    wstring name = EMPTY_WSTRING;

    // RDOSession initialized if it's the first time.
    if (!rdoSession) {
        createSafeInstances();
    }
    // RDOSession should be opened and initialized once in the constructor.
    if (!rdoSession) {
        throwClientException(ERR_OUTLOOK_RDOSESSION);
        return EMPTY_WSTRING;
    }

    try {
        if (!rdoSession->GetLoggedOn()) {
            throwClientException(ERR_OUTLOOK_NOT_LOGGED);
            return EMPTY_WSTRING;
        }
        name = (WCHAR*)rdoSession->GetProfileName();
    }
    catch (_com_error &e) {
        manageComErrors(e);
        throwClientException(ERR_OUTLOOK_GET_PROFILENAME);
        return EMPTY_WSTRING;
    }

    return name;
}


/**
 * Returns true if Outlook MAPI object is logged on.
 */
const bool ClientApplication::isLoggedOn() {

    if (!pApp) return false;

    try {
        pApp->GetSession();
    }
    catch(_com_error &e) {
        LOG.debug(DBG_OUTLOOK_NOT_LOGGED, e.ErrorMessage(), e.Error());
        return false;
    }

    return true;
}

