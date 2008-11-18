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
#include "outlook/ClientContact.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"


using namespace std;

wstring ClientContact::tmpPicturePath = L"";


/* 
 * Constructor:
 * Dummy initialize class members.
 * Instance the Redemption COM pointer.
*/
ClientContact::ClientContact() : ClientItem() {

    pContact        = NULL;
    pSafeContact    = NULL;
    willCreateAnniversaryEvent = false;
    willCreateBirthdayEvent    = false;

    createSafeContactInstance();

    // Init 'tmpPicturePath' static member.
    if (tmpPicturePath == L"") {
        WCHAR* path = readAppDataPath();
        if (path) {
            tmpPicturePath = path;
            tmpPicturePath += L"\\";
            tmpPicturePath += PICTURE_TMP_NAME;
            delete [] path;
        }
    }
}


// Destructor
ClientContact::~ClientContact() {
    if (pContact)     { pContact.Release();     }
    if (pSafeContact) { pSafeContact.Release(); }
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
void ClientContact::setCOMPtr(_ContactItemPtr& ptr, const wstring& itemID) {

    pContact = ptr;

    try {
        pSafeContact->Item = pContact;

        pItemProperties = pContact->ItemProperties;
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
            _bstr_t bstrID = pContact->GetEntryID();
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
        MAPIFolderPtr parentFolder = (MAPIFolderPtr)pContact->GetParent();
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
    itemType        = CONTACT;
    propertiesIndex = 0;
    willCreateAnniversaryEvent = false;
    willCreateBirthdayEvent    = false;

    // Assume all items of this type have same props!
    //propertyMap.clear();
}


// Entry-ID is derived from the entryID of the item.
void ClientContact::setCOMPtr(_ContactItemPtr& ptr) {
    return setCOMPtr(ptr, EMPTY_WSTRING);
}


/**
 * Returns a reference to the internal COM pointer.
 */
_ContactItemPtr& ClientContact::getCOMPtr() {
    return pContact;
}



/*
 * Creates an instance for the Redemption COM pointer.
 */
void ClientContact::createSafeContactInstance() {

    try {
        hr = pSafeContact.CreateInstance(RED_SAFECONTACT);
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
int ClientContact::saveItem() {  
    
    try {
        hr = pContact->Save();
        if (FAILED(hr)) {
            goto error;
        }
        // Get the new entry-ID.
        ID = (WCHAR*)pContact->GetEntryID();
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
int ClientContact::deleteItem() {
    
    try {
        hr = pContact->Delete();
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
ClientItem* ClientContact::copyItem() {

    IDispatchPtr     pNew    = NULL;
    _ContactItemPtr  pConNew = NULL;
    ClientContact*   cNew    = NULL;

    if (!pContact) {
        goto error;
    }

    try {
        // Copy the COM pointer
        pNew = pContact->Copy();
        if (!pNew) {
            goto error;
        }
        pConNew = (_ContactItemPtr)pNew;
        if (!pConNew) {
            goto error;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        goto error;
    }

    // Set the new COM pointer to a new allocated ClientContact.
    cNew = new ClientContact();
    cNew->setCOMPtr(pConNew);

    return (ClientItem*)cNew;

error:
    return NULL;
}



/**
 * Moves this ClientContact into the passed destination folder.
 * The move operation changes only the item location in Outlook, so
 * the item's ID is preserved.
 *
 * @param   destFolder  the destination ClientFolder to move this object to
 * @return              0 if no errors
 */
//int ClientContact::moveItem(ClientFolder* destFolder) {
//
//    if (!pContact) {
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
//        pContact->Move(pDestFolder);
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
//    LOG.error("Error moving item '%ls'", getSafeItemName(this));
//    return 1;
//}



//
// ------------------------------ Methods to manage item properties -----------------------------
//

/*
 * Return true if the property is protected by Outlook Security patch.
 * Protected properties are listed in safeContactProps array.
 */
bool ClientContact::isSecureProperty(const wstring& propertyName) {

    // search for property in safeContactProps array
    for (int i=0; safeContactProps[i]; i++) {
        if (propertyName == safeContactProps[i]) {
            return true;
        }
    }
    return false;
}


/*
 * Return true if the property needs some specific conversion of data.
 * Complex properties are listed in complexContactProps array.
 */
bool ClientContact::isComplexProperty(const wstring& propertyName) {
    
    // search for property in complexContactProps array
    for (int i=0; complexContactProps[i]; i++) {
        if (propertyName == complexContactProps[i]) {
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
const wstring ClientContact::getSafeProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    BSTR tmpVal;
    
    if (propertyName == L"Body") {
        hr = pSafeContact->get_Body(&tmpVal);
    }

    else if (propertyName == L"Email1Address") {
        hr = pSafeContact->get_Email1Address(&tmpVal);
    }
    else if (propertyName == L"Email2Address") {
        hr = pSafeContact->get_Email2Address(&tmpVal);
    }
    else if (propertyName == L"Email3Address") {
        hr = pSafeContact->get_Email3Address(&tmpVal);
    }

    else if (propertyName == L"Email1AddressType") {
        hr = pSafeContact->get_Email1AddressType(&tmpVal);
    }
    else if (propertyName == L"Email2AddressType") {
        hr = pSafeContact->get_Email2AddressType(&tmpVal);
    }
    else if (propertyName == L"Email3AddressType") {
        hr = pSafeContact->get_Email3AddressType(&tmpVal);
    }
    // Not used by now...
    //else if (propertyName == L"Email1DisplayName") {
    //    hr = pSafeContact->get_Email1DisplayName(&tmpVal);
    //}
    //else if (propertyName == L"Email2DisplayName") {
    //    hr = pSafeContact->get_Email2DisplayName(&tmpVal);
    //}
    //else if (propertyName == L"Email3DisplayName") {
    //    hr = pSafeContact->get_Email3DisplayName(&tmpVal);
    //}
    else if (propertyName == L"IMAddress") {
        hr = pSafeContact->get_IMAddress(&tmpVal);
    }


    if (FAILED(hr)) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE, itemType.c_str(), propertyName.c_str());
        throwClientException(getLastErrorMsg());
    }

    if (tmpVal) {
        propertyValue = tmpVal;

        //
        // Manage correction of EX->SMTP address: always return SMTP address automatically.
        //
        if ( (propertyName == L"Email1Address" && pSafeContact->GetEmail1AddressType() == (_bstr_t)L"EX") ||
             (propertyName == L"Email2Address" && pSafeContact->GetEmail2AddressType() == (_bstr_t)L"EX") ||
             (propertyName == L"Email3Address" && pSafeContact->GetEmail3AddressType() == (_bstr_t)L"EX") ) {

            ClientApplication* ol = ClientApplication::getInstance();
            propertyValue = ol->getSMTPfromEX(propertyValue);
        }
        else if (propertyName == L"Email1AddressType" ||
                 propertyName == L"Email2AddressType" ||
                 propertyName == L"Email3AddressType" ) {
            if (propertyValue == L"EX") {
                propertyValue = L"SMTP";
            }
        }
    }

    return propertyValue;
}


/*
 * Return the Item property value from the property name.
 * Here are processed properties that need specific conversion of 
 * the property value.
 */
const wstring ClientContact::getComplexProperty(const wstring& propertyName) {
 
    wstring propertyValue = EMPTY_WSTRING;
    WCHAR tmp[32];
    DATE date;

    //
    // Name: correct first char...
    //
    if (propertyName == L"FirstName" ||
        propertyName == L"LastName") {
        WCHAR* fullName = (WCHAR*)pContact->GetFullName();
        wstring name    = getSimpleProperty(propertyName);
        if (fullName) {
            propertyValue = getCorrectNameValue(name, fullName);
        }
        else {
            propertyValue = name;
        }
    }
    
    //
    // Date
    //
    else if (propertyName == L"Anniversary") {
        date = pContact->GetAnniversary();
        if (date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, FALSE, true);       // "yyyyMMdd"
        }
    }

    else if (propertyName == L"Birthday") {
        date = pContact->GetBirthday();
        if (date < LIMIT_MAX_DATE) {
            doubleToSystemTime(propertyValue, date, FALSE, true);       // "yyyyMMdd"
        }
    }

    else if (propertyName == L"LastModificationTime") {                 // "(double format)"
        date = pContact->GetLastModificationTime();
        swprintf_s(tmp, L"%.12f", date);
        propertyValue = tmp;
    }

    // Separator for Categories in Outlook can be "," or ";".
    // We use only ",".
    else if (propertyName == L"Categories") {
        _bstr_t categories = pContact->GetCategories();
        if (categories.length() > 0) {
            propertyValue = (WCHAR*)categories;
            replaceAll(L";", L",", propertyValue);
        }
    }

    // Read the contact's picture from disk.
    else if (propertyName == L"Photo") {

        // Safe check: picture supported since Outlook 2003
        ClientApplication* ol = ClientApplication::getInstance();
        int version = _wtoi(ol->getVersion().c_str());
        if (version < 11) {
            return L"";
        }

        bool hasPicture = vBoolToBool(pContact->GetHasPicture());
        if (hasPicture) {
            bool found = false;
            _bstr_t picName = PICTURE_OUTLOOK_NAME;
            AttachmentsPtr attachments = pContact->GetAttachments();
            if (attachments) {
                int num = attachments->GetCount();
                for (int i=0; i<num; i++) {
                    AttachmentPtr att = attachments->Item(i+1);             // index start from '1'
                    if (att && att->GetFileName() == picName) {
                        hr = att->SaveAsFile(tmpPicturePath.c_str());
                        if (SUCCEEDED(hr)) {
                            found = true;
                        }
                        break;
                    }
                }
            }
            if (found) {
                 WCHAR* b64Picture = getPictureFromFile(tmpPicturePath);
                 if (b64Picture) {
                     propertyValue = b64Picture;
                     delete [] b64Picture;
                 }
                 DeleteFile(tmpPicturePath.c_str());
            }
        }
    }

    return propertyValue;
}


#include "event/ManageListener.h"

// ------------------------------- SET PROPERTY -------------------------------
/*
 * Set the Item property value for the specific property name.
 * Here are processed properties that need specific conversion of the property value. 
 *
 * @param propertyName  : the name of the property
 * @param propertyValue : the value to store
 * @return              : 0 if no errors, 1 if errors
 */
int ClientContact::setComplexProperty(const wstring& propertyName, const wstring& propertyValue) {

    DATE date;
 
    //
    // Name: manage first char...
    //
    if (propertyName == L"FirstName" ||
        propertyName == L"LastName") {
        // Not an error!!! Set 2 times to avoid automatic 
        // replace of first char to upper case! (funny, uh?)
        setSimpleProperty(propertyName, propertyValue);
        setSimpleProperty(propertyName, propertyValue);
    }

    //
    // Date
    //
    else if (propertyName == L"Anniversary") {              // Expected "yyyyMMdd" (UTC also accepted)
        systemTimeToDouble(propertyValue, &date, true);
        if (!date) date = REFERRED_MAX_DATE;                // Use REFERRED_MAX_DATE to clear
        hr = pContact->put_Anniversary(date);
        
        if (SUCCEEDED(hr) && date < LIMIT_MAX_DATE) {       // Outlook will create automatically an event!
            willCreateAnniversaryEvent = true;
        }
    }

    else if (propertyName == L"Birthday") {                 // Expected "yyyyMMdd" (UTC also accepted)
        systemTimeToDouble(propertyValue, &date, true);
        if (!date) date = REFERRED_MAX_DATE;                // Use REFERRED_MAX_DATE to clear
        hr = pContact->put_Birthday(date);

        if (SUCCEEDED(hr) && date < LIMIT_MAX_DATE) {       // Outlook will create automatically an event!
            willCreateBirthdayEvent = true;
        }
    }

    // Separator for Categories in Outlook can be "," or ";".
    // Nothing to do (both accepted by Outlook).
    else if (propertyName == L"Categories") {
        pContact->PutCategories(propertyValue.c_str());
    }


    // Photo
    else if (propertyName == L"Photo") {
        // Safe check: picture is supported since Outlook 2003
        ClientApplication* ol = ClientApplication::getInstance();
        int version = _wtoi(ol->getVersion().c_str());
        if (version < 11) {
            return 0;
        }

        // Remove picture if any
        bool hasPicture = vBoolToBool(pContact->GetHasPicture());
        if (hasPicture) {
            hr = pContact->RemovePicture();
            if (FAILED(hr)) {
                LOG.error("Error removing picture for contact \"%ls\"", getSafeItemName(this).c_str());
            }
        }
        if (propertyValue != L"") {
            // Save picture to tmp file
            if (savePictureToFile(propertyValue, tmpPicturePath)) {
                LOG.error("Error saving the picture for contact \"%ls\"", getSafeItemName(this).c_str());
                return 1;
            }
            // Add picture to Outlook and delete tmp file
            ManageListener& manage = ManageListener::getInstance();
            pContact->AddPicture(tmpPicturePath.c_str());
            DeleteFile(tmpPicturePath.c_str());
        }
    }

    if (FAILED(hr)) {
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_SET, propertyName.c_str(), propertyValue.c_str(), itemType.c_str());
        throwClientException(getLastErrorMsg());
        return 1;
    }
    return 0;
}




// ------------------------------- Other methods -------------------------------


// Return true if Anniversary event has been created by Outlook during saveItem().
// Outlook creates this event automatically if Anniversary is set.
bool ClientContact::createdAnniversaryEvent() {

    BOOL saved = vBoolToBOOL(pContact->GetSaved());
    if (saved && willCreateAnniversaryEvent) {
        return true;
    }
    else {
        return false;
    }
}

// Return true if Birthday event has been created by Outlook during saveItem().
// Outlook creates this event automatically if Birthday is set.
bool ClientContact::createdBirthdayEvent() {

    BOOL saved = vBoolToBOOL(pContact->GetSaved());
    if (saved && willCreateBirthdayEvent) {
        return true;
    }
    else {
        return false;
    }
}



/*
 * Internal utility: get the correct format of name fields.
 * Microsoft Outlook automatically converts the first character of some
 * fields into upper-case, even if they are in lower-case.
 * So this function tries to extract these two fields from 'fullName' field, because
 * here the correct format is preserved.
 * NOTE:
 * 'FirstName' and 'LastName' are key values for synchronization and one
 * wrong character could lead to duplicates.
 */
const wstring ClientContact::getCorrectNameValue(const wstring& name, const wstring& fullName) {

    wstring::size_type start;

    //
    // First search the name as passed.
    //
    start = fullName.find(name, 0);
    if (start != wstring::npos) {
        // found!
        return name;
    }
    
    //
    // Get name with 1st letter lower case.
    //
    wstring lowerName = name;
    lowerName[0] = towlower(lowerName[0]);

    // Search the name modified.
    start = fullName.find(lowerName, 0);
    if (start != wstring::npos) {
        // found!
        return lowerName;
    }
    else {
        // not found: default = name.
        return name;
    }
}




/**
 * Reads the picture file from disk (path = filename) and returns the
 * file content encoded in base64, already splitted in lines of 72 chars.
 * @param filename   the path of file to read from disk
 * @return           the b64 file content (a new allocated WCHAR* buffer)
 */
WCHAR* ClientContact::getPictureFromFile(const wstring& filename) {

    if(!filename.length()) {
        return NULL;
    }

    char* name    = toMultibyte(filename.c_str());
    char* msg     = NULL;
    char* b64msg  = NULL;
    WCHAR* ret    = NULL;
    size_t msglen = 0;

    // Read file
    if (!readFile(name, &msg, &msglen, true)) {
        LOG.error("Error reading file %s", name);
        goto finally;
    }

    // Encode the file
    b64msg = encodeWithSpaces(msg, msglen);
    if (b64msg) {
        ret = toWideChar(b64msg);
    }

finally:
    if (name)   delete [] name;
    if (msg)    delete [] msg;
    if (b64msg) delete [] b64msg;

    return ret;
}


/**
 * Encode the message in base64, splitting the result in lines of 72 columns
 * each.
 * @return  a new allocated char* buffer
 */
char* ClientContact::encodeWithSpaces(const char *msg, int len) {
    int i, step=54, dlen=0;

    char* res = new char[len*3]; // b64 is 4/3, but we have also the newlines....
    memset(res, 0, len*3);
    res[0] = ' ';
    res[1] = ' ';
    res[2] = ' ';
    res[3] = ' ';
    char* ret = &res[4];
    for(i=0; i<len; i+=step) {
        if(len-i < step) {
            step = len-i;
        }
        dlen += b64_encode(ret+dlen, (void *)(msg+i), step);
        ret[dlen++]='\r';
        ret[dlen++]='\n';
        ret[dlen++]=' ';
        ret[dlen++]=' ';
        ret[dlen++]=' ';
        ret[dlen++]=' ';
    }

    // Terminate the string
    ret[dlen]=0;
    int ll = strlen(res);
    return res;
}


/**
 * Save picture content to a file 'filename' on disk.
 * Picture content is passed in b64 format, will be trimmed and decoded.
 * @param b64content  the file content in base64
 * @param filename    the path of file to be saved
 * @return            0 if picture saved without errors
 */
int ClientContact::savePictureToFile(const wstring& b64content, const wstring& filename) {

    int ret = 0;
    int idx;
    if(!filename.length()) {
        return 1;
    }

    // Need to copy the content: remove 'space', '\r', '\n'
    wstring picture = b64content;
    const wstring charsToRemove = L" \r\n";
    while( (idx = picture.find_first_of(charsToRemove)) >= 0 ) {
        picture.replace(idx, 1, L"");            
    }

    // Decode the file content
    char* valueA = toMultibyte(picture.c_str());
    char* name   = toMultibyte(filename.c_str());
    int rc = b64_decode(valueA, valueA);        
    if(rc > 0) {
        // Write file to disk
        if (saveFile(name, valueA, rc, true) == false) {
            ret = 1;
        }
    }

    if (valueA)  delete [] valueA;
    if (name)    delete [] name;
    return ret;
}





/* 
 * Copy Constructor
*/
ClientContact::ClientContact(const ClientContact& c) {
    hr              = c.hr;
    ID              = c.ID;
    itemType        = c.itemType;
    parentPath      = c.parentPath;
    pContact        = c.pContact;
    pItemProperties = c.pItemProperties;
    pItemProperty   = c.pItemProperty;
    propertiesIndex = c.propertiesIndex;
    propertiesCount = c.propertiesCount;
    propertyMap     = c.propertyMap;

    createSafeContactInstance();
    pSafeContact    = c.pSafeContact;

    willCreateAnniversaryEvent = c.willCreateAnniversaryEvent;
    willCreateBirthdayEvent    = c.willCreateBirthdayEvent;
}


/* 
 * Operator =
*/
ClientContact ClientContact::operator=(const ClientContact& c) {

    ClientContact cnew;

    cnew.hr              = c.hr;
    cnew.ID              = c.ID;
    cnew.itemType        = c.itemType;
    cnew.parentPath      = c.parentPath;
    cnew.pContact        = c.pContact;
    cnew.pItemProperties = c.pItemProperties;
    cnew.pItemProperty   = c.pItemProperty;
    cnew.propertiesIndex = c.propertiesIndex;
    cnew.propertiesCount = c.propertiesCount;
    cnew.propertyMap     = c.propertyMap;

    cnew.createSafeContactInstance();
    cnew.pSafeContact    = c.pSafeContact;

    cnew.willCreateAnniversaryEvent = c.willCreateAnniversaryEvent;
    cnew.willCreateBirthdayEvent    = c.willCreateBirthdayEvent;

    return cnew;
}

