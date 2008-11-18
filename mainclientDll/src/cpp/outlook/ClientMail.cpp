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
#include "outlook/ClientMail.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"


using namespace std;




// Constructor
ClientMail::ClientMail() : ClientItem() {

    pMail           = NULL;
    pSafeMail       = NULL;

    createSafeMailInstance();
}

// Destructor
ClientMail::~ClientMail() {
    if (pMail)     { pMail.Release();     }
    if (pSafeMail) { pSafeMail.Release(); }
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
void ClientMail::setCOMPtr(_MailItemPtr& ptr, const wstring& itemID) {

    pMail = ptr;

    try {
        pSafeMail->Item = pMail;

        pItemProperties = pMail->ItemProperties;
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
            _bstr_t bstrID = pMail->GetEntryID();
            // ID could not exist (if item not saved)
            if (bstrID.GetBSTR()) {
                ID = (WCHAR*)bstrID;
            }
            else {
                ID = EMPTY_WSTRING;
            }
        }

        // Full path of parent folder
        // Outlook returns "%5C" instead of "\" and "%2F" instead of "/"
        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pMail->GetParent();
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
    itemType        = MAIL;
    propertiesIndex = 0;

    // Assume all items of this type have same props!
    //propertyMap.clear();
}


// Here ID is derived from the entryID of the item.
void ClientMail::setCOMPtr(_MailItemPtr& ptr) {
    return setCOMPtr(ptr, EMPTY_WSTRING);
}


/**
 * Returns a reference to the internal COM pointer.
 */
_MailItemPtr& ClientMail::getCOMPtr() {
    return pMail;
}



/*
 * Creates an instance for the Redemption COM pointer.
 */
void ClientMail::createSafeMailInstance() {

    try {
        hr = pSafeMail.CreateInstance(RED_SAFEMAIL);
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
 * Save the current Item.
 * @return: 0 if no errors.
 */
int ClientMail::saveItem() {
    
    try {
        hr = pMail->Save();
        if (FAILED(hr))
            goto error;

        // Get the new entry-ID.
        ID = (WCHAR*)pMail->GetEntryID();
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
int ClientMail::deleteItem() {
    
    try {
        hr = pMail->Delete();
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
ClientItem* ClientMail::copyItem() {

    IDispatchPtr  pNew    = NULL;
    _MailItemPtr  pMaiNew = NULL;
    ClientMail*   cNew    = NULL;

    if (!pMail) {
        goto error;
    }

    try {
        // Copy the COM pointer
        pNew = pMail->Copy();
        if (!pNew) {
            goto error;
        }
        pMaiNew = (_MailItemPtr)pNew;
        if (!pMaiNew) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Set the new COM pointer to a new allocated ClientMail.
    cNew = new ClientMail();
    cNew->setCOMPtr(pMaiNew);

    return (ClientItem*)cNew;

error:
    return NULL;
}



/**
 * Moves this ClientMail into the passed destination folder.
 * The move operation changes only the item location in Outlook, so
 * the item's ID is preserved.
 *
 * @param   destFolder  the destination ClientFolder to move this object to
 * @return              0 if no errors
 */
//int ClientMail::moveItem(ClientFolder* destFolder) {
//
//    if (!pMail) {
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
//        pMail->Move(pDestFolder);
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


bool ClientMail::isSecureProperty(const wstring& propertyName) {
    return false;
}

bool ClientMail::isComplexProperty(const wstring& propertyName) {
    return false;
}

const wstring ClientMail::getSafeProperty(const wstring& propertyName) {
    return NULL;
}

const wstring ClientMail::getComplexProperty(const wstring& propertyName) {
    return NULL;
}




//const wstring ClientMail::getFirstProperty() {
//    return NULL;
//}
//const wstring ClientMail::getNextProperty() {
//    return NULL;
//}
//const wstring ClientMail::getPreviousProperty() {
//    return NULL;
//}
//const wstring ClientMail::getLastProperty() {
//    return NULL;
//}


//int ClientMail::setProperty(const wstring& propertyName, const wstring& propertyValue) {
//    return NULL;
//}

int ClientMail::setComplexProperty(const wstring& propertyName, const wstring& propertyValue) {
    return 0;
}


/* 
 * Copy Constructor
*/
ClientMail::ClientMail(const ClientMail& c) {
    hr              = c.hr;
    ID              = c.ID;
    itemType        = c.itemType;
    parentPath      = c.parentPath;
    pMail           = c.pMail;
    pItemProperties = c.pItemProperties;
    pItemProperty   = c.pItemProperty;
    propertiesIndex = c.propertiesIndex;
    propertiesCount = c.propertiesCount;
    propertyMap     = c.propertyMap;

    createSafeMailInstance();
    pSafeMail       = c.pSafeMail;
}


/* 
 * Operator =
*/
ClientMail ClientMail::operator=(const ClientMail& c) {

    ClientMail cnew;

    cnew.hr              = c.hr;
    cnew.ID              = c.ID;
    cnew.itemType        = c.itemType;
    cnew.parentPath      = c.parentPath;
    cnew.pMail           = c.pMail;
    cnew.pItemProperties = c.pItemProperties;
    cnew.pItemProperty   = c.pItemProperty;
    cnew.propertiesIndex = c.propertiesIndex;
    cnew.propertiesCount = c.propertiesCount;
    cnew.propertyMap     = c.propertyMap;

    cnew.createSafeMailInstance();
    cnew.pSafeMail       = c.pSafeMail;

    return cnew;
}

