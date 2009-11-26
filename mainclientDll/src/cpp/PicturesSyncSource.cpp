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


#include "PicturesSyncSource.h"
#include "winmaincpp.h"
#include "utils.h"


using namespace std;


PicturesSyncSource::PicturesSyncSource(const WCHAR* name, WindowsSyncSourceConfig* wsc)
                                      : FileSyncSource(name, wsc->getCommonConfig()),
                                        picturesConfig(*wsc) {


    StringBuffer path = picturesConfig.getFolderPath();
    if (path.empty()) {
        // If empty, set the default path for pictures (shell folder)
        path = getDefaultPicturesPath();
        picturesConfig.setFolderPath(path.c_str());
    }
    
    // "folderPath" is the one read from config, stored in registry.
    // "dir" is used by FileSyncSource during the sync process.
    dir = path;
}



/// read-only access to configuration
const WindowsSyncSourceConfig& PicturesSyncSource::getConfig() const {
    return picturesConfig;
}

/// read-write access to configuration
WindowsSyncSourceConfig& PicturesSyncSource::getConfig() {
    return picturesConfig;
}


int PicturesSyncSource::beginSync() {
    checkAbortedSync();

    // From now we consider this source synced.
    picturesConfig.setIsSynced(true);

    return FileSyncSource::beginSync();
}

int PicturesSyncSource::endSync() {
    
    int ret = FileSyncSource::endSync();

    // Set end timestamp to config: here this source is finished.
    picturesConfig.setEndTimestamp((unsigned long)time(NULL));

    return ret;
}



Enumeration* PicturesSyncSource::getAllItemList() 
{
    checkAbortedSync();

    // Don't send anything in case of SLOW-SYNC.
    return NULL;
}


int PicturesSyncSource::insertItem(SyncItem& item) 
{
    LOG.debug("PicturesSyncSource::insertItem");
    checkAbortedSync();

    // Must check if the destination folder exists. If not, create it.
    int ret = createFolder(dir);
    if (ret) {
        LOG.error("Error adding picture from Server: cannot create destination folder '%s' (code %d)", dir, ret);
        return STC_COMMAND_FAILED;
    }

    return FileSyncSource::insertItem(item);
}


int PicturesSyncSource::modifyItem(SyncItem& item) 
{
    LOG.debug("PicturesSyncSource::modifyItem");
    checkAbortedSync();

    // Must check if the destination folder exists. If not, create it.
    int ret = createFolder(dir);
    if (ret) {
        LOG.error("Error adding picture from Server: cannot create destination folder '%s' (code %d)", dir, ret);
        return STC_COMMAND_FAILED;
    }

    return FileSyncSource::modifyItem(item);
}


int PicturesSyncSource::removeItem(SyncItem& item) 
{
    checkAbortedSync();

    int ret = STC_ITEM_NOT_DELETED;
    LOG.debug("PicturesSyncSource::removeItem -> pictures on Client cannot be deleted");
    return ret;
}

int PicturesSyncSource::removeAllItems() 
{
    checkAbortedSync();
    LOG.debug("PicturesSyncSource::removeAllItems -> pictures on Client cannot be deleted");

    // nothing to do
    return 0;
}




