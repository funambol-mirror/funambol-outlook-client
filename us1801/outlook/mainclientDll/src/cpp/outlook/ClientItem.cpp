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
#include "winmaincpp.h"
#include "outlook/defs.h"

#include "outlook/ClientItem.h"
#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "outlook/itemProps.h"


using namespace std;



// Constructor
ClientItem::ClientItem() {

    hr              = S_OK;
    ID              = EMPTY_WSTRING;
    itemType        = EMPTY_WSTRING;
    parentPath      = EMPTY_WSTRING;
    pItemProperties = NULL;
    pItemProperty   = NULL;
    propertiesIndex = 0;
    propertiesCount = 0;

    propertyMap.clear();
}

// Destructor
ClientItem::~ClientItem() {

    propertyMap.clear();

    if (pItemProperty)   { pItemProperty.Release();   }
    if (pItemProperties) { pItemProperties.Release(); }
}


const wstring& ClientItem::getID() {
    return ID;
}

const wstring& ClientItem::getType() {
    return itemType;
}

const wstring& ClientItem::getParentPath() {
    return parentPath;
}


const int ClientItem::getPropertiesCount() {
    return propertiesCount;
}

const int ClientItem::getPropertiesIndex() {
    return propertiesIndex;
}



/**
 * Creates the Property Map: <propertyName, propertyIndex>
 * -----------------------------------------------------------
 * Creates the corrispondence of each property with the index of the
 * same property stored in Outlook.
 * This method should be called only once, when item accessed for the first time.
 */
void ClientItem::createPropertyMap() {

    wstring propertyName;
    int i = 0;

    try {
        for (i=0; i<propertiesCount; i++) {
            pItemProperty = pItemProperties->Item(i);
            propertyName = (WCHAR*)pItemProperty->GetName();

            // Fill the property map
            propertyMap[propertyName] = i;
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_MAP, itemType.c_str(), i);
        throwClientFatalException(getLastErrorMsg());
    }

}




/**
 * Returns the item property value from the property name.
 * PropertyMap is used to get the index of selected property, then
 * different methods are called to retrieve the correct property value:
 * 1. getSafeProperty     -> for properties with security patch protection
 * 2. getComplexProperty  -> for properties that need specific conversion
 * 3. getSimpleProperty   -> for all other properties
 *
 * @param propertyName the name  of the property
 * @return             the value of the property (wstring)
 */
const wstring ClientItem::getProperty(const wstring& propertyName) {

    wstring propertyValue = EMPTY_WSTRING;
    wstring olPropName;

    // Only first time
    if (!propertyMap.size()) {
        createPropertyMap();
    }
    // SIF property name -> Outlook property name
    olPropName = convertPropertyName(propertyName);
    

    // Access the property map to get the index
    // ----------------------------------------
    propertiesIndex = propertyMap[olPropName];

    // Note:
    // '0' if first property but also if property not found!
    // However, some complex properties could not be found (i.e. recurrence properties)
    // Test on "property not found" is in getSimpleProperty() due to some complex props
    // that could not be found (e.g. recurrence props).


    // Retrieve property value from Outlook (use propertiesIndex or name)
    // ------------------------------------
    try {
        // 1. Properties protected by Outlook Security Patch.
        if (isSecureProperty(olPropName)) {
            propertyValue = getSafeProperty(olPropName);
        }

        // 2. Properties that need specific conversion.
        else if (isComplexProperty(olPropName)) {
            propertyValue = getComplexProperty(olPropName);
        }

        // 3. All other properties (string/long types).
        else {
            propertyValue = getSimpleProperty(olPropName);
        }
    }

    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE, itemType.c_str(), propertyName.c_str());
        goto error;
    }

    return propertyValue;

error:
    throwClientException(getLastErrorMsg());
    return EMPTY_WSTRING;
}





/**
 * Convert SIF property name into Outlook property name.
 * Uses the vector 'diffPropertyNames' with correspondances.
 * If property not found, it means that the name is the same.
 */
