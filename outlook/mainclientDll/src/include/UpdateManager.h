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

#ifndef INCL_UPDATE_MANAGER
#define INCL_UPDATE_MANAGER
    
#include <string>
#include <list>
#include "updater/UpdaterConfig.h"
#include "updater/Updater.h"
#include "updater/UpdaterUI.h"
#include "customization.h"
#include "OutlookConfig.h"

using namespace std;


/**
 * This class is the manager of the software-update process.
 * Uses the Updater class to handle the https requests to obtain update informations,
 * and to store the configuration.
 * Extends the UpdaterUI interface in order to implement the callbacks from the 
 * Updater: askConfirmationForUpgrade(), askConfirmationForRecommendedUpgrade(), startUpgrade().
 */
class UpdateManager : public UpdaterUI {

private:

    /// The sole instance
    static UpdateManager* pInstance;

    /// Used to check the update information, and store the config (UpdaterConfig).
    Updater updater;

    /// the hwnd window: if not NULL, will be used to show the messagebox popups.
    HWND uiHwnd;

    int showMessage();
    int getBuildNumberFromVersion(const char* swv);
    bool isRecommended();


protected:

    UpdateManager(const char* comp, HWND hwnd, OutlookConfig* cs);
    ~UpdateManager();


public:

    /**
     * Method to get the sole instance
     */
    static UpdateManager* getInstance(const char* comp, HWND hwnd);


    ///Sets the HWND window, used in case of display msgbox.
    void setHwnd(HWND hwnd);

    /**
     * check the update on the server. If it is necessary set a value to 1
     * return  false if there is no action to do
     * return  true  if there is something to download
     */
    int  checkIsToUpdate();

    /**
     * Starts the update check progress. This may result in connecting to the
     * update server, check if a new version is available. If this is not
     * required, we can still notify the user of a new version which was
     * previously postponed (later option).
     * The method returns true iff the check discovers a new version.
    */
    void checkForUpdates();

    /**
     * Like checkForUpdates(), but starts directly the upgrade without checking the time.
     * Also resets the "skipped", so the upgrade is done anyway.
     * This method is called when a manual upgrade is requested (from UI menu).
     */
    void manualCheckForUpdates();

    /**
     * Returns true if a new version is known to be available for upgrade. This
     * method does not query the upgrade server, but it uses the information
     * available in the config.
     */
    bool isNewVersionAvailable();

    /** 
     * Sets the "url-check" value in the updater configuration, given the syncURL.
     * The url-check is composed like: "http://<syncURL_host>/updateserver/update"
     * It must be called everytime the syncURL is changed.
     */
    void setURLCheck(const StringBuffer& syncURL);    
    
    /// Dispose the static pointer
    static void dispose();

    /**
    * @return true if there is to do a mandatory update. False otherwise
    */
    bool checkForMandatoryUpdateBeforeStarting();

    // ------------- From UpdaterUI interface ---------------
    int32_t askConfirmationForRecommendedUpgrade(const UpdaterConfig& config);
    int32_t askConfirmationForMandatoryUpgrade(const UpdaterConfig& config);
    int32_t askConfirmationForUpgrade(const UpdaterConfig& config);
    
    /// Starts effectively the upgrade (opens the browser)
    void startUpgrade(const UpdaterConfig& config);

    /// Starts effectively the upgrade (opens the browser)
    void doExitAction(const UpdaterConfig& config);
    // ------------------------------------------------------

};              

/// get the UpdateManager instance
UpdateManager* getUpdateManager(const char* comp, HWND hwnd);

#endif
