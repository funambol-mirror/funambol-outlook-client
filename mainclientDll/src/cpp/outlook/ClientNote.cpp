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
#include "outlook/ClientNote.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"

using namespace std;


/* 
 * Constructor:
 * Dummy initialize class members.
 * Instance the Redemption COM pointer.
*/
ClientNote::ClientNote() : ClientItem() {
    pNote = NULL;
}


// Destructor
ClientNote::~ClientNote() {
    if (pNote) { 
        pNote.Release();
    }
}



/*
 * Set a COM pointer to this object.
 * ------------------------------------
 * This method is used to link the object to the correspondent
 * outlook COM pointer. All class members are overwrited by this call.
 * The method MUST be called before using this object, as the constructor
 * doesn't link the class COM pointer.
 * If 'itemID' parameter is an empty string, then ID is derived
 * from the entryID of the outlook item.
 */
void ClientNote::setCOMPtr(_NoteItemPtr& ptr, const wstring& itemID) {

    pNote = ptr;

    try {
        pItemProperties = pNote->ItemProperties;
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
            _bstr_t bstrID = pNote->GetEntryID();
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
        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pNote->GetParent();
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
    itemType        = NOTE;
    propertiesIndex = 0;

    // Assume all items of this type have same props!
    //propertyMap.clear();
}


// Here ID is derived from the entryID of the item.
void ClientNote::setCOMPtr(_NoteItemPtr& ptr) {
    return setCOMPtr(ptr, EMPTY_WSTRING);
}


/**
 * Returns a reference to the internal COM pointer.
 */
_NoteItemPtr& ClientNote::getCOMPtr() {
    return pNote;
}




/*
 * Save the current Item.
 * @return: 0 if no errors.
 */
int ClientNote::saveItem() {
    
    try {
        hr = pNote->Save();
        if (FAILED(hr)) {
            goto error;
        }
        // Get the new entry-ID.
        ID = (WCHAR*)pNote->GetEntryID();
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
int ClientNote::deleteItem() {
    
    try {
        hr = pNote->Delete();
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
ClientItem* ClientNote::copyItem() {

    IDispatchPtr  pNew     = NULL;
    _NoteItemPtr  pNoteNew = NULL;
    ClientNote*   cNew     = NULL;

    if (!pNote) {
        goto error;
    }

    try {
        // Copy the COM pointer
        pNew = pNote->Copy();
        if (!pNew) {
            goto error;
        }
        pNoteNew = (_NoteItemPtr)pNew;
        if (!pNoteNew) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Set the new COM pointer to a new allocated ClientNote.
    cNew = new ClientNote();
    cNew->setCOMPtr(pNoteNew);

    return (ClientItem*)cNew;

error:
    return NULL;
}


/**
 * Moves this ClientNote into the passed destination folder.
 * The move operation changes only the item location in Outlook, so
 * the item's ID is preserved.
 *
 * @param   destFolder  the destination ClientFolder to move this object to
 * @return              0 if no errors
 */
//int ClientNote::moveItem(ClientFolder* destFolder) {
//
//    if (!pNote) {
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
//        _bstr_t a = pNote->GetEntryID();
//        hr = pNote->Move(pDestFolder);
//        _bstr_t b = pNote->GetEntryID();
//        
//        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pNote->GetParent();
//        parentPath = (WCHAR*)parentFolder->GetFullFolderPath();
//        LOG.debug("");
//    }
//    catch(_com_error &e) {
//        manageComErrors(e);
//        goto error;
//    }
//
//    //parentPath = destFolder->getPath();
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
 * Protected properties are listed in safeNoteProps array.
 */
bool ClientNote::isSecureProperty(const wstring& propertyName) {
    
    // search for property in safeContactProps array
    for (int i=0; safeNoteProps[i]; i++) {
        if (propertyName == safeNoteProps[i]) {
            return true;
        }
    }
    return false;
}


/*
 * Return true if the property needs some specific conversion of data.
 * Complex properties are listed in complexNoteProps array.
 */
bool ClientNote::isComplexProperty(const wstring& propertyName) {
    
    // search for property in complexNoteProps array
    for (int i=0; complexNoteProps[i]; i++) {
        if (propertyName == complexNoteProps[i]) {
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
 */
const wstring ClientNote::getSafeProperty(const wstring& propertyName) {
    
    wstring propertyValue = EMPTY_WSTRING;

    if (propertyName == L"Body") {
        // Use Redemption MAPIUtils object, from ClientApplication.
        // (there is no safeNote object)
        ClientApplication* ol = ClientApplication::getInstance();
        propertyValue = ol->getBodyFromID(ID);
    }

    return propertyValue;
}


/*
 * Return the Item property value from the property name.
 * Here are processed properties that need specific conversion of 
 * the property value.
 */
const wstring ClientNote::getComplexProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;

    // "Subject" property is retrieved as simple property.
    // (no conversion needed on this side)
    if (propertyName == L"Subject") {
        propertyValue = getSimpleProperty(propertyName);
    }

    else if (propertyName == L"LastModificationTime") {         // "(double format)"
        DATE date = pNote->GetLastModificationTime();
        WCHAR tmp[32];
        swprintf_s(tmp, L"%.12f", date);
        propertyValue = tmp;
    }

    // Separator for Categories in Outlook can be "," or ";".
    // We use only ",".
    else if (propertyName == L"Categories") {
        _bstr_t categories = pNote->GetCategories();
        if (categories.length() > 0) {
            propertyValue = (WCHAR*)categories;
            replaceAll(L";", L",", propertyValue);
        }
    }

    // "Color" is deprecated from Outlook2007 ("Categories" is used)
    // In that case we ignore it.
    else if (propertyName == L"Color") {
        ClientApplication* ol = ClientApplication::getInstance();
        int majorVersion = _wtoi(ol->getVersion().c_str());
        if (majorVersion < 12) {                                // '12.x.y' is Outlook 2007
            WCHAR tmp[4];
            OlNoteColor color = pNote->GetColor();
            swprintf_s(tmp, L"%d", color);
            propertyValue = tmp;
        }
    }

    else if ( (propertyName == L"Height") || (propertyName == L"Width") ||
              (propertyName == L"Left")   || (propertyName == L"Top") ) {
        propertyValue = getSimpleProperty(propertyName);
    }

    return propertyValue;
}





// ------------------------------- SET PROPERTY -------------------------------
/*
 * Set the Item property value for the specific property name.
 * Here are processed properties that need specific conversion of the property value. 
 *
 * @param propertyName  : the name of the property
 * @param propertyValue : the value to store
 * @return              : 0 if no errors, 1 if errors
 */
int ClientNote::setComplexProperty(const wstring& propertyName, const wstring& propertyValue) {

    // "Subject" has no put method (it's retrieved from 'Body') -> ignore it!
    if (propertyName == L"Subject") {
        return 0;
    }
    // Separator for Categories in Outlook can be "," or ";".
    // Nothing to do (both accepted by Outlook).
    else if (propertyName == L"Categories") {
        pNote->PutCategories(propertyValue.c_str());
    }

    // "Color" is deprecated from Outlook2007 ("Categories" is used)
    // In that case we ignore it.
    else if (propertyName == L"Color") {
        ClientApplication* ol = ClientApplication::getInstance();
        int majorVersion = _wtoi(ol->getVersion().c_str());
        if (majorVersion < 12) {                                // '12.x.y' is Outlook 2007
            int color = _wtoi(propertyValue.c_str());
            pNote->PutColor((OlNoteColor)color);
        }
    }

    // Outlook does not accept empty values
    else if ( (propertyName == L"Height") || (propertyName == L"Width") ||
              (propertyName == L"Left")   || (propertyName == L"Top") ) {
        if (!propertyValue.empty()) {
            pItemProperty = pItemProperties->Item(propertiesIndex);
            _bstr_t bstrValue = (_bstr_t)propertyValue.c_str();    
            pItemProperty->PutValue(bstrValue);
        }
    }

    return 0;
}





/* 
 * Copy Constructor
*/
ClientNote::ClientNote(const ClientNote& c) {
    hr              = c.hr;
    ID              = c.ID;
    itemType        = c.itemType;
    parentPath      = c.parentPath;
    pNote           = c.pNote;
    pItemProperties = c.pItemProperties;
    pItemProperty   = c.pItemProperty;
    propertiesIndex = c.propertiesIndex;
    propertiesCount = c.propertiesCount;
    propertyMap     = c.propertyMap;
}


/* 
 * Operator =
*/
ClientNote ClientNote::operator=(const ClientNote& c) {

    ClientNote cnew;

    cnew.hr              = c.hr;
    cnew.ID              = c.ID;
    cnew.itemType        = c.itemType;
    cnew.parentPath      = c.parentPath;
    cnew.pNote           = c.pNote;
    cnew.pItemProperties = c.pItemProperties;
    cnew.pItemProperty   = c.pItemProperty;
    cnew.propertiesIndex = c.propertiesIndex;
    cnew.propertiesCount = c.propertiesCount;
    cnew.propertyMap     = c.propertyMap;

    return cnew;
}

