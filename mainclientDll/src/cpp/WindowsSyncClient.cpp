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

#include "spds/spdsutils.h"
#include "WindowsSyncClient.h"
#include "winmaincpp.h"
#include "utils.h"
#include "HwndFunctions.h"

#include <list>
#include <string>
using namespace std;


typedef list<wstring>               sourceNameList;
typedef sourceNameList::iterator    sourceNameIterator;


/**
 * Constructor. Set the pointer of (externally owned) SyncSources array.
 */
WindowsSyncClient::WindowsSyncClient(WindowsSyncSource** sources) : SyncClient() {
    winSyncSources = sources;
}

WindowsSyncClient::~WindowsSyncClient() {
}



/**
 * Checks if the server requested a full-sync for at least one SyncSource.
 * This method is called just after 'prepareSync()'. If a full-sync has been
 * requested by server, a timed message-box will be prompted to warn the user.
 * If user avoid starting the full sync, en error code (5) will be returned.
 * In case of no user-interaction, after a time out (10sec) the full-sync will 
 * normally continue.
 *
 * @return 0 if synchronization is to continue,
 *         5 if user aborted the synchronization (error code 5 = Sync aborted by the user to avoid full-sync)
 */
int WindowsSyncClient::continueAfterPrepareSync() {
    int ret = 0;
    LOG.debug("Checking for forced slow-sync...");
    
    // it sets to use the timezone in outgoing recurring appointment
    ClientApplication* cApp = ClientApplication::getInstance();
    cApp->setOutgoingTimezone(true);

    if (winSyncSources == NULL) {
        return 0;
    }


    if (DISPLAY_SLOWSYNC_WARNING) {

        sourceNameList      slowSyncList;
        sourceNameIterator  iter;
        slowSyncList.clear();
        SyncMode currentSyncMode, initialSyncMode;

        //
        // Create a list of sources that will run a slow-sync (requested by server).
        //
        int i=0;
        while (winSyncSources[i]) {
            currentSyncMode = winSyncSources[i]->getSyncMode();
            initialSyncMode = syncModeCode(winSyncSources[i]->getConfig().getSync());

            if ( isFullSyncMode(currentSyncMode) && (initialSyncMode != currentSyncMode) ) {
                slowSyncList.push_back(winSyncSources[i]->getName());
            }
            i++;
        }


        //
        // If necessary, display warning (timed messagebox)
        //
        int size = slowSyncList.size();
        if (size > 0) {

            // Format a smart message...
            wstring message = WMSG_BOX_ASK_SLOW_1;
            wstring names;
            iter = slowSyncList.begin();
            
            names += (*iter);
            names += L"s";
            iter ++;
            i = 1;
            while (iter != slowSyncList.end()) {
                if (i == size-1)  names += L" and ";
                else              names += L", ";

                names += (*iter);
                names += L"s";
                iter ++;
                i ++;
            }
            message += names;

            WCHAR* tmp = new WCHAR[wcslen(WMSG_BOX_ASK_SLOW_2) + 10];
            wsprintf(tmp, WMSG_BOX_ASK_SLOW_2, ASK_SLOW_TIMEOUT);
            message += tmp;
            delete [] tmp;


            // Prompt a timed MessageBox (10sec default)
            unsigned int flags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND| MB_TOPMOST;
            int res = TimedMessageBox(NULL, message.c_str(), TEXT(PROGRAM_NAME), flags, ASK_SLOW_TIMEOUT * 1000);

            // '-1' is returned after the time-out.
            if (res == IDYES || res == -1) {
                ret = 0;
            }
            else {
                setErrorF(5, INFO_SYNC_ABORTED_BY_USER_SLOW, names.c_str());
                LOG.info(getLastErrorMsg());
                ret = 5;           // code 5 = Sync aborted by the user to avoid full-sync.
            }
        }
    }
	

    //
	// Send a msg to UI: now re-enable the UI buttons!
    //
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_UNLOCK_BUTTONS, NULL, NULL);

    return ret;
}

       