const wstring ClientItem::convertPropertyName(const wstring& SIFName) {

    wstring outlookName = SIFName;

    for(int i=0; i<DIFF_PROPERTY_NAMES_COUNT; i++) {
        if (SIFName == diffPropertyNames[i].SIFName) {
            outlookName = diffPropertyNames[i].OutlookName;
            break;
        }
    }
    return outlookName;
}

    


/**
 * Returns the Item property value from the propertiesIndex (class member).
 * Used for normal properties, simple copy into a string value.
 *
 * @param propertyName  the name of the property
 * @return              the value retrieved
 */
const wstring ClientItem::getSimpleProperty(const wstring& propertyName) {

    // Test if property not found (index 0...)
    if (propertiesIndex == 0) {
        if (propertyName != (WCHAR*)pItemProperties->Item(0)->GetName()) {
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_NOT_FOUND, propertyName.c_str(), itemType.c_str());
            throwClientException(getLastErrorMsg());
            return EMPTY_WSTRING;
        }
    }


    pItemProperty     = pItemProperties->Item(propertiesIndex);
    _bstr_t bstrValue = (_bstr_t)pItemProperty->GetValue();

    return (WCHAR*)bstrValue;

    // --- Alternate way: test which is faster (TODO) ---
    //_variant_t val;
    //_variant_t ix = propertiesIndex;
    //pItemProperties->raw_Item((VARIANT)ix, &pItemProperty);
    //hr = pItemProperty->get_Value(&val);

    //if (val.vt == VT_BSTR) {
    //    return (WCHAR*)val.bstrVal;
    //}
    //else {
    //    WCHAR tmp[20];
    //    wsprintf(tmp, L"%li", val.lVal);
    //    return tmp;
    //}
    // --------------------------------------------------
}




/**
 * Set the item property value for the specific property name.
 * PropertyMap is used to get the index of selected property.
 *
 * @param propertyName   the name  of the property
 * @param propertyValue  the value to store
 * @return               0 if no errors, 1 if errors
 */
int ClientItem::setProperty(const wstring& propertyName, const wstring& propertyValue) {

    int ret = 1;

    // Only first time
    if (!propertyMap.size()) {
        createPropertyMap();
    }
    // SIF property name -> Outlook property name
    wstring olPropName = convertPropertyName(propertyName);


    // Access the property map to get the index
    // ----------------------------------------
    propertiesIndex = propertyMap[olPropName];

    // Note:
    // '0' if first property but also if property not found!
    // However, some complex properties could not be found (i.e. recurrence properties)
    // Test on "property not found" is in setSimpleProperty() due to some complex props
    // that could not be found (e.g. recurrence props).


    // Set item property
    // -----------------
    try {
        // 1. Properties that need specific conversion.
        if (isComplexProperty(olPropName)) {
            ret = setComplexProperty(olPropName, propertyValue);
        }

        // 2. All other properties (also safe props, don't need redeption in write mode).
        else {
            ret = setSimpleProperty(olPropName, propertyValue);
        }
    }
    catch(_com_error &e) {
        manageComErrors(e);
        setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_VALUE_SET, propertyName.c_str(), propertyValue.c_str(), itemType.c_str());
        ret = 1;
        goto error;
    }

    return ret;

error:
    throwClientException(getLastErrorMsg());         // TBD: comment here to avoid dropping item at first wrong prop...
    return ret;
}



/*
 * Set the Item property value for the specific property name.
 * Used for normal properties, simply write the string value.
 *
 * @param propertyName  : the name  of the property
 * @param propertyValue : the value to store
 * @return              : 0 if no errors, 1 if errors
 */
int ClientItem::setSimpleProperty(const wstring& propertyName, const wstring& propertyValue) {

    // Test if property not found (index 0...) 
    if (propertiesIndex == 0) {
        if (propertyName != (WCHAR*)pItemProperties->Item(0)->GetName()) {
            setErrorF(getLastErrorCode(), ERR_OUTLOOK_PROP_NOT_FOUND, propertyName.c_str(), itemType.c_str());
            return 1;
        }
    }

    pItemProperty = pItemProperties->Item(propertiesIndex);
    _bstr_t bstrValue = (_bstr_t)propertyValue.c_str();    
    pItemProperty->PutValue(bstrValue);
    return 0;
}
