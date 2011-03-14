/*
 * Funambol is a mobile platform developed by Funambol, Inc.
 * Copyright (C) 2003 - 2009 Funambol, Inc.
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

#ifndef INCL_WIN_FILE_SYNC_SOURCE
#define INCL_WIN_FILE_SYNC_SOURCE

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "base/fscapi.h"
#include "spds/constants.h"
#include "spds/SyncItem.h"
#include "spds/SyncMap.h"
#include "spds/SyncStatus.h"
#include "base/util/ItemContainer.h"
#include "spds/FileData.h"
#include "client/CacheSyncSource.h"
#include "client/MediaSyncSource.h"
#include "WindowsSyncSource.h"

BEGIN_NAMESPACE

/**
 * This class extends the MediaSyncSource class, to sync for files.
 * It just defines specific filterings for files:
 * - by size (< 50MB)
 * - by date (last modif date)
 * - by type (not used: all files are accepted)
 */
class WinFileSyncSource : public MediaSyncSource
{

public:

    /**
     * Constructor. if folderPath is empty, here we read and set the default
     * value, which is the shell folder for my documents 
     * "dir" is used by FileSyncSource during the sync process, so it's set to the same
     * value of "folderPath".
     */
    WinFileSyncSource(const WCHAR* name, WindowsSyncSourceConfig* sc);
    ~WinFileSyncSource() {};

    const WindowsSyncSourceConfig& getConfig() const;
    WindowsSyncSourceConfig& getConfig();

    /**
     * Sets the 'isSynced' flag, then calls the father method FileSyncSource::beginSync().
     */
    int beginSync();

    /**
     * Calls the father method FileSyncSource::endSync(), then sets the endTimestamp value to 'now'.
     */
    int endSync();


    //
    /// From FileSyncSource
    //
    Enumeration* getAllItemList();
    int insertItem(SyncItem& item);
    int modifyItem(SyncItem& item);
    int removeItem(SyncItem& item);
    int removeAllItems();

    // Proxy to the filesConfig methods.
    bool getIsSynced() const { return filesConfig.getIsSynced(); }
    void setIsSynced(bool v) { filesConfig.setIsSynced(v);       }


protected:

    /// Configuration object for the source. It's a reference to WindowsSyncSourceConfig
    /// object owned by OutlookConfig. It's automatically initialized in the constructor.
    WindowsSyncSourceConfig& filesConfig;

    /**
     * Overrides MediaSyncSource::getKeyAndSignature().
     * Utility method that populates the keyValuePair with
     * the couple key/signature starting from the SyncItem.
     * The SyncItem key set is the LUID of this item.
     * Used in the addItem and updateItem
     *
     * @param item - IN:  the SyncItem
     * @param kvp  - OUT: the KeyValuePair to be populate
     */
    virtual void getKeyAndSignature(SyncItem& item, KeyValuePair& kvp);


    /**
     * Filter out unwanted folders and files.
     */
    bool filterOutgoingItem(const StringBuffer& fullName, struct stat& st);

};

END_NAMESPACE

/** @} */
/** @endcond */
#endif
