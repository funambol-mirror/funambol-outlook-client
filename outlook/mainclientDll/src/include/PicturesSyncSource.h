/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2009 Funambol, Inc.
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

#ifndef INCL_PICTURES_SYNC_SOURCE
#define INCL_PICTURES_SYNC_SOURCE

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "base/fscapi.h"
#include "WindowsSyncSource.h"
#include "spds/FileData.h"
#include "client/CacheSyncSource.h"
#include "client/FileSyncSource.h"

BEGIN_NAMESPACE

/**
 * This class extends the FileSyncSource class, it's specialized for pictures items. 
 * The sync direction is fixed on "one-way-from-server", so the method getAllItemList()
 * symply does nothing. 
 * Deletes coming from Server are not expected, so th method
 * removeItem() is not implemented (we don't want to delete pictures on Client).
 * "picturesConfig::folderPath" and "dir" have the same meaning, so they are set at
 * the same value in the constructor.
 * Configuration paramenters that need to be saved inside the registry are stored in the
 * WindowsSyncSourceConfig& picturesConfig object, owned by OutlookConfig.
 */
class PicturesSyncSource : public FileSyncSource
{

protected:

    /// Configuration object for the source. It's a reference to WindowsSyncSourceConfig
    /// object owned by OutlookConfig. It's automatically initialized in the constructor.
    WindowsSyncSourceConfig& picturesConfig;


public:

    /**
     * Constructor. if folderPath is empty, here we read and set the default
     * value, which is the shell folder for pictures (i.e. "C:\Users\<username>\Images")
     * "dir" is used by FileSyncSource during the sync process, so it's set to the same
     * value of "folderPath".
     */
    PicturesSyncSource(const WCHAR* name, WindowsSyncSourceConfig* sc);
    ~PicturesSyncSource() {};


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



    // Proxy to the picturesConfig methods.
    bool getIsSynced() const { return picturesConfig.getIsSynced(); }
    void setIsSynced(bool v) { picturesConfig.setIsSynced(v);       }

};

END_NAMESPACE

/** @} */
/** @endcond */
#endif
