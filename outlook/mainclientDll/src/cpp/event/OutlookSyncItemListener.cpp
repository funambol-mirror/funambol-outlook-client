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


#include "event/OutlookSyncItemListener.h"
#include "base/Log.h"
#include "HwndFunctions.h"
#include "utils.h"

/* 
   wParam = -1 -> sending items
   wParam = 1  -> receiving items 
 */

OutlookSyncItemListener::OutlookSyncItemListener() : 
                                 uploadingItemKey(TEXT("")), 
                                 downloadingItemKey(TEXT("")) {
}

void OutlookSyncItemListener::itemAddedByServer(SyncItemEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM) 1 , (LPARAM) syncSourceNameToIndex(event.getSourceName()));
}

void OutlookSyncItemListener::itemDeletedByServer(SyncItemEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM) 1, (LPARAM) syncSourceNameToIndex(event.getSourceName()));
}

void OutlookSyncItemListener::itemUpdatedByServer(SyncItemEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM) 1, (LPARAM) syncSourceNameToIndex(event.getSourceName()));
}

void OutlookSyncItemListener::itemAddedByClient(SyncItemEvent &event) {

    int sourceID = syncSourceNameToIndex(event.getSourceName());
    if (sourceID == SYNCSOURCE_PICTURES || 
        sourceID == SYNCSOURCE_VIDEOS   ||
        sourceID == SYNCSOURCE_FILES) {
        if (OutlookConfig::getInstance()->getServerMediaHttpUpload()) {
            // Ignore this event: it's just the syncML metadata (no item content)
            return;
        }
    }

    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, 
               (WPARAM)-1, (LPARAM)sourceID);
}

void OutlookSyncItemListener::itemUpdatedByClient(SyncItemEvent &event) {
    
    int sourceID = syncSourceNameToIndex(event.getSourceName());
    if (sourceID == SYNCSOURCE_PICTURES || 
        sourceID == SYNCSOURCE_VIDEOS   ||
        sourceID == SYNCSOURCE_FILES) {
        if (OutlookConfig::getInstance()->getServerMediaHttpUpload()) {
            // Ignore this event: it's just the syncML metadata (no item content)
            return;
        }
    }
    
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, 
               (WPARAM)-1, (LPARAM)sourceID);
}

void OutlookSyncItemListener::itemUploadedByClient(SyncItemEvent &event) {
    
    int sourceID = syncSourceNameToIndex(event.getSourceName());
    if (sourceID == SYNCSOURCE_PICTURES || 
        sourceID == SYNCSOURCE_VIDEOS   ||
        sourceID == SYNCSOURCE_FILES) {
        if (OutlookConfig::getInstance()->getServerMediaHttpUpload()) {
            // A media item has been uploaded to the server
            SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, 
                       (WPARAM)-1, (LPARAM)sourceID);
        }
    }
}

void OutlookSyncItemListener::itemDeletedByClient(SyncItemEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM) -1, (LPARAM) syncSourceNameToIndex(event.getSourceName()));
}


// Media sync listeners
void OutlookSyncItemListener::itemUploading(SyncItemEvent& event) {
    int sourceID = syncSourceNameToIndex(event.getSourceName());

    // in case of retry, the item's key is the same (no need to update UI)
    if (uploadingItemKey != event.getItemKey()) {
        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM)-1, (LPARAM)sourceID);
        uploadingItemKey = event.getItemKey();
    }
}

void OutlookSyncItemListener::itemUploaded(SyncItemEvent& event) {
    uploadingItemKey = TEXT("");
    LOG.debug("end upload");
}

void OutlookSyncItemListener::itemDownloading(SyncItemEvent& event) {
    int sourceID = syncSourceNameToIndex(event.getSourceName());

    // in case of retry, the item's key is the same (no need to update UI)
    if (downloadingItemKey != event.getItemKey()) {
        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNC_ITEM_SYNCED, (WPARAM)1, (LPARAM)sourceID);
        downloadingItemKey = event.getItemKey();
    }
}

void OutlookSyncItemListener::itemDownloaded(SyncItemEvent& event) {
    downloadingItemKey = TEXT("");
    LOG.debug("end download");
}
