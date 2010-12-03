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

#include "spds/SyncItem.h"
#include "spds/SyncItemStatus.h"
#include "base/util/utils.h"
#include "base/adapter/PlatformAdapter.h"
#include "base/Log.h"
#include "syncml/core/TagNames.h"
#include "base/util/ArrayListEnumeration.h"
#include "customization.h"
#include "PicturesSyncSource.h"
#include "OutlookConfig.h"
#include "winmaincpp.h"
#include "utils.h"


using namespace std;

BEGIN_NAMESPACE

/**
 * Fills and returns a MediaSyncSourceParams object, with current informations
 * about Server URL, username and Client sw version.
 * These params are needed by MediaSyncSource to correctly handle the cache file.
 */
static MediaSyncSourceParams getMediaParams()
{
    OutlookConfig* config = getConfig();
    MediaSyncSourceParams params;

    params.setUrl      (config->getSyncURL());
    params.setUsername (config->getUsername());
    params.setPassword (config->getPassword());
    params.setSwv      (config->getSwv());
    params.setDeviceID (config->getDevID());

    params.setUserAgent(config->getUserAgent());

    // Set the filtering by size (exclude pics with size > MAX_IMAGE_SIZE)
    params.setFilterBySize(MAX_IMAGE_SIZE);

    // Set the filtering by date (exclude pics modified BEFORE install timestamp)
    /*  -- (filtering by date not implemented yet) --
    WindowsSyncSourceConfig* wssc = config->getSyncSourceConfig(PICTURE_);
    if (wssc) {
        if (wssc->getIncludeOldItems() == false) {
            params.setFilterByDate(config->getInstallTimestamp());   // it's in UTC
        }
    }
    */

    return params;
}

/**
 * We want to set the cache files under the config folder, in a subfolder
 * with the name of this source.
 */
static StringBuffer getCacheDir(const char* sourceName) {

    StringBuffer cacheDir = PlatformAdapter::getConfigFolder();
    if (!cacheDir.endsWith("\\") && !cacheDir.endsWith("/")) {
        cacheDir += "/";
    }
    cacheDir += sourceName;

    // Must check if the destination folder exists. If not, create it.
    int ret = createFolder(cacheDir.c_str());
    if (ret) {
        LOG.error("Error in picture sync: cannot create cache folder '%s' (code %d)", cacheDir.c_str(), ret);
        return "/";
    }

    LOG.debug("PictureSyncSource: cache dir is %s", cacheDir.c_str());
    return cacheDir;
}


PicturesSyncSource::PicturesSyncSource(const WCHAR* name, WindowsSyncSourceConfig* wsc)
                                      : MediaSyncSource(name, wsc->getCommonConfig(),
                                                        getCacheDir(wsc->getName()),    // fake the dir, to set the cache. It's then set in the constructor.
                                                        getMediaParams()),
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

    return MediaSyncSource::beginSync();
}

int PicturesSyncSource::endSync() {

    int ret = MediaSyncSource::endSync();

    // Set end timestamp to config: here this source is finished.
    picturesConfig.setEndTimestamp((unsigned long)time(NULL));

    return ret;
}


Enumeration* PicturesSyncSource::getAllItemList()
{
    checkAbortedSync();
    return MediaSyncSource::getAllItemList();
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

    ret = FileSyncSource::insertItem(item);

    // Fix the LUID mapping: for new items, the item's key returned by the client
    // is the luid (incremental number), not the file name.
    if (item.getKey()) {
        StringBuffer fullName = getCompleteName(dir.c_str(), item.getKey());
        StringBuffer luid = getLUIDFromPath(fullName);

        LOG.debug("PicturesSyncSource::insertItem - LUID used for '%ls' is %s", item.getKey(), luid.c_str());
        WCHAR* wluid = toWideChar(luid.c_str());
        item.setKey(wluid);
        delete [] wluid;
    }
    return ret;
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

    // Fix the LUID mapping: for updates and deletes, the item's key sent by the
    // Server is the luid, which must be converted to the local file name.
    WString wluid = item.getKey();
    StringBuffer luid;
    luid.convert(wluid.c_str());
    StringBuffer fullName = getPathFromLUID(luid);

    StringBuffer name = getFileNameFromPath(fullName);

    LOG.debug("PicturesSyncSource::modifyItem - LUID '%s' is associated to %s", luid.c_str(), name.c_str());
    WCHAR* wname = toWideChar(name.c_str());
    item.setKey(wname);     // only the file name is required
    delete [] wname;


    ret = FileSyncSource::modifyItem(item);

    // restore the original key (the luid)
    item.setKey(wluid.c_str());
    return ret;
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
    LOG.info("Removing ALL pictures from folder: '%s'", dir.c_str());
    checkAbortedSync();

    return MediaSyncSource::removeAllItems();
}



void PicturesSyncSource::getKeyAndSignature(SyncItem& item, KeyValuePair& kvp)
{
    // FIX the item's key.
    // Incoming item: the key for the cache is the full path.
    StringBuffer key;
    key.convert(item.getKey());

    StringBuffer path = getPathFromLUID(key);
    StringBuffer sign = getItemSignature(path);

    if (!path.null()) {
        kvp.setKey(path);
        kvp.setValue(sign);
    }
}


bool PicturesSyncSource::filterOutgoingItem(const StringBuffer& fullName, struct stat& st)
{
    // [removed] Skip images bigger than MAX_IMAGE_SIZE
    // (it's done in MediaSyncSource)


    if (! S_ISDIR(st.st_mode) ) {
        // Skip non-image files
        if ( !checkFileExtension(fullName, "jpg",  true) &&
             !checkFileExtension(fullName, "jpeg", true) &&
             !checkFileExtension(fullName, "gif",  true) &&
             !checkFileExtension(fullName, "png",  true) ) {
            // not an image file
            return true;
        }
    }

    return MediaSyncSource::filterOutgoingItem(fullName, st);
}


END_NAMESPACE
