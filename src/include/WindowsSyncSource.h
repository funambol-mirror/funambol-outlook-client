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

#ifndef INCL_WINDOWS_SYNC_SOURCE
#define INCL_WINDOWS_SYNC_SOURCE

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "base/fscapi.h"
#include "spds/constants.h"
#include "spds/SyncItem.h"
#include "spds/SyncMap.h"
#include "spds/SyncStatus.h"
#include "spds/SyncSource.h"
#include"WindowsSyncSourceConfig.h"
#include "outlook/ClientApplication.h"

#include <string>
#include <list>


typedef std::list<std::wstring>         itemKeyList;
typedef itemKeyList::iterator           itemKeyIterator;

/** @cond DEV */
///
/// Error codes for WindowsSyncSource:
/// Codes  < 100 = soft errors -> SOURCE_ERROR not required
/// Codes >= 100 = bad  errors -> after 10 bad errors 'state' = SOURCE_ERROR
///
#define ERR_CODE_FOLDER_PATH              1     /**< Item ignored (path not correct for current sync)    */
#define ERR_CODE_DELETE_NOT_FOUND         2     /**< Item not deleted cause not found                    */
#define ERR_CODE_OLD_ITEMS_PATH         100
#define ERR_CODE_UPDATE_NOT_FOUND       101
#define ERR_CODE_FOLDER_OPEN            102
#define ERR_CODE_ITEM_CREATE            104
#define ERR_CODE_ITEM_FILL              105
#define ERR_CODE_ITEM_SAVE              106
#define ERR_CODE_DELETE                 107
#define ERR_CODE_ITEM_BAD_TYPE          108
#define ERR_CODE_ITEM_GET               109
#define ERR_CODE_NO_ROOT_FOLDER         110
#define ERR_CODE_OPEN_OUTLOOK           111
#define ERR_CODE_READ_ALL_ITEMS         112

#define MAX_SOURCE_ERRORS                10     /**< Max number of bad errors allowed                    */

#define TIMESTAMP_DELAY                   2     /**< Outlook introduces some delay while saving items    */
/** @endcond */


/**
 *****************************************************************************************
 * This class is the extension of SyncSource class for specific Client objects.
 * Implements all methods to get/add syncItems on Client,
 * plus there are some internal methods to handle operations on the SyncSource.
 * 
 * All syncItems are managed inside this class, the only difference is the member 'name'.
 *****************************************************************************************
 */
class WindowsSyncSource : public SyncSource {

private:

    /// Pointer to the sole instance of ClientApplication (singleton)
    ClientApplication* outlook;


protected:

    /// Configuration object for the source. It's a reference to WindowsSyncSourceConfig
    /// object owned by OutlookConfig. It's automatically initialized in the constructor.
    WindowsSyncSourceConfig& winConfig;


    /// Lists of item keys.
    itemKeyList  allItems;
    itemKeyList  newItems;
    itemKeyList  modItems;
    itemKeyList  delItems;
    itemKeyList  allItemsPaths;         // This is the list of all items path in Outlook

    /// Iterators of lists.
    itemKeyIterator  iAll;
    itemKeyIterator  iNew;
    itemKeyIterator  iMod;
    itemKeyIterator  iDel;
    itemKeyIterator  iAllPaths;


    /// Counter of number of errors occurred
    unsigned int numErrors;

    /// Default folder path in Outlook
    std::wstring defaultFolderPath;


    // Internal utility methods:
    // -------------------------
    // Get the starting Outlook folder for this source.
    ClientFolder* getStartFolder();

    // Get all items inside 'folder' and push them (only keys) into 'listItems' list.
    void pushAllSubfolderItemsToList(ClientFolder* folder, itemKeyList& listItems, itemKeyList& listItemsPaths);
    void pushAllItemsToList         (ClientFolder* folder, itemKeyList& listItems, itemKeyList& listItemsPaths);

    // Fill internal itemKeyLists of NEW/MOD/DEL item keys from last successfull sync.
    int manageModificationsFromLastSync();

    // Add forced modified items (passed list) to the modItems list.
    int addForcedItemsToModList(itemKeyList& forcedItems);

    // Create/parse XML string with old item's keys.
    std::wstring createOldItems();
    void parseOldItems(std::wstring& data, itemKeyList& listItems, itemKeyList& listFolders);

    // Reset and close the data file (where current items ID are stored).
    int resetDataFile(const std::wstring& itemType);
    int closeDataFile(const std::wstring& itemType);

    // Returns true if path passed is allowed to sync items inside it.
    bool folderPathAllowed(const std::wstring& p);

    // Verifies if birthday/anniversary were created by Outlook (only for contacts save).
    void checkBirthdayAnniversary(ClientItem* cItem);
    int  deleteAppointment(ClientItem* cItem, const std::wstring& propertyName);

    // Common actions to do when an error occurs inside SyncSource.
    void manageSourceError(const int errorCode, const char* errorMsg);

    /// Calls manageSourceError, with msg formatted in a string.
    void manageSourceErrorF(const int errorCode, const char *msgFormat, ...);

    void extractFolder(const std::wstring dataString, const std::wstring dataType, std::wstring& path);


public:

    /**
     * Constructor: creates a WindowsSyncSource with the specified name and config.
     *
     * @param name   the name of the SyncSource
     * @param wsc    to init by reference the 'winConfig' member (for client-specific props)
     *               'config' member is initialized from wsc->getCommonConfig() to get the original
     *               common properties used by API.
     */
    WindowsSyncSource::WindowsSyncSource(const WCHAR* name, WindowsSyncSourceConfig* wsc);

    ~WindowsSyncSource();


    /// Used to access configuration of SS
    const WindowsSyncSourceConfig& getConfig() const;
    WindowsSyncSourceConfig& getConfig();

    int beginSync();
    int endSync();

    void WindowsSyncSource::setItemStatus(const WCHAR* key, int status);
    void assign(WindowsSyncSource& s);
    //ArrayElement* clone();
    int removeAllItems() {return 0; }

    // --------- Methods to get syncItems from Client --------------
    SyncItem* getFirstItem();
    SyncItem* getNextItem ();

    SyncItem* getFirstNewItem();
    SyncItem* getNextNewItem ();
    SyncItem* getFirstUpdatedItem();
    SyncItem* getNextUpdatedItem ();

    SyncItem* getFirstDeletedItem();
    SyncItem* getNextDeletedItem ();

    SyncItem* getFirstItemKey();
    SyncItem* getNextItemKey();


    // --------- Methods to set syncItems into Client --------------
    int addItem   (SyncItem& item);
    int updateItem(SyncItem& item);
    int deleteItem(SyncItem& item);

};

/** @} */
/** @endcond */
#endif
