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

#include "base/Log.h"
#include "base/util/utils.h"
#include "base/stringUtils.h"
#include "spds/SyncItemStatus.h"
#include "event/FireEvent.h"
#include "winmaincpp.h"
#include "WindowsSyncSource.h"
#include "utils.h"
#include "HwndFunctions.h"
#include "outlook/ClientFolder.h"
#include "outlook/ClientException.h"
#include "outlook/ClientContact.h"
#include "SyncException.h"
#include "vocl/AppDefs.h"
#include "spds/constants.h"


using namespace std;



//------------------------------ WindowsSyncSource Methods --------------------------------------------
WindowsSyncSource::WindowsSyncSource(const WCHAR* name, WindowsSyncSourceConfig* wsc) : 
                                     SyncSource(name, wsc->getCommonConfig()),
                                     winConfig(*wsc) {
    outlook   = NULL;
    numErrors = 0;

    allItems.clear();
    newItems.clear();
    modItems.clear();
    delItems.clear();
    allItemsPaths.clear();
    filteredItems.clear();

    defaultFolderPath = EMPTY_WSTRING;

    forceOpenOutlook = false;
}


WindowsSyncSource::~WindowsSyncSource() {

    allItems.clear();
    newItems.clear();
    modItems.clear();
    delItems.clear();
    allItemsPaths.clear();
    filteredItems.clear();

    outlook = NULL;
}




/// read-only access to win configuration
const WindowsSyncSourceConfig& WindowsSyncSource::getConfig() const {
    return winConfig;
}

/// read-write access to win configuration
WindowsSyncSourceConfig& WindowsSyncSource::getConfig() {
    return winConfig;
}



/**
 * This is the first method called. 
 * It executes all operations to be done before synchronizing this source:
 * - opens Outlook session (if not yet done)
 * - gets the selected folder for synchronization defined in configuration
 * - creates a list of ALL items (keys) to be synchronized
 *
 * @return  0 if no errors occurred
 */
int WindowsSyncSource::beginSync() {
    checkAbortedSync();

    initWinItems();

    // From now we consider this source synced.
    winConfig.setIsSynced(true);

    //
    // ------- Opens Outlook session --------
    // Session will be closed after sync complete, in 'closeOutlook()'.
    //
    LOG.debug(DBG_OUTLOOK_OPEN, getName());
    try {
        outlook = ClientApplication::getInstance(!forceOpenOutlook);
    }
    catch (ClientException* e) {
        // Must set the errors, here could be a fatal exception
        manageSourceErrorF(ERR_CODE_OPEN_OUTLOOK, ERR_BEGIN_SYNC, getName());
        report->setState(SOURCE_ERROR);
        setErrorF(0, ERR_BEGIN_SYNC, getName());
        e->setExceptionData(e->getErrorMsg(), e->getErrorCode(), false, true);
        manageClientException(e);
        throwSyncException(getLastErrorMsg(), 2);
    }

    // Just store the default folder path (can be used more times during sync)
    ClientFolder* folder = NULL;
    try {
        folder = outlook->getDefaultFolder(getName());
        if (folder)  defaultFolderPath = folder->getPath();
        else         LOG.error(ERR_FOLDER_DEFAULT_PATH, getName());
    }
    catch (...) {
        LOG.error(ERR_FOLDER_DEFAULT_PATH, getName());
    }

    // This is the folder we want to sync (folder selected from config).
    folder = getStartFolder();
    if (!folder) {
        manageSourceErrorF(ERR_CODE_FOLDER_PATH_MATCH, ERR_BEGIN_SYNC, getName());
        setErrorF(ERR_CODE_FOLDER_PATH_MATCH, getLastErrorMsg());
        closeOutlook();
        throwSyncException("Folder paths do not match", ERR_CODE_FOLDER_PATH_MATCH);
    }

    defaultFolderPath = folder->getPath();

    if ( strcmp(getConfig().getName(), "appointment")== 0 && getSyncMode() == SYNC_SLOW ){
            DateFilter& f = getDateFilter();
            filterDirection = (int)(f.getDirection());
            f.setDirection(DateFilter::DIR_INOUT);
    }

    // Update the filters.
    // Some filters can change, based on the current config (time..)
    updateFilters();

    //
    // Create the list of ALL items.
    //
    allItems.clear();
    allItemsPaths.clear();
    filteredItems.clear();
    try {
        if (winConfig.getUseSubfolders() == true) {
            pushAllSubfolderItemsToList(folder, allItems, allItemsPaths);
        }
        else {
            pushAllItemsToList(folder, allItems, allItemsPaths);
        }
    }
    catch (ClientException* e) {
        // Must set the errors, here could be a fatal exception
        manageSourceErrorF(ERR_CODE_READ_ALL_ITEMS, ERR_BEGIN_SYNC, getName());
        report->setState(SOURCE_ERROR);
        manageClientException(e);
        goto error;
    }

    checkAbortedSync();
    return 0;

error:
    report->setState(SOURCE_ERROR);
    return 1;
}



/**
 * This is the last method called. 
 * It executes all operations to be done after this source has been synchronized:
 * - creates a list of ALL items (keys) currently on Outlook
 * - formats the list as an XML with item keys/path
 * - stores the XML string into FileSystem (create a cache file)
 *
 * @return  0 if no errors occurred
 */
int WindowsSyncSource::endSync() {
    itemKeyIterator it;
    std::map<std::wstring, std::wstring>::iterator mit;

    LOG.debug("Ending sync for '%ls'", getName());

    //checkAbortedSync();

    if ( strcmp(getConfig().getName(), "appointment")== 0 && getSyncMode() == SYNC_SLOW ){
            DateFilter& filter = getDateFilter();
            filter.setDirection(((DateFilter::FilterDirection)filterDirection));
    }

    int ret = 0;
    WCHAR* oldItemsPath = readDataPath(getName());
    if (!oldItemsPath) {
        manageSourceError(ERR_CODE_OLD_ITEMS_PATH, getLastErrorMsg());
        return 1;
    }

    LOG.debug("Opened old items db");

    //
    // Format data string as an XML with items keys.
    //
    wstring data = createOldItems();

    LOG.debug("Old items list created");

    if (writeToFile(data, oldItemsPath)) {

        // directories could not exist... (first time).
        if (makeDataDirs()) {
            LOG.error(getLastErrorMsg());
            goto error;
        }
        // retry
        if (writeToFile(data, oldItemsPath)) {
            LOG.error(getLastErrorMsg());
            goto error;
        }
    }

    LOG.debug("Wrote items db");

    // For appointments: close the file for 'forced modified items'.
    if (!wcscmp(getName(), APPOINTMENT)) {
        closeDataFile(APPOINTMENT_FORCED_MODIFIED);
    }

    LOG.debug("Closed db");

    writeIdMap(idMap);

    ret = 0;
    goto finally;

error:
    setErrorF(getLastErrorCode(), ERR_END_SYNC, getName());
    report->setLastErrorMsg(getLastErrorMsg());
    report->setState(SOURCE_ERROR);
    ret = 1;
    goto finally;

finally:

    LOG.debug("Setting end timestamp");
    // Set end timestamp to config: here this source is finished.
    winConfig.setEndTimestamp((unsigned long)time(NULL));

    if (oldItemsPath) {
        delete [] oldItemsPath;
        oldItemsPath = NULL;
    }

    LOG.info("Clearing cache for %ls", getName());
    clearCache();

    return ret;
}



