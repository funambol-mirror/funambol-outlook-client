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

#ifndef INCL_CLIENTCONTACT
#define INCL_CLIENTCONTACT

/** @cond OLPLUGIN */
/** @addtogroup outlook_items */
/** @{ */

#include "outlook/defs.h"
#include "outlook/ClientItem.h"

#include <string>


/// Standard name of the contact's picture in Outlook
#define PICTURE_OUTLOOK_NAME           L"ContactPicture.jpg"
#define PICTURE_TMP_NAME               L"~pic-tmp.jpg"



/**
 * Wraps the Outlook contact object.
 * Implements methods to get/set specific properties for this 
 * object type.
 */
class ClientContact : public ClientItem {

private:
    
    /// Pointer to microsoft outlook objects.
    _ContactItemPtr  pContact;

    /// Pointer to Redemption safe objects.
    Redemption::ISafeContactItemPtr pSafeContact;

    /// Used to know if Outlook will automatically create events during saveItem().
    bool willCreateAnniversaryEvent;
    bool willCreateBirthdayEvent;

    /// Where the picture is temporary saved.
    static std::wstring tmpPicturePath;

    
    void createSafeContactInstance();

    bool isSecureProperty (const std::wstring& propertyName);
    bool isComplexProperty(const std::wstring& propertyName);

    const std::wstring getSafeProperty   (const std::wstring& propertyName);
    const std::wstring getComplexProperty(const std::wstring& propertyName);

    int setComplexProperty(const std::wstring& propertyName, const std::wstring& propertyValue);

    /// Internal utility: get the correct format of firstName and lastName fields.
    const std::wstring getCorrectNameValue(const std::wstring& name, const std::wstring& fullName);

    // Methods to handle contact's picture.
    WCHAR* getPictureFromFile(const std::wstring& filename);
    char*  encodeWithSpaces(const char *msg, int len);
    int    savePictureToFile(const std::wstring& b64content, const std::wstring& filename);

public:

    /// Constructor
    ClientContact();
    ClientContact(const ClientContact& c);
    ClientContact operator=(const ClientContact& c);

    /// Destructor
    ~ClientContact();

    /// Set a COM pointer to this object: refresh all members.
    void setCOMPtr(_ContactItemPtr& ptr, const std::wstring& itemID);
    void setCOMPtr(_ContactItemPtr& ptr);

    // Returns a reference to the internal COM pointer.
    _ContactItemPtr& getCOMPtr();


    //
    // Client operations on the item.
    //
    int saveItem();
    int deleteItem();
    ClientItem* copyItem();
    //int moveItem(ClientFolder* destFolder);


    /// Returns true if a Anniversary event has been created silently by Outlook during saveItem().
    bool createdAnniversaryEvent();
    /// Returns true if a Birthday event has been created silently by Outlook during saveItem().
    bool createdBirthdayEvent();
};

/** @} */
/** @endcond */
#endif
