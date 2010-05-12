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

#ifndef INCL_CLIENTMAIL
#define INCL_CLIENTMAIL

/** @cond OLPLUGIN */
/** @addtogroup outlook_items */
/** @{ */

#include "outlook/defs.h"
#include "outlook/ClientItem.h"

#include <string>

//Forward declaration of Folder
class ClientFolder;


////// ---------- NOT USED BY NOW -------- ///////////
class ClientMail : public ClientItem {

private:

    // Pointer to microsoft outlook objects.
    _MailItemPtr     pMail;

    // Pointer to Redemption safe objects.
    Redemption::ISafeMailItemPtr pSafeMail;


    void createSafeMailInstance();

    bool isSecureProperty (const std::wstring& propertyName);
    bool isComplexProperty(const std::wstring& propertyName);

    const std::wstring getSafeProperty   (const std::wstring& propertyName);
    const std::wstring getComplexProperty(const std::wstring& propertyName);

    int setComplexProperty(const std::wstring& propertyName, const std::wstring& propertyValue);

public:

    // Constructor
    ClientMail();
    ClientMail(const ClientMail& c);
    ClientMail operator=(const ClientMail& c);

    // Destructor
    ~ClientMail();


    // Set a COM pointer to this object: refresh all members.
    void setCOMPtr(_MailItemPtr& ptr, const std::wstring& itemID);
    void setCOMPtr(_MailItemPtr& ptr);

    // Returns a reference to the internal COM pointer.
    _MailItemPtr& getCOMPtr();

    //
    // Client operations on the item.
    //
    int saveItem();
    int deleteItem();
    ClientItem* copyItem();
    int moveItem(ClientFolder* destFolder);


    //
    // Methods to menage item properties.
    //
    //const std::wstring getFirstProperty();
    //const std::wstring getNextProperty();
    //const std::wstring getPreviousProperty();
    //const std::wstring getLastProperty();

    //int setProperty(const std::wstring& propertyName, const std::wstring& propertyValue);

};

/** @} */
/** @endcond */
#endif