// **********************************************************************************************
// ************************* Methods to get items from Client to Server *************************
// **********************************************************************************************
/**
 * It's called in case of full-sync or refresh-from-client sync.
 * @return the first SyncItem of allItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getFirstItem() {
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

  
    // For appointments: reset the 'appointment_modified' file,
    // ready to append items for this sync session.
    if (!wcscmp(getName(), APPOINTMENT)) {
        resetDataFile(APPOINTMENT_FORCED_MODIFIED);
    }


    // Fire client's number of changes (full sync)
    long noc = allItems.size();
    fireSyncSourceEvent(winConfig.getURI(), winConfig.getName(), getSyncMode(), noc, SYNC_SOURCE_TOTAL_CLIENT_ITEMS);


    if (allItems.size() > 0) {

        iAll = allItems.begin();

        try {
            cItem = outlook->getItemFromID(*iAll, getName());
            sItem = convertToSyncItem(cItem, winConfig.getType(), defaultFolderPath);
            LOG.info(INFO_GET_ITEM, getName(), getSafeItemName(cItem).c_str());
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iAll++;
        if (!sItem) {
            return getNextItem();
        }
    }

    idMap.clear();
    idMapReverse.clear();

    return sItem;
}


/**
 * It's called in case of full-sync or refresh-from-client sync.
 * @return the next SyncItem of allItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getNextItem() {
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

    if ( (allItems.size() > 0) && (iAll != allItems.end()) ) {
        try {
            cItem = outlook->getItemFromID(*iAll, getName());
            sItem = convertToSyncItem(cItem, winConfig.getType(), defaultFolderPath);
            LOG.info(INFO_GET_ITEM, getName(), getSafeItemName(cItem).c_str());
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iAll++;
        if (!sItem) {
            return getNextItem();
        }
    }
    return sItem; 
}


/**
 * It's called in case of two-way sync. 
 * Creates the lists of newItems, modItems and delItems, comparing differences 
 * between allItems list and the list of all items from the last sync (stored in cache files).
 * @return the first SyncItem of newItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getFirstNewItem() {
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

    //
    // Create list of NEW/MOD/DEL items from previous sync.
    //

    int manageCode = manageModificationsFromLastSync();
    if (manageCode > 0)
    {
        LOG.info(INFO_OLD_ITEMS_NOT_FOUND);
    }
    else if (manageCode < 0 && DLLCustomization::warnOnLargeDelete)
    {
        WCHAR temp[512];
        LOG.info(ERR_NO_DATA_ITEM, getName());
        swprintf(temp, 512, WMSG_BOX_NO_DATA_ITEM, getName(), getName());
        bool cancel = (IDNO == MessageBox(NULL,temp, WPROGRAM_NAME, MB_YESNO));
        if (cancel)
        {
            manageSourceError(ERR_CODE_ITEM_GET, getLastErrorMsg());
            //ClientApplication::invalidate();
            //throwSyncException(lastErrorMsg, ERR_CODE_ITEM_GET);
            //softTerminateSync();
            SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_CANCEL_SYNC, NULL, NULL);
            //checkAbortedSync();
        }
    }



    // For appointments: reset the 'appointment_modified' file,
    // ready to append items for this sync session.
    if (!wcscmp(getName(), APPOINTMENT)) {
        resetDataFile(APPOINTMENT_FORCED_MODIFIED);
    }


    // Fire client's number of changes (two-way sync)
    long noc = newItems.size() + modItems.size() + delItems.size();
    fireSyncSourceEvent(winConfig.getURI(), winConfig.getName(), getSyncMode(), noc, SYNC_SOURCE_TOTAL_CLIENT_ITEMS);


    //
    // Get first new item
    //
    if (newItems.size() > 0) {
        
        iNew = newItems.begin();

        try {
            cItem = outlook->getItemFromID(*iNew, getName());
            sItem = convertToSyncItem(cItem, winConfig.getType(), defaultFolderPath, false);
            LOG.info(INFO_GET_NEW_ITEM, getName(), getSafeItemName(cItem).c_str());
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET_NEW, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iNew++;
        if (!sItem) {
            return getNextNewItem();
        }
    }
    return sItem;
}

/**
 * It's called in case of two-way sync.
 * @return the next SyncItem of newItems list
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getNextNewItem() {   
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

    if ( (newItems.size() > 0) && (iNew != newItems.end()) ) {
        try {
            cItem = outlook->getItemFromID(*iNew, getName());
            sItem = convertToSyncItem(cItem, winConfig.getType(), defaultFolderPath);
            LOG.info(INFO_GET_NEW_ITEM, getName(), getSafeItemName(cItem).c_str());
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET_NEW, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iNew++;
        if (!sItem) {
            return getNextNewItem();
        }
    }
    return sItem; 
}


/**
 * It's called in case of two-way sync.
 * @return the first SyncItem of modItems list
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getFirstUpdatedItem() {
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

    if (modItems.size() > 0) {

        iMod = modItems.begin();
        std::wstring key = *iMod;

        try {
            cItem = outlook->getItemFromID(*iMod, getName());
            if (cItem && cItem->isReadOnly()) {
                sItem = NULL;
            } else {
                sItem = convertToSyncItem(cItem, config->getType(), defaultFolderPath);
                LOG.info(INFO_GET_UPDATED_ITEM, getName(), getSafeItemName(cItem).c_str());
            }
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET_MOD, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iMod++;
        if (!sItem) {
            removeIdFromMap(key);
            return getNextUpdatedItem();
        }
        else {
            // Updated item - send original id from before upgrade
            std::wstring key = sItem->getKey();
            if (isNewIdInMap(key)){
                sItem->setKey(getOldIdFromNewId(key).c_str());
            }
        }
    }
    return sItem;
}

/**
 * It's called in case of two-way sync.
 * @return the next SyncItem of modItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getNextUpdatedItem() {    
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    ClientItem* cItem = NULL;
    SyncItem*   sItem = NULL;

    if ( (modItems.size() > 0) && (iMod != modItems.end()) ) {

        std::wstring key = *iMod;

        try {
            cItem = outlook->getItemFromID(*iMod, getName());
            if (cItem && cItem->isReadOnly()) {
                sItem = NULL;
            } else {
                sItem = convertToSyncItem(cItem, config->getType(), defaultFolderPath);
                LOG.info(INFO_GET_UPDATED_ITEM, getName(), getSafeItemName(cItem).c_str());
            }
        }
        catch (ClientException* e) {
            manageClientException(e);
            manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET_MOD, getSafeItemName(cItem).c_str(), getName());
            sItem = NULL;
        }
        iMod++;
        if (!sItem) {
            removeIdFromMap(key);
            return getNextUpdatedItem();
        }
        else {
            // Updated item - send original id from before upgrade
            std::wstring key = sItem->getKey();
            if (isNewIdInMap(key)){
                sItem->setKey(getOldIdFromNewId(key).c_str());
            }
        }
    }
    return sItem;           
}

/**
 * It's called in case of two-way sync.
 * @return the first SyncItem of delItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getFirstDeletedItem() {
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }
    SyncItem* sItem = NULL;

    if (delItems.size() > 0) {
        iDel = delItems.begin();
        sItem = new SyncItem((*iDel).c_str());
        removeIdFromMap(*iDel);
        sItem->setDataSize(0);
        iDel++;
    }
    return sItem;
}

/**
 * It's called in case of two-way sync.
 * @return the next SyncItem of delItems list.
 * @note   returns a new allocated SyncItem, deleted internally by API.
 */
SyncItem* WindowsSyncSource::getNextDeletedItem() {    
    checkAbortedSync();

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }
    SyncItem* sItem = NULL;

    if ( (delItems.size() > 0) && (iDel != delItems.end()) ) {
        removeIdFromMap(*iDel);

        sItem = new SyncItem((*iDel).c_str());
        sItem->setDataSize(0);
        iDel++;
    }
    return sItem;   
}



int WindowsSyncSource::removeAllItems() {
    checkAbortedSync();

    // For appointments: reset the 'appointment_modified' file,
    // ready to append items for this sync session.
    if (!wcscmp(getName(), APPOINTMENT)) {
        resetDataFile(APPOINTMENT_FORCED_MODIFIED);
    }

    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return NULL;
    }

    
    //
    // Remove internally ALL items: totalItems = allItems + filteredItems.
    // ---------------------------- 
    itemKeyList totalItems = allItems;
    if (DLLCustomization::removeFilteredDataOnCleanup) {
        totalItems.sort();
        filteredItems.sort();
        totalItems.merge(filteredItems);    // lists MUST be sorted!
    }
    itemKeyIterator iTotal = totalItems.begin();

    if (totalItems.size() > 0) {
        LOG.info(INFO_REMOVING_ALL_ITEMS, getName(), totalItems.size());
        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_DELETE_CLIENT_ITEMS);

        wstring savedSubject = EMPTY_WSTRING;
        ClientItem* cItem = NULL;
        
        int i=1;
        iTotal = totalItems.begin();
        while (iTotal != totalItems.end()) {
            // Check aborted once every 50...
            if ((i % 50) == 0) checkAbortedSync();

            try {
                // Get each item
                cItem = outlook->getItemFromID(*iTotal, getName());

                if (!cItem) {
                    manageSourceErrorF(ERR_CODE_ITEM_GET, ERR_ITEM_GET, getSafeItemName(cItem).c_str(), getName());
                    iTotal++; 
                    continue;
                }
                savedSubject = getSafeItemName(cItem);

                // Delete each item
                if (cItem->deleteItem()) {
                    // item not deleted
                    report->addItem(CLIENT, COMMAND_DELETE, (*iTotal).c_str(), STC_COMMAND_FAILED, ERR_ITEM_DELETE_INTERNALLY);
                    manageSourceErrorF(ERR_CODE_DELETE, ERR_ITEM_DELETE, getName(), savedSubject.c_str(), cItem->getParentPath().c_str());
                }
                else {
                    // OK
                    report->addItem(CLIENT, COMMAND_DELETE, (*iTotal).c_str(), STC_OK, INFO_REMOVED_INTERNALLY);
                    LOG.debug(INFO_ITEM_DELETED, getName(), savedSubject.c_str());
                }
            }
            catch (ClientException* e) {
                manageClientException(e);
                manageSourceErrorF(ERR_CODE_DELETE, ERR_ITEM_DELETE, getName(), savedSubject.c_str(), cItem->getParentPath().c_str());
            }
            iTotal++;
            i++;
        }
    }

    idMap.clear();
    idMapReverse.clear();

    // Here, all items have been removed.
    return 0;
}




// **********************************************************************************************
// ************************* Methods to set items from Server to Client *************************
// **********************************************************************************************
/**
 * Adds the item from the server to the client.
 * Sets the SyncItem key (the GUID of item).
 * @return  code 201 if operation succesful, code 500 if errors.
 */
