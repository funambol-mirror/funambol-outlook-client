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

#ifndef INCL_CLIENTITEM
#define INCL_CLIENTITEM

/** @cond OLPLUGIN */
/** @addtogroup outlook_items */
/** @{ */

#include "outlook/defs.h"

#include <map>
#include <string>



/**
******************************************************************************
* Wraps the generic Outlook item (could be any of the item types).
* Methods to get/set simple item properties are defined here, 
* more specific properties are retrieved using the implemented methods in
* each Client object.
* Class methods automatically catch and manage COM pointers exceptions.
* Class methods throw ClientException pointer in case of error.
******************************************************************************
*/
class ClientItem {

protected:

    std::wstring ID;            /**< The item entry-ID.                                 */
    std::wstring itemType;      /**< The item-type of the item (contact/task/mail...)   */
    std::wstring parentPath;    /**< The full path of parent folder.\                   */


    /// Map of item properties: <name, index>
    std::map<std::wstring,int> propertyMap;
    int propertiesCount;
    int propertiesIndex;

    /// Pointers to microsoft outlook objects.
    ItemPropertiesPtr   pItemProperties;
    ItemPropertyPtr     pItemProperty;

    /// Result of COM pointers operations.
    HRESULT hr;


    void createPropertyMap();
    const std::wstring convertPropertyName(const std::wstring& SIFName);

    virtual bool isSecureProperty  (const std::wstring& propertyName) = 0;
    virtual bool isComplexProperty (const std::wstring& propertyName) = 0;

    const         std::wstring getSimpleProperty (const std::wstring& propertyName);
    virtual const std::wstring getSafeProperty   (const std::wstring& propertyName) = 0;
    virtual const std::wstring getComplexProperty(const std::wstring& propertyName) = 0;

    int         setSimpleProperty (const std::wstring& propertyName, const std::wstring& propertyValue);
    virtual int setComplexProperty(const std::wstring& propertyName, const std::wstring& propertyValue) = 0;

public:

    /// Constructor
    ClientItem();

    /// Destructor
    virtual ~ClientItem();


    const std::wstring& getID();
    const std::wstring& getType();
    const std::wstring& getParentPath();


    //
    // Client operations on the item.
    //
    virtual int         saveItem() = 0;
    virtual int       deleteItem() = 0;
    virtual ClientItem* copyItem() = 0;
    //virtual int       moveItem(ClientFolder* destFolder) = 0;
    // saveAs/close...?


    //
    // Methods to menage item properties.
    //
    const int getPropertiesCount();
    const int getPropertiesIndex();

    /// To retrieve a property value from its name.
    const std::wstring getProperty(const std::wstring& propertyName);
    /// To set a property value from its name.
    int setProperty(const std::wstring& propertyName, const std::wstring& propertyValue);

};

/** @} */
/** @endcond */
#endif
