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


#include "UpdateManager.h"
#include "http/WinTransportAgent.h"
#include "http/URL.h"
#include "http/Proxy.h"
#include "http/TransportAgent.h"
#include "http/TransportAgentFactory.h"
#include "base/errors.h"
#include "winmaincpp.h"
#include "HwndFunctions.h"
#include <shellapi.h>
#include "utils.h"


// Init static pointer.
UpdateManager* UpdateManager::pInstance = NULL;

// method used to get the singleton instance
UpdateManager* getUpdateManager(const char* comp, HWND hwnd) {
    return UpdateManager::getInstance(comp, hwnd);
}


UpdateManager* UpdateManager::getInstance(const char* comp, HWND hwnd) {
    if (pInstance == NULL) {
        OutlookConfig* cs = (OutlookConfig*)getConfig();
        pInstance = new UpdateManager(comp, hwnd, cs);
    }
    return pInstance;
}

UpdateManager::UpdateManager(const char* comp, HWND hwnd, OutlookConfig* cs) 
                                      : updater(comp, cs->getSwv(), cs->getUpdaterConfig()),
                                      uiHwnd(hwnd) {
}

UpdateManager::~UpdateManager() { 
}

void UpdateManager::dispose() {
    if (pInstance) {
        delete pInstance;
        pInstance = NULL;
    }
}

void UpdateManager::setHwnd(HWND hwnd) {
    this->uiHwnd = hwnd;
}


int UpdateManager::checkIsToUpdate() {
    return updater.checkIsToUpdate();
}

void UpdateManager::checkForUpdates() {
    updater.setUI(this);
    updater.start();
}

void UpdateManager::manualCheckForUpdates() {

    // Reset the "skipped", so the upgrade is done anyway
    UpdaterConfig& config = getConfig()->getUpdaterConfig();
    config.setSkipped("0");

    updater.setUI(this);

    // Start the upgrade without checking the time
    updater.newVersionAvailable();
}


bool UpdateManager::isNewVersionAvailable() {
    return updater.isNewVersionAvailable();
}


int32_t UpdateManager::askConfirmationForRecommendedUpgrade(const UpdaterConfig& config) {
    return askConfirmationForUpgrade(config);
}

int32_t UpdateManager::askConfirmationForMandatoryUpgrade(const UpdaterConfig& config) {
    return askConfirmationForUpgrade(config);
}

int32_t UpdateManager::askConfirmationForUpgrade(const UpdaterConfig& ) {

    UpdaterConfig& config = ((OutlookConfig*)getConfig())->getUpdaterConfig();

    int ret = showMessage();
    
    if (ret == 0) { 
        // response is yes  
        // minimize the window if there is one
        if (uiHwnd == NULL) {
            uiHwnd = HwndFunctions::getWindowHandle();
        }
        if (uiHwnd != NULL) {
            SendMessage(uiHwnd, ID_MYMSG_OK, NULL, NULL);
        }
    } else if (ret == 2) {
        // response is skip
        OpenMessageBox(uiHwnd, TYPE_SKIPPED_ACTION, NULL);   
    }
   
    return ret;

}