int WindowsSyncSource::addItem(SyncItem& item) {
    checkAbortedSync();
    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return STC_COMMAND_FAILED;
    }

	int ret = STC_COMMAND_FAILED;
    ClientFolder* folder = NULL;
    ClientItem*    cItem = NULL;
    bool isSifFormat = isSIF(item.getDataType());
    wstring path;

    // Get data content.
    char* charData = (char*)item.getData();
    if (!charData) {
        LOG.debug("No data content");
        return STC_COMMAND_FAILED;
    }
    charData[item.getDataSize()] = 0;
    WCHAR* data = toWideChar(charData);
    wstring dataString = data;                                      // TBD: Could we avoid these 2 copy?


    // Check if is an accepted mime type
    // For compatibility: accept all types (only warning).
    if ( !isAcceptedDataType(item.getDataType()) ) {
        //goto errorBadType;
        LOG.info(INFO_WRONG_MIME_TYPE, item.getDataType());
    }
    
    // Get folder path.
    extractFolder(dataString, item.getDataType(), path);


    //
    // Filter folders not allowed (not under selected folder)
    //
    if (!folderPathAllowed(path)) {
        goto errorFolderPath;
    }

    //
    // Get destination ClientFolder.
    //
    try {
        folder = outlook->getFolderFromPath(getName(), path);
        if (!folder) goto errorFolder;
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorFolder;
    }

    //
    // Add new ClientItem.
    //
    try {
        cItem = folder->addItem();
        if (!cItem) goto errorCreate;
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorCreate;
    }

    //
    // Fill all ClientItem's properties.
    //
    try {
        if (fillClientItem(dataString, cItem, getName(), item.getDataType())) {
            goto errorFillItem;
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorFillItem;
    }

    //
    // Filter incoming items: only if direction INPUT is enabled.
    //
    try {
        if (!filterClientItem(cItem, DateFilter::DIR_IN, COMMAND_ADD)) {
            goto filteredItem;
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorFillItem;
    }

    //
    // Save ClientItem.
    //
    try {
        if (cItem->saveItem()) {
            goto errorSave;  
        }
        lastAddedId = cItem->getID();
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorSave;
    }

    // Adjustment for Contacts: check if birthday/anniversary created
    // -> notify user on LOG.
    checkBirthdayAnniversary(cItem);   

    // Set item key = GUID.
    item.setKey(cItem->getID().c_str());

    itemAdded(cItem);

    LOG.info(INFO_ITEM_ADDED, getName(), getSafeItemName(cItem).c_str());
    ret = STC_ITEM_ADDED;
    goto finally;

//
// Error cases: notify error but continue.
//
errorFolder:
    manageSourceErrorF(ERR_CODE_FOLDER_OPEN, ERR_FOLDER_OPEN, getName(), path.c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorFolderPath:
    manageSourceErrorF(ERR_CODE_FOLDER_PATH, ERR_FOLDER_PATH, path.c_str(), getName());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorCreate:
    manageSourceErrorF(ERR_CODE_ITEM_CREATE, ERR_ITEM_CREATE, getName(), folder->getPath().c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorFillItem:
    manageSourceErrorF(ERR_CODE_ITEM_FILL, ERR_ITEM_FILL, getName(), getSafeItemName(cItem).c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

//errorBadType:
//    manageSourceErrorF(ERR_CODE_ITEM_BAD_TYPE, ERR_ITEM_BAD_TYPE, item.getDataType());
//    ret = UNSUPPORTED_MEDIA_TYPE;
//    goto finally;

errorSave:
    manageSourceErrorF(ERR_CODE_ITEM_SAVE, ERR_ITEM_SAVE, getName(), getSafeItemName(cItem).c_str(), folder->getPath().c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

filteredItem:
    // Don't set the source error, it can happen often.
    LOG.debug(ERR_INPUT_ITEM_FILTERED, getName(), getSafeItemName(cItem).c_str());
    ret = STC_PERMISSION_DENIED; //STC_COMMAND_FAILED;   // correct? or a fake STC_ITEM_ADDED?
    goto finally;

finally:
    if (data) {
        delete [] data;
    }
    return ret;
}


/**
 * Update the item from the server to the client.
 * Checks if destination folder is the same of the item's folder.
 * @note    If folder path is changed we need to move the item to new position (return code 201).
 *          Cannot move the item without changing the ID, so we have to create a new item
 *          and delete the old one. We MUST NOT update the allItems list (nor the allItemsPath), 
 *          so during next sync we will have 1 deleted item and 1 new item to send to server.
 * @return  code 200 if operation succesful, 
 *          code 201 if item updated to a new position, 
 *          code 500 if errors.
 */
int WindowsSyncSource::updateItem(SyncItem& item) {
    checkAbortedSync();
    
    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return STC_COMMAND_FAILED;
    }

	int ret = STC_COMMAND_FAILED;
    bool isSifFormat = isSIF(item.getDataType());
    ClientItem* cItem = NULL;


    // Get data content.
    char* charData = (char*)item.getData();
    if (!charData) {
        LOG.debug("No data content");
        return STC_COMMAND_FAILED;
    }
    charData[item.getDataSize()] = 0;
    WCHAR* data = toWideChar(charData);
    wstring dataString = data;                                      // TBD: Could we avoid these 2 copy?
    wstring path = EMPTY_WSTRING;
    wstring oldId;


    // Check if is an accepted mime type
    // For compatibility: accept all types (only warning).
    if ( !isAcceptedDataType(item.getDataType()) ) {
        //goto errorBadType;
        LOG.info(INFO_WRONG_MIME_TYPE, item.getDataType());
    }

    //
    // Get ClientItem to update.
    //

    if (isOldIdInMap(item.getKey())) {
        std::wstring originalId = item.getKey();
        item.setKey(getNewIdFromOldId(item.getKey()).c_str());
    }

    try {
        cItem = outlook->getItemFromID(item.getKey(), getName());
        if (!cItem) goto errorNotFound;
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorNotFound;
    }


    // Filter folders not allowed (not under selected folder)
    if (!folderPathAllowed(cItem->getParentPath())) {
        goto errorFolderPath;
    }


    //
    // Check if folder is the same.
    // If folder path is changed we need to move the item to new position (return code 201)
    //
    extractFolder(dataString, item.getDataType(), path);
    if (path != EMPTY_WSTRING && 
        path != cItem->getParentPath()) {
        LOG.debug("Path changed: need to add the item \"%ls\" as NEW and delete old one from Outlook.", getSafeItemName(cItem).c_str());
        
        // ADD new item
        ret = addItem(item);
        if (ret == STC_ITEM_ADDED) {
            // DELETE old item
            wstring savedSubject = EMPTY_WSTRING;
            try {
                savedSubject = getSafeItemName(cItem);
                oldId = cItem->getID();
                if (cItem->deleteItem()) {
                    goto errorDelete;
                }
            }
            catch (ClientException* e) {
                manageClientException(e);
                goto errorDelete;
            }
            LOG.info(INFO_ITEM_DELETED, getName(), savedSubject.c_str());

            // Update the path in the path list, and map the id
            for (iAll = allItems.begin(), iAllPaths = allItemsPaths.begin(); iAll != allItems.end(); iAll++, iAllPaths++) {
                if ((*iAll) == oldId) {
                    (*iAll) = lastAddedId;
                    (*iAllPaths) = path;
                    break;
                }
            }
            addToIdMap(oldId, lastAddedId);
        }
        else {
            ret = STC_COMMAND_FAILED;
        }
        goto finally;
    }


    //
    // Fill all ClientItem's properties.
    //
    try {
        if (fillClientItem(dataString, cItem, getName(), item.getDataType())) {
            goto errorFillItem;
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorFillItem;
    }

    //
    // Filter incoming items: only if direction INPUT is enabled.
    // (note: filter must be done AFTER we modified the item with updated data)
    if (!filterClientItem(cItem, DateFilter::DIR_IN, COMMAND_REPLACE)) {
        goto filteredItem;
    }

    //
    // Save ClientItem.
    //
    try {
        if (cItem->saveItem()) {
            goto errorSave;  
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorSave;
    }

    // Adjustment for Contacts: check if birthday/anniversary created
    // -> notify user on LOG.
    checkBirthdayAnniversary(cItem);


    LOG.info(INFO_ITEM_UPDATED, getName(), getSafeItemName(cItem).c_str());
    ret = STC_OK;
    goto finally;


//
// Error cases: notify error but continue.
//
errorNotFound:
    manageSourceErrorF(ERR_CODE_UPDATE_NOT_FOUND, ERR_ITEM_UPDATE_NOT_FOUND, getName(), item.getKey());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorFolderPath:
    manageSourceErrorF(ERR_CODE_FOLDER_PATH, ERR_FOLDER_PATH, cItem->getParentPath().c_str(), getName());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorFillItem:
    manageSourceErrorF(ERR_CODE_ITEM_FILL, ERR_ITEM_FILL, getName(), getSafeItemName(cItem).c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

//errorBadType:
//    manageSourceErrorF(ERR_CODE_ITEM_BAD_TYPE, ERR_ITEM_BAD_TYPE, item.getDataType());
//    ret = UNSUPPORTED_MEDIA_TYPE;
//    goto finally;

errorDelete:
    manageSourceErrorF(ERR_CODE_DELETE, ERR_ITEM_DELETE, getName(), getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

errorSave:
    manageSourceErrorF(ERR_CODE_ITEM_SAVE, ERR_ITEM_SAVE, getName(), getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

filteredItem:
    // Don't set the source error, it can happen often.
    LOG.debug(ERR_INPUT_ITEM_FILTERED, getName(), getSafeItemName(cItem).c_str());
    ret = STC_COMMAND_FAILED;   // correct? or a fake STC_OK?
    goto finally;

finally:
    if (data) {
        delete [] data;
    }
    return ret;
}


/**
 * Delete the SyncItem (empty, only key) from server to client.
 * @return  code 200 if operation succesful, 211 if item not found, 500 if errors occurred.
 */
int WindowsSyncSource::deleteItem(SyncItem& item) {
    checkAbortedSync();
    
    if (!report->checkState()) {
        LOG.debug(DBG_STATE_ERR_ITEM_IGNORED);
        return STC_COMMAND_FAILED;
    }

    int ret = STC_COMMAND_FAILED;
    ClientItem* cItem = NULL;
    wstring savedSubject;

    //
    // Get ClientItem to delete.
    //

    if (isOldIdInMap(item.getKey())) {
        std::wstring originalId = item.getKey();
        item.setKey(getNewIdFromOldId(item.getKey()).c_str());
        removeIdFromMap(item.getKey());
        removeIdFromMap(originalId);
    }

    try {
        cItem = outlook->getItemFromID(item.getKey(), getName());
        if (!cItem) goto errorNotFound;
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorNotFound;
    }

    //
    // Filter folders not allowed (not under selected folder)
    //
    if (!folderPathAllowed(cItem->getParentPath())) {
        goto errorFolderPath;
    }

    //
    // Filter incoming items: only if direction INPUT is enabled.
    // (sure we need this, for deleted items?)
    if (!filterClientItem(cItem, DateFilter::DIR_IN, COMMAND_DELETE)) {
        goto filteredItem;
    }

    //
    // Delete ClientItem.
    //
    try {
        savedSubject = getSafeItemName(cItem);
        if (cItem->deleteItem()) {
            goto errorDelete;
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        goto errorDelete;
    }

    LOG.info(INFO_ITEM_DELETED, getName(), savedSubject.c_str());
    ret = STC_OK;
    goto finally;


//
// Error cases: notify error but continue.
// 
errorNotFound:
    manageSourceErrorF(ERR_CODE_DELETE_NOT_FOUND, ERR_ITEM_DELETE_NOT_FOUND, getName(), item.getKey());
    ret = ITEM_NOT_DELETED;
    goto finally;

errorFolderPath:
    manageSourceErrorF(ERR_CODE_FOLDER_PATH, ERR_FOLDER_PATH, cItem->getParentPath().c_str(), getName());
    ret = ITEM_NOT_DELETED;
    goto finally;

errorDelete:
    manageSourceErrorF(ERR_CODE_DELETE, ERR_ITEM_DELETE, getName(), getSafeItemName(cItem).c_str(), cItem->getParentPath().c_str());
    ret = STC_COMMAND_FAILED;
    goto finally;

filteredItem:
    // Don't set the source error, it can happen often.
    LOG.debug(ERR_INPUT_ITEM_DEL_FILTERED, getName(), getSafeItemName(cItem).c_str());
    ret = ITEM_NOT_DELETED;   // correct?
    goto finally;

finally:
    return ret;
}



//
// ---------------------------------- Other Methods ----------------------------------
//

void WindowsSyncSource::setItemStatus(const WCHAR* key, int status) {
    //LOG.debug("key: %ls, status: %i", key, status);
}



void WindowsSyncSource::assign(WindowsSyncSource& s) {

    setSyncMode  (s.getSyncMode  ());
    setLastSync  (s.getLastSync  ());
    setNextSync  (s.getNextSync  ());
    setLastAnchor(s.getLastAnchor());
    setNextAnchor(s.getNextAnchor());
    setFilter    (s.getFilter    ());

    // Warning: pointers to the same external object!
    setReport(s.getReport());
}

//ArrayElement* WindowsSyncSource::clone() {
//
//    // Warning: config objects are linked to the same object owned by OutlookConfig.
//    WindowsSyncSource* s = new WindowsSyncSource( getName(), &(getConfig()) );
//
//    s->assign(*this);
//    return s;
//}



/**
 * Get the starting Outlook folder for this source. Starting folder is
 * retrieved from path (stored in config), if path not set the default folder
 * for this source will be returned.
 *
 * @return  the starting folder with items to sync
 */
ClientFolder* WindowsSyncSource::getStartFolder() {

    ClientFolder* folder = NULL;
    WCHAR* wp = toWideChar(getConfig().getFolderPath());
    wstring path = wp;

    // Replace "\\" with "%5C" which is not a valid sequence (skip 1st char).
    // This is done because "\" is the separator used to select the folder, so it's not good.
    // (then we'll replace the "%5C" with "\" inside ClientApplication::getFolderFromPath() )
    replaceAll(L"\\\\", L"%5C", path, 1);

    try {
        folder = outlook->getFolderFromPath(getName(), path);
    }
    catch (ClientException* e) {
        manageClientException(e);
        LOG.error(ERR_FOLDER_OPEN, getName(), path.c_str());
        if (wp) delete [] wp;
        return NULL;
    }

    if (path.compare(L"") != 0 && folder->getPath().compare(path.c_str()) != 0) {
        return NULL;
    }

    if (folder) {
        // Save current path used to configuration.
        char* p = toMultibyte(folder->getPath().c_str());
        getConfig().setFolderPath(p);
        if (p) delete [] p;
    }
    else {
        LOG.error(ERR_FOLDER_OPEN, getName(), path.c_str());
    }

    if (wp) delete [] wp;
    return folder;
}




/**
 * Get all items inside 'folder' and also all items from every subfolder 
 * (call to this method recursively). All items are pushed into 'listItems' list.
 * 
 * @param folder          the ClientFolder pointer to search items
 * @param listItems       the list<wstring> where pushing items
 * @param listItemsPaths  the list<wstring> where pushing items paths (optimization, used at the end of sync)
 */
void WindowsSyncSource::pushAllSubfolderItemsToList(ClientFolder* folder, itemKeyList& listItems, itemKeyList& listItemsPaths) {

    if (!folder) {
        return;
    }

    //
    // Push this folder's items.
    //
    pushAllItemsToList(folder, listItems, listItemsPaths);

    //
    // Push all subfolder's items.
    //
    ClientFolder* subFolder = NULL;
    int count = folder->getSubfoldersCount();
    std::vector<std::wstring> folders;
    for (int x = 0; x < count; x++) {
        std::wstring name = folder->getSubfolder(x)->getName();
        folders.push_back(name);
    }

    for (int x = 0; x < count; x++) {
        subFolder = folder->getSubfolderFromName(folders[x]);
        if(subFolder) {
            // Recursive call!
            pushAllSubfolderItemsToList(subFolder, listItems, listItemsPaths);
        }
    }
}




/**
 * Get all items inside 'folder' and push them (only keys) into 'listItems' list.
 * Sends messages to update statusbar on each item read.
 * Also normalize appointment exceptions: mod exceptions need to be
 * converted to new appointments (see doc on Recurrence Exceptions) and added to the 'listItems'.
 * 
 * @param folder          the ClientFolder pointer to search items
 * @param listItems       the list<wstring> where pushing items ID
 * @param listItemsPaths  the list<wstring> where pushing items paths (optimization, used at the end of sync)
 */
void WindowsSyncSource::pushAllItemsToList(ClientFolder* folder, itemKeyList& listItems, itemKeyList& listItemsPaths) {

    if (!folder) {
        return;
    }
    ClientItem* item = NULL;

    // MUST save the item number! (normalization of app exceptions can
    // lead to new items creations).
    int itemsCount = folder->getItemsCount();
    LOG.debug(DBG_READ_ALL_ITEMS, getName(), folder->getPath().c_str(), itemsCount);

    if (itemsCount == 0) {
        return;
    }
        if (LOG.getLevel() >= LOG_LEVEL_DEBUG) {
            LOG.debug("Reading item: \"%ls\"", getSafeItemName(item).c_str());
        }

    itemKeyList existingItems;

    item = folder->getFirstItem();
    // Item could be NULL if type not correct! -> ignore it.
    if (item != NULL) {
        existingItems.push_back(item->getID());
    }

    for (int i = 1; i < itemsCount; i++) {
        item = folder->getNextItem();
        // Item could be NULL if type not correct! -> ignore it.
        if (item != NULL && filterClientItem(item, DateFilter::DIR_OUT)) {
            existingItems.push_back(item->getID());
        } else {
            // We save the list of filteredItems ID, because we may use them later (see manageModifications())
            if (item) { 
                filteredItems.push_back(item->getID()); 
            }
        }
    }

    itemKeyList::iterator it;
    int count = 0;

    for (it = existingItems.begin(); it != existingItems.end(); it++) {
        item = outlook->getItemFromID(*it, getName());

        // SHOULD NEVER FAIL
        if (item) {
            // First normalize exceptions (for appointment).
            if (!wcscmp(getName(), APPOINTMENT)) {
                int current = listItems.size();
                if (normalizeExceptions(item, listItems, listItemsPaths))
                {
                    char temp[500];
                    char * subject = toMultibyte(item->getProperty(L"Subject").c_str());
                    sprintf(temp, "Unable to normalize appointment: %s.  Please check this event", subject);
                    throwClientException(temp,0,0,true);
                }
            }

            std::wstring modtime = item->getProperty(L"LastModificationTime");
            long modtimestamp = variantTimeToTimeStamp(_wtof(modtime.c_str()));
            std::wstring itemID = item->getID();
            std::wstring parentPath = item->getParentPath();
            cacheItem(itemID,modtimestamp,parentPath);

            listItems.push_back(item->getID());
            listItemsPaths.push_back(item->getParentPath());
            SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, (WPARAM)listItems.size(), (LPARAM)SBAR_CHECK_ALL_ITEMS);
        } else {
            throwClientException("Unable to access all items from outlook",0,0,true);
        }

        // Check aborted once every 5 (normalize exc may take more time)
        if ((count % 5) == 0) {
            checkAbortedSync();
        }

        count++;
    }
}


bool WindowsSyncSource::filterClientItem(ClientItem* item, 
                                         DateFilter::FilterDirection direction,
                                         const char* command)
{
    if (!item) return false;

    // So far, only 'date filter' for appointments
    if (item->getType() == APPOINTMENT) {
        ClientAppointment* cApp = (ClientAppointment*)item;
        if (!cApp) return false;                      

        // Apply the filter.
        // Filter only if desired direction is enabled.
        DateFilter& filter = getDateFilter();
        if (filter.getDirection() & direction) {
            if (direction == DateFilter::DIR_IN) {
                if (command && strcmp(command, COMMAND_DELETE) != 0) {
                    // Incoming items: if recurring, we MUST save the recPattern
                    // otherwise the recPattern will result non-updated.
                    ClientRecurrence* cRec = cApp->getRecPattern();
                    // Forcing a read after the save to have the values inside
                    // cRec updated to the item just sent from the server
                    if (cRec){
                        cRec->save();
                        cRec->refresh();
                    }
                }
                // Incoming items: the COMPtr for recPattern is not yet valid (it will
                // be ok once item saved) so we check the ClientItem members (strings).

                return filter.execute(item);
            }
            else {
                // Outgoing items: for better performance check directly the COMPtr.
                _AppointmentItemPtr& app = cApp->getCOMPtr();
                return filter.execute(app);
            }
        }
    }

    return true;
}

void WindowsSyncSource::updateFilters()
{
    // So far, only 'date filter' for appointments
    if (!wcscmp(getName(),APPOINTMENT)) {
        DateFilter& filter = getDateFilter();
        if (filter.isEnabled()) {
            filter.updateNow();
        }
    }
}

/**
 * Updates the list of all items and paths by removing and adding items.
 * The lists of items and paths must be the same length, otherwise this
 * would jeopardize the integrity of the list of all items.
 */
void WindowsSyncSource::updateAllItemsLists(itemKeyList &itemsToDelete, itemKeyList &itemPathsToDelete,
    itemKeyList &itemsToAdd, itemKeyList &itemPathsToAdd) {

    if (itemsToDelete.size() != itemPathsToDelete.size()) {
        return;
    }

    if (itemsToAdd.size() != itemPathsToAdd.size()) {
        return;
    }

    itemKeyIterator iDel;
    itemKeyIterator iAdd;

    itemKeyIterator iPathDel;
    itemKeyIterator iPathAdd;

    iDel = itemsToDelete.begin();
    iPathDel = itemPathsToDelete.begin();
    while (iDel != itemsToDelete.end()) {

        iAll = allItems.begin();
        iAllPaths = allItemsPaths.begin();
        while (iAll != allItems.end()) {
            if (*iAll == *iDel) {
                iAll = allItems.erase(iAll);
                iAllPaths = allItemsPaths.erase(iAllPaths);
            }
            if (iAll != allItems.end()) {
                iAll++;
                iAllPaths++;
            }
        }

        iDel++;
        iPathDel++;
    }

    iAdd = itemsToAdd.begin();
    iPathAdd = itemPathsToAdd.begin();
    while (iAdd != itemsToAdd.end()) {
        allItems.push_back(*iAdd);
        allItemsPaths.push_back(*iPathAdd);
        iAdd++;
        iPathAdd++;
    }
}



/**
 * Creates the NEW/MOD/DEL items lists from last successfull sync.
 * - read old items list from filesystem -> fill oldItems list
 * - fill NEW/MOD/DEL lists comparing allItems and oldItems
 * - add forced MOD items from previous sync (for appointment exceptions)
 *
 * @return  0 if no errors.
 *          In case of error, all items will be set as modified items.
 *          -1 if there was a large number of deletes
 */
int WindowsSyncSource::manageModificationsFromLastSync() {

    int ret = 0;
    ClientItem* cItem = NULL;
    wstring fileContent;
    wstring lastSyncTime = EMPTY_WSTRING;
    itemKeyList oldItemsFolders;                // List of old items folder paths.
    itemKeyIterator iFol;

    long lastModTimeStamp  = 0;
    double lastModVarTime  = 0;
    std::wstring parentPath;

    std::wstring idMapFile;

    itemKeyList allItemsDeletes;
    itemKeyList allItemsAdds;

    itemKeyList allItemPathsDeletes;
    itemKeyList allItemPathsAdds;

    // Refresh status bar on UI.
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_CHECK_MOD_ITEMS);

    //
    // Get oldItems file content.
    //
    WCHAR* oldItemsPath = readDataPath(getName());
    if (!oldItemsPath) {
        // Error getting file path.
        manageSourceError(ERR_CODE_OLD_ITEMS_PATH, getLastErrorMsg());
        goto error;
    }
    fileContent = readFromFile(oldItemsPath);
    if (!fileContent.length()) {
        // File should never be empty!
        goto error;
    }

    idMapFile = this->getIdMapFile();
    if (idMapFile.compare(L"") == 0) {
        // Error getting file path.
        manageSourceError(ERR_CODE_ID_MAP_PATH, getLastErrorMsg());
        goto error;
    }

    constructIdMaps(idMapFile);

    //
    // Parse XML file with old items keys -> use 'delItems' for list of old items.
    //
    oldItemsFolders.clear();
    parseOldItems(fileContent, delItems, oldItemsFolders);
    if (delItems.size() == 0) {
        // all items are new.
        newItems = allItems;
        modItems.clear();
        ret = 0;
        goto finally;
    }

    // Safe check.
    if (delItems.size() != oldItemsFolders.size()) {
        // This should never happen...
        LOG.error("Internal error: SS %ls, delItems.size = %d, oldItemsFolders.size = %d", getName(), delItems.size(), oldItemsFolders.size());
        newItems = allItems;
        modItems.clear();
        ret = 0;
        goto finally;
    }

    // Last sync timestamp to compare each item's mod time (read from oldItems file).
    if (getElementContent(fileContent, L"LastSyncTime", lastSyncTime)) {
        setErrorF(getLastErrorCode(), ERR_SOURCE_LASTSYNCTIME_NOT_FOUND, oldItemsPath);
        LOG.error(getLastErrorMsg());
        goto error;
    }
    long lastSyncTimeStamp = _wtoi(lastSyncTime.c_str());

    //
    // Fill NEW/MOD/DEL lists of ClientItem keys.
    //
    iAll = allItems.begin();
    bool found;
    int i=1;
    while (iAll != allItems.end()) {
        found = false;

        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, (WPARAM)i, (LPARAM)SBAR_CHECK_MOD_ITEMS2);
        // Check aborted once every 100...
        if ((i % 100) == 0) {
            checkAbortedSync();
        }

        iDel = delItems.begin();
        iFol = oldItemsFolders.begin();
        while (iDel != delItems.end()) {

            std::wstring itemId = *iAll;

            // The item is not deleted - the id has changed (moved)
            if (isNewIdInMap(itemId) && getOldIdFromNewId(itemId) == *iDel){
                itemId = *iDel;
            }
            
            // Found!
            if (itemId == *iDel) {
                bool clearDelete = true;
                found = true;
                cItem = NULL;
                if (getItemDetails(*iAll, parentPath, lastModTimeStamp, cItem)) {

                    // 1. Check last modification time of the item.
                    // Note: saving items Outlook can introduce a delay of some seconds!
                    if (lastModTimeStamp > lastSyncTimeStamp + TIMESTAMP_DELAY) {
                        // *** Modified item ***
                        modItems.push_back(*iAll);
                    }

                    // 2. New since v.6.0.9: check if folder path is the same (ignore if foder info not exists)
                    else if ((*iFol) != EMPTY_WSTRING) {
                        if (parentPath != (*iFol)) {
                            // *** Modified item: the folder name has changed ;) ***

                            // We send items that were modified by a folder change
                            // by copying the item, and deleting the original,
                            // so that moves between folders work better on some servers
                            if (DLLCustomization::sendMovedAsNew) {
                                if (!cItem) {
                                    cItem = outlook->getItemFromID(*iAll, getName());
                                }
                                if (cItem && !cItem->isReadOnly()) {
                                    LOG.info("Item with write permissions moved between folders - copying");
                                    ClientItem * newItem = cItem->copyItem();
                                    newItem->clearUserProperties();
                                    newItem->saveItem();
                                    newItems.push_back(newItem->getID());
                                    allItemsDeletes.push_back(cItem->getID());
                                    allItemPathsDeletes.push_back(cItem->getParentPath());
                                    allItemsAdds.push_back(newItem->getID());
                                    allItemPathsAdds.push_back(newItem->getParentPath());
                                    cItem->deleteItem();
                                    clearDelete = false;
                                }
                            } else {
                                modItems.push_back(*iAll);
                            }
                        }
                    }
                }

                // Dont clear the deleted item for folder moves
                if (clearDelete) {
                    delItems.erase(iDel);         // Remove the old item found from delItems list! (so won't be checked again)
                    oldItemsFolders.erase(iFol);
                    break;                        // Go directly to next allItems key (iDel is invalid)
                }
            } 
            iDel++;
            iFol++;
        }

        if (!found) {
            // We didn't find it in oldItems -> *** New item ***
            newItems.push_back(*iAll);

            // New item - never been synced, entry in idMap is useless - clear it
            removeNewIdFromMap(*iAll);
        }
        iAll ++;
        i++;
    }

    //
    // 'delItems' still in the list are items not found in allItems.
    //
    // Some of these items could result 'deleted' because of a new filter, we don't
    // want to send the <Delete> for filtered out items, so we remove them.
    // For example, we don't send all the deletes the first time the user enables
    // the filtering on appointment dates..
    if (DLLCustomization::dontSendFilteredItemsAsDeleted) {
        iDel = delItems.begin();
        while (iDel != delItems.end()) {
            iFiltered = filteredItems.begin();
            bool found = false;

            while (iFiltered != filteredItems.end()) {
                if (*iDel == *iFiltered) {
                    // found: it's a filtered item. Returns the it of next item.
                    iDel = delItems.erase(iDel);
                    found = true;
                    break;
                }
                iFiltered++;
            }

            if (!found) { iDel++; }
        }
    }


    //
    // For appointments: add forced modified items to MOD list.
    // (to manage appointment exceptions, see doc on exceptions)
    //
    if (!wcscmp(getName(), APPOINTMENT)) {
        WCHAR* modFile = readDataPath(APPOINTMENT_FORCED_MODIFIED);
        if (modFile) {

            itemKeyList forcedItems;
            forcedItems.clear();

            itemKeyList forcedItemsFolders;
            forcedItemsFolders.clear();

            wstring modData = readFromFile(modFile);
            parseOldItems(modData, forcedItems, forcedItemsFolders);
            // MUST check for duplicates...
            addForcedItemsToModList(forcedItems);
            forcedItems.clear();

            delete [] modFile;
            modFile = NULL;
        }
    }

    // If more than half are deleted, return delete warning
    if (delItems.size() > 0 && ((double)allItems.size())/((double)delItems.size()) <= 2) {
        ret = -1;
    } else {
        ret = 0;
    }

    // Update the allItems lists
    updateAllItemsLists(allItemsDeletes, allItemPathsDeletes, allItemsAdds, allItemPathsAdds);

    goto finally;
error:
    // consider all items as modified.
    newItems.clear();
    modItems = allItems;
    delItems.clear();
    ret = 1;
    goto finally;

finally:
    if (oldItemsPath) {
        delete [] oldItemsPath;
        oldItemsPath = NULL;
    }
    checkAbortedSync();
    return ret;
}




/**
 * Scan the passed list of 'forced modified items' and add each element
 * to the 'modItems' list. Elements are added ONLY if not yet found
 * in one of the lists (NEW/MOD/DEL) - to avoid duplicates.
 *
 * @param forcedItems  list of forced items to scan
 * @return             the number of items added to modItems list
 */
int WindowsSyncSource::addForcedItemsToModList(itemKeyList& forcedItems) {

    if (!forcedItems.size()) {
        return 0;
    }

    itemKeyIterator iForced = forcedItems.begin();
    int i=0;
    while (iForced != forcedItems.end()) {
        bool found = false;

        // Check NEW items...
        iNew = newItems.begin();
        while (iNew != newItems.end()) {
            if (*iForced == *iNew) {
                found = true; break;
            }
            iNew ++;
        }
        if (found) {
            iForced ++; continue;
        }

        // Check MOD items...
        iMod = modItems.begin();
        while (iMod != modItems.end()) {
            if (*iForced == *iMod) {
                found = true; break;
            }
            iMod ++;
        }
        if (found) {
            iForced ++; continue;
        }

        // Check DEL items...
        iDel = delItems.begin();
        while (iDel != delItems.end()) {
            if (*iForced == *iDel) {
                found = true; break;
            }
            iDel ++;
        }
        if (found) {
            iForced ++; continue;
        }

        // If here, the forced item has not been found.
        // -> add to the modItems list
        modItems.push_back(*iForced);
        i++;
        iForced ++;
    }

    return i;
}



/**
 * Reset the specified data file: destroy any content and
 * add a "<itemType>" tag at the beginning of file.
 * The file path is "<APP_DATA>\Funambol\Outlook Client\<itemType>.db"
 *
 * @param itemType  the itemType
 * @return          0 if no errors
 */
int WindowsSyncSource::resetDataFile(const wstring& itemType) {

    int ret = 0;
    WCHAR* dataFile = readDataPath(itemType.c_str());
    if (dataFile) {
        wstring xml = L"<";
        xml += itemType;
        xml += L">\n";
        ret = writeToFile(xml, dataFile, L"w");       // Destroy content.

        delete [] dataFile;
        dataFile = NULL;
    }
    return ret;
}


/**
 * Close the specified data file: append a "</itemType>" tag 
 * at the end of the file.
 * The file path is "<APP_DATA>\Funambol\Outlook Client\<itemType>.db"
 *
 * @param itemType  the itemType
 * @return          0 if no errors
 */
int WindowsSyncSource::closeDataFile(const wstring& itemType) {

    int ret = 0;
    WCHAR* dataFile = readDataPath(itemType.c_str());
    if (dataFile) {
        wstring xml = L"</";
        xml += itemType;
        xml += L">\n";
        ret = writeToFile(xml, dataFile, L"a");       // Append content.

        delete [] dataFile;
        dataFile = NULL;
    }
    return ret;
}





/**
 * Format the list of all current items into a string, as an XML.
 *
 *    <contact>
 *    <LastSyncTime>1168862109</LastSyncTime>
 *    <Item>
 *       <ID>00013505600G4BGG494D009680</ID>
 *       <Folder>\\Personal Folder\Contacts\subf</Folder>
 *    </Item>
 *    <Item>
 *       <ID>00033505600G4BGG494D006620</ID>
 *       <Folder>\\Personal Folder\Contacts</Folder>
 *    </Item>
 *    ...
 *    </contact>
 *
 * 'ID' is the item's ID in Outlook.
 * 'Folder' is the item's folder path in Outlook.
 *
 * @return  the formatted XML string
 */
wstring WindowsSyncSource::createOldItems() {

    wstring data;
    ItemReport *itemDeleted, *itemAdded;
    bool deleted = false;

    // Refresh status bar on UI.
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_WRITE_OLD_ITEMS);

    int clientDeletedCount = report->getItemReportCount(CLIENT, COMMAND_DELETE);
    int clientAddedCount   = report->getItemReportCount(CLIENT, COMMAND_ADD);

    // This is the last successful synctime for this source.
    unsigned long lastSyncTime = (unsigned long)time(NULL);
    WCHAR lastTimestamp[12];
    wsprintf(lastTimestamp, L"%li", lastSyncTime);

    //
    // TBD: if clientDeletedCount > allItems.size()
    //      it's better to refresh allItems directly from Outlook?
    //

    // This is also not to leave file empty...
    data  = L"<";
    data += getName();
    data += L">\n";

    data += L"<LastSyncTime>";
    data += lastTimestamp;
    data += L"</LastSyncTime>\n";

    //
    // Cycle ALL items...
    //
    iAll = allItems.begin();
    iAllPaths = allItemsPaths.begin();
    int j=1;

    LOG.debug("Generating list for deleted/updated/unchanged items");

    while (iAll != allItems.end()) {

        // Check aborted once every 100...
        if ((j % 100) == 0) {
            checkAbortedSync();
        }

        // Search this ID inside deleted report
        deleted = false;
        for (int i=0; i < clientDeletedCount; i++) {
            itemDeleted = report->getItemReport(CLIENT, COMMAND_DELETE, i);
            if ( !wcscmp((*iAll).c_str(), itemDeleted->getId()) &&
                 !isErrorStatus(itemDeleted->getStatus()) ) {
                deleted = true;
                break;
            }
        }
        // Exclude items deleted successfully.
        if (!deleted) {
            data += L"<Item>\n";
            data += L"    <ID>";       data += *iAll;       data += L"</ID>\n";
            data += L"    <Folder>";   data += *iAllPaths;  data += L"</Folder>\n";
            data += L"</Item>\n";
        }
        iAll ++;
        iAllPaths ++;
        j++;
    }
    
    LOG.debug("Generating list for added items");

    // Plus items added successfully.
    for (int i=0; i < clientAddedCount; i++) {
        itemAdded = report->getItemReport(CLIENT, COMMAND_ADD, i);
        if ( !isErrorStatus(itemAdded->getStatus()) ) {
            std::wstring wid = itemAdded->getId();
            std::wstring folder;
            if (getItemDetails(wid, folder)) {
                data += L"<Item>\n";
                data += L"    <ID>";       data += wid;       data += L"</ID>\n";
                data += L"    <Folder>";   data += folder;    data += L"</Folder>\n";
                data += L"</Item>\n";
            }
        }
    }

    data += L"</";
    data += getName();
    data += L">\n";

    return data;
}

/**
 * Parse the list of all current items into a string, from an XML.
 *
 *    <contact>
 *    <LastSyncTime>1168862109</LastSyncTime>
 *    <Item>
 *       <ID>00013505600G4BGG494D009680</ID>
 *       <Folder>\\Personal Folder\Contacts\subf</Folder>
 *    </Item>
 *    <Item>
 *       <ID>00033505600G4BGG494D006620</ID>
 *       <Folder>\\Personal Folder\Contacts</Folder>
 *    </Item>
 *    ...
 *    </contact>
 *
 * and push all items' keys into 'listItems' list. Also push all items' folder
 * paths into 'listFolder' list. Each key of listItems corresponds to the folder element
 * of the same index.
 * It supports old format of db files (before v.6.0.9), like this:
 *
 *    <contact>
 *    <LastSyncTime>1168862109</LastSyncTime>
 *    <Item>00013505600G4BGG494D009680</Item>
 *    <Item>00033505600G4BGG494D006620</Item>
 *    ...
 *    </contact>
 *
 * @param  data         the input string to parse
 * @param  listItems    [IN-OUT] list of item keys, filled here
 * @param  listFolders  [IN-OUT] list of folder paths, filled here
 */
void WindowsSyncSource::parseOldItems(wstring& data, itemKeyList& listItems, itemKeyList& listFolders) {
    
    wstring::size_type pos, start, end;
    wstring item;

    pos = 0;
    while ( !getElementContent(data, L"Item", item, pos, start, end) ) {
        pos = end;
        if (!item.length()) {
            continue;
        }

        // Search for <ID>
        wstring ID;
        if (getElementContent(item, L"ID", ID)) {
            //
            // <ID> not found -> db old style -> folder info is not available.
            //
            listItems.push_back(item);
            listFolders.push_back(EMPTY_WSTRING);   // empty data will be ignored when checking modifications.
            continue;
        }
        if (!ID.length()) {
            continue;
        }

        // Search for <Folder>
        wstring folder;
        if (!getElementContent(item, L"Folder", folder)) {
            if (folder.length() > 0) {
                listItems.push_back(ID);
                listFolders.push_back(folder);
            }
        }
    }
}


/**
 * Returns true if path passed is allowed to sync items inside it.
 */
bool WindowsSyncSource::folderPathAllowed(const wstring& p) {
    
    bool ret = false;
    WCHAR* tmp = toWideChar(getConfig().getFolderPath());
    wstring startFolder = tmp;
    wstring path = p;
    const wstring delim = L"\\";
    wstring::size_type start, end;
    wstring pathNoRoot = path;
    
    // Always allow the default folder...
    if (p == startFolder || 
        p == EMPTY_WSTRING ||
        p == L"\\" ||
        p == L"/") {
        ret = true;
        goto finally;
    }

    //
    // Remove the root folder from startFolder (to allow different datastore names)
    // (e.g. "\\MyPIM\Contacts\folder1" -> "\\Personal Folders\Contacts\folder1")
    //
    start = path.find_first_not_of(delim);
    if (start != wstring::npos) {
        end = path.find_first_of(delim, start);
        if (end == wstring::npos) {
            end = path.length();
        }
        pathNoRoot = path.substr(end, path.length());
        wstring rootFolderName = path.substr(start, end-start);
        ClientFolder* f = outlook->getRootFolderFromName(rootFolderName);

        // If folder doesn't exists -> try get the default personal folder...
        if (!f) {
            LOG.debug("Continue with default root folder.");
            f = outlook->getDefaultRootFolder();
            // If neither default root -> error
            if (!f) {
                manageSourceError(ERR_CODE_NO_ROOT_FOLDER, ERR_OUTLOOK_NO_ROOTFOLDER);
                return false;
            }
            // Substitute root folder with the default root.
            path = f->getPath();
            path += pathNoRoot;
        }
    }
    // Now can correspond exactly, after substitution.
    if (path == startFolder) {
        ret = true;
        goto finally;
    }


    // to avoid matching of names that begin with the same chars...
    startFolder += L"\\";
    path        += L"\\";

    // Allow if path is under the startFolder.
    if (winConfig.getUseSubfolders() == true) {
        if (path.find(startFolder, 0) != wstring::npos) {
            ret = true;
        }
        else {
            ret = false;
        }
    }
    else {
        ret = false;
    }

finally:
    if (tmp) {
        delete [] tmp;
    }
    return ret;
}


/**
 * Verifies if birthday/anniversary were created by Outlook (only for contacts save).
 * If created, they are immediately deleted.
 */
void WindowsSyncSource::checkBirthdayAnniversary(ClientItem* cItem) {

    if (!wcscmp(getName(), CONTACT)) {
        try {
            ClientContact* c = (ClientContact*)cItem;
            if (c) {
                if (c->createdAnniversaryEvent()) {
                    if (deleteAppointment(c, L"Anniversary") == 0) {
                        LOG.debug(DBG_ANNIVERSARY_DELETED, getSafeItemName(cItem).c_str());
                    }
                }
                if (c->createdBirthdayEvent()) {
                    if (deleteAppointment(c, L"Birthday") == 0) {
                        LOG.debug(DBG_BIRTHDAY_DELETED, getSafeItemName(cItem).c_str());
                    }
                }
            }
        }
        catch (ClientException* e) {
            manageClientException(e);
            LOG.error(ERR_EVENTS_CREATED);
        }
    }
}


/**
 * Deletes an appointement event (birthday or anniversary) automatically created
 * by Outlook while saving item 'cItem'.
 * The item to delete is searched inside default Outlook folder, where:
 * - 'Start' corresponds exactly to the date of birthday/anniversary
 * - 'Subject' contains the 'Subject' of contact cItem (usually is e.g. "Mike Portnoy's birthday").
 *   [Note: the subject does not contain exactly the FullName nor the FileAs nor FirstName...]
 * - 'Creation time' is almost now (max error = 2 sec)
 * 
 * @param cItem        : the ClientContact that was saved
 * @param propertyName : "Anniversary" or "Birthday"
 * @return               0 if found and correctly deleted - 1 otherwise
 */
int WindowsSyncSource::deleteAppointment(ClientItem* cItem, const wstring& propertyName) {

    wstring::size_type pos = wstring::npos;
    ClientFolder* folder = NULL;

    try {
        outlook = ClientApplication::getInstance(true);
    }
    catch (ClientException* e) {
        manageClientException(e);
        return 1;
    }

    if (outlook) {
        folder = outlook->getDefaultFolder(APPOINTMENT);
    }
    if (!folder || !folder->getItemsCount()) {
        return 1;
    }

    wstring subject  = cItem->getProperty(L"Subject");
    long now = (long)time(NULL);

    // It's just created, so should be the last one ;)
    ClientItem* newApp = folder->getLastItem();
    if (!newApp)
        return 1;

    if (newApp->getProperty(L"Start") == cItem->getProperty(propertyName)) {        // 1. start = birthday/anniversary date
        if (subject.size() > 0) {
            pos = newApp->getProperty(L"Subject").find(subject);                    // 2. subject contains contact's subject (not empty)
        }
        if (pos != wstring::npos) {
            DATE creationTime = ((ClientAppointment*)newApp)->getCreationTime();
            long creationTStamp = variantTimeToTimeStamp(creationTime);
            if (now - creationTStamp <= 2) {                                        // 3. creation time is almost now
                return (newApp->deleteItem());
            }
        }
    }

    // Hmm, strange... let's check also all the others.
    while (folder->getItemsIndex() > 0) {
        pos = wstring::npos;
        newApp = folder->getPreviousItem();
        if (!newApp) return 1;
        if (newApp->getProperty(L"Start") == cItem->getProperty(propertyName)) {
            if (subject.size() > 0) {
                pos = newApp->getProperty(L"Subject").find(subject);
            }
            if (pos != wstring::npos) {
                DATE creationTime = ((ClientAppointment*)newApp)->getCreationTime();
                long creationTStamp = variantTimeToTimeStamp(creationTime);
                if (now - creationTStamp <= 2) {
                    return (newApp->deleteItem());
                }
            }
        }
    }

    return 1;
}



/**
 * Extract 'Folder' path searching inside string 'dataString' (SIF or vCard/vCal).
 * Path retrieved is written into 'path' variable (empty string if 'Folder' not found).
 *
 * @param dataString   the input string to search 'Folder' info
 * @param dataType     the mime data type (SIF/vCard)
 * @param path         [OUTPUT] the folder path exctracted
 */
void WindowsSyncSource::extractFolder(const wstring dataString, const wstring dataType, wstring& path) {

    bool isSifFormat = isSIF(dataType);
    ClientFolder* folder = NULL;

    // Get folder path.
    if (isSifFormat) {
        getElementContent(dataString, L"Folder", path, 0);
        replaceAll(L"&lt;",  L"<", path);
        replaceAll(L"&gt;",  L">", path);
        replaceAll(L"&amp;", L"&", path);
    }
    else {
        // creating the WinItem just to take the x-funambol-folder in the proper way without
        // duplicate code
        wstring propertyValue;
        WCHAR* fields[] = {{L"Folder"}};
        // Internally switch to the correct WinObject and
        // fill it (parse data string + fill propertyMap).
        WinItem* winItem = createWinItem(isSifFormat, getName(), dataString, (const WCHAR**)fields);
        bool res = winItem->getProperty(L"Folder", propertyValue);
        if (res) {
            path = propertyValue;
            delete winItem;
        } else {
            LOG.debug("extractFolder method: failed to get the folder. use the default");
        }
        // vCard/vCalendar: parse the string.
        //path = getVPropertyValue(dataString, L"X-FUNAMBOL-FOLDER");
    }

    if (path != EMPTY_WSTRING) {
        // Replace "DEFAULT_FOLDER" with "\\Personal Folders\Contacts"
        // So the default folder is preserved even if path is different
        if (path.find(DEFAULT_FOLDER, 0) == 0) {
            path.replace(0, wcslen(DEFAULT_FOLDER), this->getStartFolder()->getPath());
        } else {
            if (path.find_first_of(L"\\",0) != 0)
                path = L"\\" + path;
            path = this->getStartFolder()->getPath() + path;
        }
    }

    // If path not specified, use the selected folder from config.
    // If selected folder not specified either, use default folder.
    if (path == EMPTY_WSTRING || path == L"/" || path == L"\\") {
        WCHAR* tmp = toWideChar(winConfig.getFolderPath());
        path = tmp;
        delete [] tmp;
    }
}


/**
 * Common actions to do when an error occurs inside SyncSource. 
 * Updates the internal report.
 * Doesn't set the Source state, as generally we want to continue the sync.
 * (source state is set only after too many errors on this source)
 */
void WindowsSyncSource::manageSourceError(const int errorCode, const char* errorMsg) {

    // Set the global errors (message and code)? 
    // No: we want only the source error, not a global sync error.
    //setError(errorCode, errorMsg);
    if (errorMsg == NULL) {
        LOG.error("manageSourceError: the errorMsg is NULL. Set to empty...");
        errorMsg = "";

    }
    report->setLastErrorCode(errorCode);
    report->setLastErrorMsg (errorMsg);
    
    //
    // Error codes for WindowsSyncSource:
    // Codes  < 100 = "soft" errors (error state not required)
    // Codes >= 100 = bad errors -> after 10 bad errors 'state' = SOURCE_ERROR
    //
    if (errorCode < 100) {
        LOG.debug(errorMsg);
    }
    else {
        LOG.error(errorMsg);
        numErrors ++;
        if (numErrors >= MAX_SOURCE_ERRORS) {
            // Don't bother me any longer...
            LOG.info(ERR_SOURCE_TOO_MANY_ERRORS, numErrors, getName());
            report->setState(SOURCE_ERROR);
        }
    }
}


void WindowsSyncSource::manageSourceErrorF(const int errorCode, const char *msgFormat, ...) {

    // Print the msg to a StringBuffer
    StringBuffer msg;
    va_list argList;
    va_start(argList, msgFormat);
    msg.vsprintf(msgFormat, argList);
    va_end(argList);

    // Set the source errors (message and code).
    manageSourceError(errorCode, msg.c_str());
}

void WindowsSyncSource::cacheItem(std::wstring itemID, long lastModified, std::wstring parentPath)
{
    cache[itemID] = CacheData(lastModified, parentPath);
}

void WindowsSyncSource::clearCache() {
    cache.clear();
}

bool WindowsSyncSource::getItemDetails(const std::wstring & itemID, std::wstring & parentPath) {
    long temp;
    return getItemDetails(itemID, parentPath, temp);
}

bool WindowsSyncSource::getItemDetails(const std::wstring & itemID, std::wstring & parentPath, long & lastModified) {
    ClientItem * cItem = NULL;
    return getItemDetails(itemID, parentPath, lastModified, cItem);
}

bool WindowsSyncSource::getItemDetails(const std::wstring & itemID, std::wstring & parentPath, long & lastModified, ClientItem * & cItem) {
    bool result = true;
    if (!getItemDetailsFromCache(itemID, parentPath, lastModified)) {
        cItem = outlook->getItemFromID(itemID, getName());

        if (cItem) {// Cache failed, get data
            double lastModVarTime = _wtof(cItem->getProperty(L"LastModificationTime").c_str());
            lastModified = variantTimeToTimeStamp(lastModVarTime);
            parentPath = cItem->getParentPath();
        } else {
            result = false;
        }
    }

    return result;
}

bool WindowsSyncSource::getItemDetailsFromCache(const std::wstring & itemID, std::wstring & parentPath, long & lastModified) {
    std::map<std::wstring, WindowsSyncSource::CacheData>::iterator it;
    it = cache.find(itemID);

    if (it == cache.end())
        return false;

    lastModified = (it->second).lastModified;
    parentPath = (it->second).parentPath;
    return true;
}

void WindowsSyncSource::itemAdded(ClientItem * item) {
    std::wstring itemID = item->getID();
    std::wstring parentPath = item->getParentPath();
            
    addedItems[itemID] = CacheData(0, parentPath);
}

// move all items (and subfolder items) into Shared/
int WindowsSyncSource::upgradeCalendarFolders(bool fixMyCalendar) {

    OutlookConfig * config = OutlookConfig::getInstance();
    LOG.setLevel(LOG_LEVEL_DEBUG);
    LOG.setLogPath(config->getLogDir());
    LOG.setLogName(OL_PLUGIN_LOG_NAME);

    LOG.info("Upgrading calendar folders");

    if (wcscmp(getName(), APPOINTMENT) != 0) {
        LOG.info("Not appt source");
        return -1;
    }

    LOG.info("Is appointments source");

    forceOpenOutlook = true;
    LOG.info("About to set report");
    setReport(new SyncSourceReport(APPOINTMENT_));
    LOG.info("Just set report");
    if (beginSync() != 0) {
        LOG.info("beginSync failed");
        return 1;
    }
    forceOpenOutlook = false;

    LOG.info("beginSync successful");

    std::map<std::wstring, std::wstring> folderChanges;

    bool fail = false;

    std::wstring mapFile = getIdMapFile();
    LOG.info("Got ID map file");
    if (mapFile.length() == 0) {
        LOG.info("Map file path not available");
        return 2;
    }
    
    constructIdMaps(mapFile);

    LOG.info("Id map read");

    // traverse all the items
    iAll = allItems.begin();
    iAllPaths = allItemsPaths.begin();
    itemKeyList uniqueItemPaths(allItemsPaths);

    LOG.info("Moving items");

    int count = 0;

    while (iAll != allItems.end()) {

        count++;
        if (count % 100 == 0) { 
            writeIdMap(idMap);
        }

        if (!fixMyCalendar) {
            if (!isNewIdInMap(*iAll)) {

                LOG.info("Moving item %ls", iAll->c_str());

                // check that the item is not already in Calendar/Shared/
                // the item is allowed to be in Calendar/Subfolder/Shared, so
                // check that, too
                ClientAppointment * cItem;
                try {
                    cItem = (ClientAppointment*)outlook->getItemFromID(*iAll, getName());
                } catch (...) {
                    fail = true;
                    break;
                }

                wstring newPath = L"";

                LOG.info("Calculating new folder name");

                if ((*iAllPaths) != defaultFolderPath) {
                    int sharedFolderPosition = (*iAllPaths).find(L"\\Shared\\");
                    if (sharedFolderPosition == string::npos ||
                        (size_t)sharedFolderPosition > (defaultFolderPath.size() + 1)) {
                            wstring parentPath = cItem->getParentPath();
                            wstring tempPath = parentPath.substr(defaultFolderPath.size());
                            if (tempPath.find(L" - ") == wstring::npos) {
                                newPath = defaultFolderPath + L"\\Shared" + tempPath + L" - My Calendar";
                            } else {
                                newPath = defaultFolderPath + L"\\Shared" + tempPath;
                            }
                    }

                    LOG.info("New folder: %ls", newPath.c_str());

                    folderChanges[(*iAllPaths)] = newPath;

                    std::wstring originalId;
                    std::wstring newId;
                    try {
                        LOG.info("Performing move");
                        ClientFolder* dest = outlook->getFolderFromPath(getName(), newPath);

                        // Move the item
                        originalId = cItem->getID();
                        cItem->moveItem(dest);
                        newId = cItem->getID();

                        idMap[newId] = originalId;

                        LOG.info("Move successful");
                    } catch (...) {
                        LOG.info("Move failed");
                        fail = true;
                        break;
                    }

                }

                *iAllPaths = newPath;

            } else {
                LOG.info("Item already been moved %ls", iAll->c_str());
            }
        
        } else {
            LOG.info("Checking to see if we should move item back: %ls", iAll->c_str());

            std::wstring badFolderPath = defaultFolderPath + L"\\My Calendar";

            ClientAppointment * cItem;
            try {
                cItem = (ClientAppointment*)outlook->getItemFromID(*iAll, getName());
            } catch (...) {
                fail = true;
                break;
            }

            std::wstring oldOriginalId;
            if (isNewIdInMap(*iAll)) {
                oldOriginalId = getOldIdFromNewId(*iAll);
            } else {
                oldOriginalId = *iAll;
            }

            if (cItem->getParentPath().compare(badFolderPath) == 0) {

                std::wstring newId;
                std::wstring newPath = defaultFolderPath;

                try {
                    LOG.info("Performing move");
                    ClientFolder* dest = outlook->getFolderFromPath(getName(), newPath);

                    // Move the item
                    cItem->moveItem(dest);
                    newId = cItem->getID();

                    removeIdFromMap(oldOriginalId);
                    idMap[newId] = oldOriginalId;

                    LOG.info("Move successful");
                } catch (...) {
                    LOG.info("Move failed");
                    fail = true;
                    break;
                }

            }

        }

        iAll++;
        iAllPaths++;
    }

    int success = this->writeIdMap(idMap);

    if (success != 0) {
        LOG.error("Unable to write id map");
        return 3;
    }

    if (!fail) {
        uniqueItemPaths.unique();
        itemKeyIterator iUniquePaths = uniqueItemPaths.begin();
        while (iUniquePaths != uniqueItemPaths.end()) {
            if ((*iUniquePaths) != defaultFolderPath) {
                int sharedFolderPosition = (*iUniquePaths).find(L"\\Shared\\");
                if (sharedFolderPosition == string::npos ||
                    (size_t)sharedFolderPosition > (defaultFolderPath.size() + 1)) {
                    ClientFolder* itemFolder = NULL;
                    try { 
                        itemFolder = outlook->getFolderFromPath(getName(), (*iUniquePaths));
                    } catch (...) {
                        LOG.info("Unable to get folder %ls", (*iUniquePaths).c_str());
                    }
                    if (itemFolder) {
                        LOG.info("Checking if folder is empty for delete: %ls", (*iUniquePaths).c_str());

                        int itemCount = 1;
                        try {
                            itemCount = itemFolder->getItemsCount();
                            LOG.info("Folder has %d items", itemCount);
                        } catch (...) {
                            LOG.error("Item count unavailable, defaulting to 1");
                        }
                        
                        if (itemCount == 0) {
                            LOG.info("Deleting folder");
                            try {
                                itemFolder->deleteFolder();
                            } catch (...) {
                                LOG.info("Delete failed");
                            }
                        } else {
                            LOG.info("Not deleting folder");
                        }
                    }
                }
            }
            iUniquePaths++;
        }
    }

    WCHAR* oldItemsPath = readDataPath(getName());
    if (!oldItemsPath) {
        LOG.error("Unable to find items database");
        manageSourceError(ERR_CODE_OLD_ITEMS_PATH, getLastErrorMsg());
        return 4;
    }

    std::wstring fileContent = readFromFile(oldItemsPath);

    std::map<std::wstring, std::wstring>::iterator iter;
    for (iter = folderChanges.begin(); iter != folderChanges.end(); iter++) {
        std::wstring originalContent = L"<Folder>" + iter->first + L"</Folder>";
        std::wstring newContent = L"<Folder>" + iter->second + L"</Folder>";
        replaceAll(originalContent, newContent, fileContent);
    }

    if (writeToFile(fileContent, oldItemsPath) != 0) {
        LOG.error("Unable to write items database");
        return 5;
    }

    if (fail) {
        return -2;
    }

    LOG.info("Folder upgrade complete");

    return 0;
}

std::wstring WindowsSyncSource::getIdMapFile() {
    std::wstring idMapName = getName();
    idMapName += L"_ids";
    WCHAR * idMapFile = readDataPath(idMapName.c_str());
    std::wstring result = L"";
    if (!idMapFile) {
        // Error getting file path.
        manageSourceError(ERR_CODE_OLD_ITEMS_PATH, getLastErrorMsg());
    } else {
        result = idMapFile;
        delete idMapFile;
    }
    return result;
}


std::map<std::wstring, std::wstring> WindowsSyncSource::readIdMap(const std::wstring & idMapFile) {
    
    std::map<std::wstring, std::wstring> tempIdMap;

    std::wstring idMapContent = readFromFile(idMapFile);
    if (!idMapContent.length()) {
        return tempIdMap;
    }

    std::wstring itemTag = L"Item";
    std::wstring originalIdTag = L"OriginalId";
    std::wstring newIdTag = L"NewId";

    std::wstring item, originalId, newId;
    std::wstring::size_type pos = 0, start, end;
    while (getElementContent(idMapContent, itemTag, item, pos, start, end) == 0) {
        pos = end;
        int foundOriginal = getElementContent(item, originalIdTag, originalId, 0);
        int foundNew = getElementContent(item, newIdTag, newId, 0);
        if (foundOriginal == 0 && foundNew == 0) {
            tempIdMap[newId] = originalId;
        }
        originalId = L"";
        newId = L"";
    }

    return tempIdMap;
}

int WindowsSyncSource::writeIdMap(const std::map<std::wstring, std::wstring> & tempIdMap) {


    std::wstring mapFile = getIdMapFile();
    if (mapFile.compare(L"") == 0) {
        return -1;
    }

    if (tempIdMap.size() == 0) {
        _wremove(mapFile.c_str());
        return 0;
    }

    std::wstring content = L"";
    std::map<std::wstring, std::wstring>::const_iterator it;
    for (it = tempIdMap.begin(); it != tempIdMap.end(); it++) {
        content += L"<Item>\n";
        content += L"\t<NewId>";
        content += it->first;
        content += L"</NewId>\n";
        content += L"\t<OriginalId>";
        content += it->second;
        content += L"</OriginalId>\n";
        content += L"</Item>\n";
    }

    int result = writeToFile(content, mapFile);

    return result;
}

void WindowsSyncSource::constructIdMaps(std::wstring idMapFile) {
    idMap.clear();
    idMapReverse.clear();

    idMap = readIdMap(idMapFile);
    std::map<std::wstring, std::wstring>::iterator it;
    for (it = idMap.begin(); it != idMap.end(); it++) {
        idMapReverse[it->second] = it->first;
    }
}

void WindowsSyncSource::addToIdMap(const std::wstring & oldId, const std::wstring & newId) {
    wstring actualOldId = oldId;
    // Its already been mapped once, remap it
    if (isNewIdInMap(oldId)) {
        actualOldId = getOldIdFromNewId(oldId);
        removeNewIdFromMap(oldId);
    }
    idMap[newId] = actualOldId;
    idMapReverse[actualOldId] = newId;
}

bool WindowsSyncSource::isNewIdInMap(const std::wstring & id) {
    return (idMap.count(id) != 0);
}

bool WindowsSyncSource::isOldIdInMap(const std::wstring & id) {
    return (idMapReverse.count(id) != 0);
}

std::wstring WindowsSyncSource::getOldIdFromNewId(const std::wstring & id) {
    return idMap[id];
}

std::wstring WindowsSyncSource::getNewIdFromOldId(const std::wstring & id) {
    return idMapReverse[id];
}

void WindowsSyncSource::removeOldIdFromMap(const std::wstring & id) {
    if (isOldIdInMap(id)) {
        std::wstring newId = getNewIdFromOldId(id);
        idMapReverse.erase(id);
        removeNewIdFromMap(newId);
    }
}

void WindowsSyncSource::removeNewIdFromMap(const std::wstring & id) {
    if (isNewIdInMap(id)) {
        std::wstring newId = getOldIdFromNewId(id);
        idMap.erase(id);
        removeOldIdFromMap(newId);
    }
}

void WindowsSyncSource::removeIdFromMap(const std::wstring & id) {
    removeOldIdFromMap(id);
    removeNewIdFromMap(id);
}