int UpdateManager::showMessage() {

    LOG.debug("UpdateManager::showMessage");
    UpdaterConfig& config = ((OutlookConfig*)getConfig())->getUpdaterConfig();
    if (uiHwnd == NULL) {
        LOG.debug("uiHwnd is NULL");
    }
    
    int ret = -1;
    int currentVersion = getBuildNumberFromVersion(config.getCurrentVersion().c_str());
    int version = getBuildNumberFromVersion(config.getVersion().c_str());
    
    if (config.getCurrentVersion() == config.getVersion()) {
        ret = 3;
        LOG.debug("The version required is the same installed on the client");
    } else if (currentVersion > version) {
        ret = 3;
        LOG.debug("The version required is lower than the installed on the client");
    } else if (config.getUpdateType() == UP_TYPE_OPTIONAL) {
        if (config.getVersion() == config.getSkipped()) {
            ret = 4;            
            LOG.debug("The version required is the same just skipped");
        } else {                
            ret = OpenMessageBox(uiHwnd, TYPE_NOW_LATER_SKIP_OPTIONAL, NULL);        
            LOG.debug("Asked if the user wants to update (optional): response (0Ye, 1La, 2Sk) %i", ret);
        }
    } else if (config.getUpdateType() == UP_TYPE_MANDATORY) {
        if (updater.isMandatoryUpdateActivationDateExceeded()) {
            ret = OpenMessageBox(uiHwnd, TYPE_NOW_EXIT_MANDATORY, NULL);        
            LOG.debug("The user must update or exit (mandatory): response (0Ye, 1Ex) %i", ret);            
        } else {
            ret = OpenMessageBox(uiHwnd, TYPE_NOW_LATER_MANDATORY, NULL);        
            LOG.debug("Asked if the user wants to update (mandatory): response (0Ye, 1La) %i", ret);
        }
    } else { // reccomended
        ret = OpenMessageBox(uiHwnd, TYPE_NOW_LATER_RECCOMENDED, NULL);        
        LOG.debug("Asked if the user wants to update (recommended): response (0Ye, 1La) %i", ret);
    }
      
    return ret;
}


int UpdateManager::getBuildNumberFromVersion(const char* swv) {
        
    int major=0, minor=0, build=0;
    if (!swv) {
        return 0;
    }    
    sscanf(swv, "%d.%d.%d", &major, &minor, &build);
    return (major*10000 + minor*100 + build);
}

bool UpdateManager::isRecommended() {
    UpdaterConfig& config = ((OutlookConfig*)getConfig())->getUpdaterConfig();
    return (config.getRecommended() != "0") ? true : false;
}



void UpdateManager::startUpgrade(const UpdaterConfig& config) {

    SHELLEXECUTEINFO lpExecInfo;
    memset(&lpExecInfo, 0, sizeof(SHELLEXECUTEINFO));
    lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);    
    
    wchar_t* site = toWideChar(config.getUrlUpdate().c_str());
    lpExecInfo.lpFile = site;
    lpExecInfo.nShow = SW_SHOWNORMAL;
    lpExecInfo.lpVerb = TEXT("open");
    ShellExecuteEx(&lpExecInfo);
    
    if (site) { 
        delete [] site; 
        site = NULL; 
    }
}

void UpdateManager::doExitAction(const UpdaterConfig& config) {
    if (uiHwnd == NULL) {
        uiHwnd = HwndFunctions::getWindowHandle();
    }
    if (uiHwnd != NULL) {
        SendMessage(uiHwnd, WM_CLOSE, NULL, NULL);
    }    
}

void UpdateManager::setURLCheck(const StringBuffer& syncURL)
{
    // Get the host & port info
    URL url;
    url.setURL(syncURL.c_str());
    StringBuffer port(":80");
    if (url.port != 0) { 
        port = ":"; 
        port += itow(url.port); 
    }        
            
    // Compose the url
    StringBuffer urlCheck(url.protocol);
    urlCheck += "://"; 
    urlCheck += url.host;
    urlCheck += port;
    urlCheck += UP_URL_RESOURCE;
    
    StringBuffer ss(UP_URL_RESOURCE);
    if (ss.find("http://")  != StringBuffer::npos ||
        ss.find("https://") != StringBuffer::npos ) {
        urlCheck = UP_URL_RESOURCE;
    }

    LOG.debug("Url to check = %s", urlCheck.c_str());
    
    // Save to registry.
    UpdaterConfig& config = ((OutlookConfig*)getConfig())->getUpdaterConfig();
    config.setUrlCheck(urlCheck);
    config.save();
}

bool UpdateManager::checkForMandatoryUpdateBeforeStarting() {    

    UpdaterConfig& config = ((OutlookConfig*)getConfig())->getUpdaterConfig();
    updater.setUI(this);
    return updater.newVersionAvailable(true);
}
