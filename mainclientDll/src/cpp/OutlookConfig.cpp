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

#include "base/stringUtils.h"
#include "spdm/dmtree.h"
#include "spdm/DMTreeFactory.h"
#include "base/debug.h"
#include "spds/spdsutils.h"
#include "OutlookConfig.h"
#include "DefaultWinConfigFactory.h"
#include "winmaincpp.h"
#include "utils.h"
#include "DateFilter.h"

#include "outlook/ClientApplication.h"
#include "outlook/ClientException.h"

#include "base/adapter/PlatformAdapter.h"

using namespace std;


//
// Init static pointer.
//
OutlookConfig* OutlookConfig::pinstance = NULL;


/**
 * Method to get the sole instance of OutlookConfig
 */
OutlookConfig* OutlookConfig::getInstance() {
    if (pinstance == NULL) {
        PlatformAdapter::init(APPLICATION_URI);
        pinstance = new OutlookConfig;
    }
    return pinstance;
}

/// Returns true if static instance is not NULL.
bool OutlookConfig::isInstantiated() {
    return (pinstance ? true : false);
}


/// Constructor
//OutlookConfig::OutlookConfig() : DMTClientConfig(APPLICATION_URI) {
OutlookConfig::OutlookConfig() {
    
    DMTClientConfig::initialize();
    winSourceConfigs      = NULL;
    workingDir            = NULL;
    logDir                = NULL;
    winSourceConfigsCount = 0;
    fullSync              = false;
    abortSync             = false;
    upgraded              = false;
    oldSwv                = 0;
}

/// Destructor
OutlookConfig::~OutlookConfig() {

    if (workingDir) {
        delete [] workingDir;
        workingDir = NULL;
    }
    if (logDir) {
        delete [] logDir;
        logDir = NULL;
    }

    if (winSourceConfigs) {
        delete [] winSourceConfigs;
        winSourceConfigs = NULL;
    }
    pinstance = NULL;
}



// ------------------------- Read properties from win registry -------------------------
/**
 * Read the configuration from Windows registry into this object. 
 * This method overrides 'DMTClientConfig::read()'.
 * 'DMTClientConfig::read()' is first called to read all common properties, then
 * specific SyncSource properties are retrieved.
 *
 * A separate 'winSourceConfigs' array is used to store all specific SS config, common
 * props are linked to original 'sourceConfigs' array (no copy!).
 *
 * @return TRUE if no errors
 */
bool OutlookConfig::read() {

    unsigned int i=0;

    // Read timezone info.
    readCurrentTimezone();

    //
    // Read common properties
    //
    //lastErrorCode = ERR_NONE;
    resetError();
    DMTClientConfig::read();
    if (getLastErrorCode() != ERR_NONE) {
        // Double check on the value of swv. If empty, we consider the config not existing.
        StringBuffer currentSwv(getClientConfig().getSwv());
        if (currentSwv.empty()) {
            return false;
        }
    }

    // Username/Password are stored encrypted (new since 6.0.9).
    decryptPrivateData();


    if (sourceConfigsCount < 1) {
        return false;
    }
    winSourceConfigsCount = sourceConfigsCount;

    //
    // Read additional properties for SyncSources (use winSyncSourceConfig)
    //
    if (winSourceConfigs) {
        delete [] winSourceConfigs;
    }
    if (!open()) {
        return false;
    }

    winSourceConfigs = new WindowsSyncSourceConfig[sourceConfigsCount];
    for (i=0; i<sourceConfigsCount; i++) {

        // Link internal pointer to sourceConfigs array
        winSourceConfigs[i].setCommonConfig(DMTClientConfig::getSyncSourceConfig(i));

        // Read specific properties
        readWinSourceConfig(i);
    }
    close();


    // Current working dir: read 'installDir' from HKLM
    char* installPath = readPropertyValue(APPLICATION_URI, PROPERTY_INSTALLDIR, HKEY_LOCAL_MACHINE);
    if (!installPath || strlen(installPath) == 0) {
        LOG.error(ERR_INSTALL_DIR);
        char msg[100];
        sprintf(msg, ERR_HKLM_KEYNOTFOUND, PROPERTY_INSTALLDIR);
        safeMessageBox(msg);
        exit(1);
    }
    setWorkingDir(installPath);


    // Current data dir: for LOG file and temporary files (no win registry).
    WCHAR* wlogPath = readAppDataPath();
    if (!wlogPath) {
        LOG.error(getLastErrorMsg());
        return false;
    }
    char* logPath = toMultibyte(wlogPath);
    setLogDir(logPath);


    // Reset fullSync/abortSync flags
    fullSync  = false;
    abortSync = false;


    if (installPath) delete [] installPath;
    if (logPath)     delete [] logPath;
    if (wlogPath)    delete [] wlogPath;
    return true;
}

/**
 * Read client-specific SyncSource properties from Win registry.
 * @param i : the index of node (and syncsource) under 'sourcesNode' node
 */
void OutlookConfig::readWinSourceConfig(unsigned int i) {

    char* tmp;
    if (!sourcesNode) {
        open();
    }

    ManagementNode* node = sourcesNode->getChild(i);
    
    if (node) {
        tmp = node->readPropertyValue(PROPERTY_USE_SUBFOLDERS);    
        winSourceConfigs[i].setUseSubfolders((*tmp == '1') ? true : false);
        delete [] tmp;

        tmp = node->readPropertyValue(PROPERTY_FOLDER_PATH);    
        winSourceConfigs[i].setFolderPath(tmp);
        delete [] tmp;

        tmp = node->readPropertyValue(PROPERTY_SYNC_END); 
        winSourceConfigs[i].setEndTimestamp( ((*tmp) ? strtol(tmp, NULL, 10) : 0) );
        delete [] tmp;

        //
        // For appoitment source: read filtering params and populate DateFilter.
        //
        if (!strcmp(winSourceConfigs[i].getName(), APPOINTMENT_)) {
            DateFilter& filter = winSourceConfigs[i].getDateFilter();

            tmp = node->readPropertyValue(PROPERTY_FILTER_DATE_DIRECTION);
            if (tmp) {
                filter.setDirection((DateFilter::FilterDirection)atoi(tmp));
            }
            delete [] tmp;

            tmp = node->readPropertyValue(PROPERTY_FILTER_DATE_LOWER);
            if (tmp && (strlen(tmp) == 1)) {
                filter.setRelativeLowerDate((DateFilter::RelativeLowerDate)atoi(tmp));
            }
            delete [] tmp;
        }
    }
}



/**
 * Reads all sources timestamps from win registry and set
 * all values into configuration.
 */
void OutlookConfig::readSourcesTimestamps() {

    if (!open()) {
        return;
    }

    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        ManagementNode* node = sourcesNode->getChild(i);
        if (node) {
            // This sets only variables that the library uses internally, like anchors 
            readSourceVars(i, *sourcesNode, *node);
        }
    }

    close();
}


/**
 * Reads only "sync" properties of each source, to win registry.
 */
void OutlookConfig::readSyncModes() {

    if (!open()) {
        return;
    }

    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        ManagementNode* node = sourcesNode->getChild(i);
        if (node) {
            winSourceConfigs[i].setSync(node->readPropertyValue(PROPERTY_SOURCE_SYNC));
        }
    }

    close();
}


/**
 * Populate 'currentTimezone' structure, reading values from TIME_ZONE_INFORMATION
 * and also directly from Windows Registry.
 * @note  some mandatory informations cannot be retrieved from Win32 API calls,
 *        so we have to get them from HKLM keys.
 * @return 0 if no errors
 */
int OutlookConfig::readCurrentTimezone() {

    //
    // Get all known info from TIME_ZONE_INFORMATION.
    //
    TIME_ZONE_INFORMATION tzInfo;
    DWORD tzID = GetTimeZoneInformation(&tzInfo);
    if (tzID == TIME_ZONE_ID_DAYLIGHT) {
        currentTimezone.isDaylight = true;
    }
    else {
        currentTimezone.isDaylight = false;
    }
    currentTimezone.bias         = tzInfo.Bias;
    currentTimezone.daylightBias = tzInfo.DaylightBias;
    currentTimezone.daylightDate = tzInfo.DaylightDate;
    currentTimezone.daylightName = tzInfo.DaylightName;
    currentTimezone.standardBias = tzInfo.StandardBias;
    currentTimezone.standardDate = tzInfo.StandardDate;
    currentTimezone.standardName = tzInfo.StandardName;

    //
    // Now go directly to Win Registry keys and get the 
    // other mandatory informations.
    //
    bool found = false;
    HKEY hkTimeZones;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TIMEZONE_CONTEXT, 0, KEY_READ, &hkTimeZones) == ERROR_SUCCESS) {
        HKEY  hkTimeZone;
        DWORD dwIndex = 0;
        WCHAR keyName[DIM_MANAGEMENT_PATH];
        DWORD keyNameLenght = DIM_MANAGEMENT_PATH;

        // Scan all timezones, searching for the current one.
        while (RegEnumKey(hkTimeZones, dwIndex++, keyName, keyNameLenght) != ERROR_NO_MORE_ITEMS) {
            if (RegOpenKeyEx(hkTimeZones, keyName, 0, KEY_READ, &hkTimeZone) == ERROR_SUCCESS) {

                WCHAR stdName[DIM_MANAGEMENT_PATH];
                DWORD dwDataSize = DIM_MANAGEMENT_PATH * sizeof(WCHAR);
                RegQueryValueEx(hkTimeZone, L"Std", NULL, NULL, (BYTE*)&stdName, &dwDataSize);
                if (!wcscmp(stdName, currentTimezone.standardName.c_str())) {
                    found = true;

                    // Get Index
                    DWORD dwTimeZoneIndex;
                    dwDataSize = sizeof(DWORD);
                    RegQueryValueEx(hkTimeZone, L"Index", NULL, NULL, (BYTE*)&dwTimeZoneIndex, &dwDataSize);

                    // Get Display name
                    WCHAR displayName[DIM_MANAGEMENT_PATH];
                    dwDataSize = DIM_MANAGEMENT_PATH * sizeof(WCHAR);
                    RegQueryValueEx(hkTimeZone, L"Display", NULL, NULL, (BYTE*)&displayName, &dwDataSize);

                    // Set properties to currentTimezone struct.
                    currentTimezone.index       = dwTimeZoneIndex;
                    currentTimezone.displayName = displayName;
                    currentTimezone.keyName     = keyName;

                    RegCloseKey(hkTimeZone);
                    break;
                }
            }
            keyNameLenght = DIM_MANAGEMENT_PATH;
            RegCloseKey(hkTimeZone);
        }
        RegCloseKey(hkTimeZones);
    }
    else {
        return 1;
    }

    if (!found) {
        LOG.info("Error reading the timezone info from Win Registry");
        return 1;
    }

    LOG.debug("Current Timezone = %ls", currentTimezone.displayName.c_str());
    return 0;
}



// ---------------------------- Save properties to win registry ----------------------------
/**
 * Save the configuration from this object into Windows registry.
 * If SyncReport pointer is passed not NULL, each SyncSource configuration
 * will be saved ONLY if that source was successfully synced.
 * This method overrides 'DMTClientConfig::save()'.
 *
 * A separate 'winSourceConfigs' array is used to store all SS config, so a
 * specific method 'saveWinSourceConfig()' is used to save sources config
 * into the windows registry.
 *
 * @return TRUE if no errors
 */
bool OutlookConfig::save(SyncReport* report) {

    bool ret = false;
    LOG.debug(DBG_WRITING_CONFIG_TO_DM);

    if (!open()) {
        return false;
    }

    // Username/Password are stored encrypted (new since 6.0.9).
    encryptPrivateData();

    //
    // SyncML management node (TBD: manage dirty flags!)
    //
    saveAccessConfig(*syncMLNode);
    saveDeviceConfig(*syncMLNode);

    // If asked, we need to return clear data...
    decryptPrivateData();


    //
    // Sources management node
    // -------------------------------
    // Save source props only if:
    // - report is NULL (we are not after a sync)
    // OR
    // - source completed successfully
    // -------------------------------
    //lastErrorCode = ERR_NONE;
    resetError();
    for(unsigned int i=0; i<sourceConfigsCount; i++) {
        if ( (report==NULL) || 
             (report->getSyncSourceReport(i) && report->getSyncSourceReport(i)->checkState()) ) {
            saveWinSourceConfig(i);
        }
    }
    //ret = (lastErrorCode == ERR_NONE);
    resetError();
    ret = (getLastErrorCode() != 0);

    close();
    return ret;
}

// Standard call to save configuration, ALL properties will be saved.
bool OutlookConfig::save() {
    return save(NULL);
}


/**
 * Save WindowsSyncSourceConfig properties in DMTree for the desired Source.
 * Source properties are placed in specific node under sources node.
 * Notes:
 * if the node for the current source is not found, it is created!
 * if we are under a restore sync (slow/refresh), 'sync' property will 
 * be skipped (keep previous value from registry)
 *
 * @param i : the index of SyncSource node
 */
void OutlookConfig::saveWinSourceConfig(unsigned int i) {

    ManagementNode* node;
    char nodeName[DIM_MANAGEMENT_PATH];

    if (!sourcesNode) {
        open();
    }

    //
    // If node not found, create node from Source name.
    //
    if (sourcesNode->getChild(i) == NULL) {
        char* fn = sourcesNode->createFullName();
        sprintf(nodeName, "%s/%s", fn, winSourceConfigs[i].getName());
        delete [] fn;
        node = dmt->readManagementNode(nodeName);
        //LOG.debug(INFO_CONFIG_NODE_CREATED, nodeName);
    }
    else {
        node = (ManagementNode*)sourcesNode->getChild(i)->clone();
    }


    //
    // Save source properties
    //
    if (node) {
        char buf[512];

        // Specific props:
        node->setPropertyValue(PROPERTY_USE_SUBFOLDERS,    (winSourceConfigs[i].getUseSubfolders () ? "1" : "0"));    
        node->setPropertyValue(PROPERTY_FOLDER_PATH,        winSourceConfigs[i].getFolderPath    ());
        timestampToAnchor(winSourceConfigs[i].getEndTimestamp(), buf); 
        node->setPropertyValue(PROPERTY_SYNC_END,           buf);

        // Common props:
        node->setPropertyValue(PROPERTY_SOURCE_NAME,        winSourceConfigs[i].getName          ());    
        node->setPropertyValue(PROPERTY_SOURCE_URI,         winSourceConfigs[i].getURI           ());
        node->setPropertyValue(PROPERTY_SOURCE_TYPE,        winSourceConfigs[i].getType          ());
        node->setPropertyValue(PROPERTY_SOURCE_VERSION,     winSourceConfigs[i].getVersion       ());
        node->setPropertyValue(PROPERTY_SOURCE_SYNC_MODES,  winSourceConfigs[i].getSyncModes     ());
        node->setPropertyValue(PROPERTY_SOURCE_ENCODING,    winSourceConfigs[i].getEncoding      ());    
        node->setPropertyValue(PROPERTY_SOURCE_SUPP_TYPES,  winSourceConfigs[i].getSupportedTypes());
        node->setPropertyValue(PROPERTY_SOURCE_ENCRYPTION,  winSourceConfigs[i].getEncryption    ());

        timestampToAnchor(winSourceConfigs[i].getLast(), buf); 
        node->setPropertyValue(PROPERTY_SOURCE_LAST_SYNC, buf);

        // If we are just after a sync and this is a full sync (slow/refresh), 
        // DO NOT save the 'sync' property (so won't be a restore again next time).
        if (fullSync == false) {
            node->setPropertyValue(PROPERTY_SOURCE_SYNC,    winSourceConfigs[i].getSync          ());
        }

        // Save filtering props
        if (!strcmp(winSourceConfigs[i].getName(), APPOINTMENT_)) {
            StringBuffer buf("");
            DateFilter& filter = winSourceConfigs[i].getDateFilter();

            buf.sprintf("%d", filter.getRelativeLowerDate());
            node->setPropertyValue(PROPERTY_FILTER_DATE_LOWER, buf.c_str());

            buf.sprintf("%d", filter.getDirection());
            node->setPropertyValue(PROPERTY_FILTER_DATE_DIRECTION, buf.c_str());
        }
        delete node; 
    }
}


/**
 * Save only "sync" properties of each source, to win registry.
 */
void OutlookConfig::saveSyncModes() {

    if (!sourcesNode) {
        open();
    }

    ManagementNode* node = NULL;
    for(unsigned int i=0; i<sourceConfigsCount; ++i) {
        node = sourcesNode->getChild(i);
        if (node) {
            node->setPropertyValue(PROPERTY_SOURCE_SYNC, winSourceConfigs[i].getSync());
        }
        node = NULL;
    }

    close();
}


/**
 * Save only "beginSync" property to win registry.
 */
void OutlookConfig::saveBeginSync() {

    DMTree* dmt          = NULL;
    ManagementNode* node = NULL;
    char context[DIM_MANAGEMENT_PATH];
    char buf[32];

    // Get node.
    sprintf(context, "%s%s%s", APPLICATION_URI, CONTEXT_SPDS_SYNCML, CONTEXT_EXT);
    dmt = DMTreeFactory::getDMTree(context);
    if (!dmt)   goto finally;
    node = dmt->readManagementNode(context);
    if (!node)  goto finally;

    // Set value.
    timestampToAnchor(getAccessConfig().getBeginSync(), buf);
    node->setPropertyValue(PROPERTY_SYNC_BEGIN, buf);

finally:
    if (dmt)   delete dmt;
    if (node)  delete node;
    return;
}



// ------------------------------ Get/Set objects ----------------------------------

/**
 * Return a pointer to the internal WindowsSyncSourceConfig object from 
 * its name (must NOT be freed by caller).
 * This method replaces 'getSyncSourceConfig()' of DMTClientConfig.
 *
 * @param name : the source name
 * @return       the correspondent WindowsSyncSourceConfig pointer
 */
_declspec(dllexport) WindowsSyncSourceConfig* OutlookConfig::getSyncSourceConfig(const char* name) {
    if ((name == NULL) || (strlen(name) == 0)) {
        return NULL;
    }

    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        if (strcmp(winSourceConfigs[i].getName(), name) == 0) {
            return &winSourceConfigs[i];
        }
    }

    return FALSE;
}

/**
 * Return a pointer to the internal WindowsSyncSourceConfig object from 
 * its index in winSourceConfigs array (must NOT be freed by caller).
 * This method replaces 'getSyncSourceConfig()' of DMTClientConfig.
 *
 * NOTE: please use the "getSyncSourceConfig(const char* name)" method, to 
 *       ensure the correct WindowsSyncSourceConfig* is used!
 *
 * @param i  : the index of source in winSourceConfigs array
 * @return     the correspondent WindowsSyncSourceConfig pointer
 */
 WindowsSyncSourceConfig* OutlookConfig::getSyncSourceConfig(unsigned int i) {
    if (i >= sourceConfigsCount) {
        return NULL;
    }

    return &winSourceConfigs[i];
}



/**
 * Set the passed WindowsSyncSourceConfig object into the correspondent object
 * inside 'winSourceConfigs' array. The values are copied into the object that
 * matches the same name of the passed one.
 * This method replaces the 'SyncManagerConfig::setSyncSourceConfig()'.
 * Note:
 * If a WindowsSyncSourceConfig with the same name is not found, the passed
 * object is added at the end of the 'winSourceConfig' array.
 *
 * @param wsc : the WindowsSyncSourceConfig passed by reference
 * @return      TRUE if no errors
 */
BOOL OutlookConfig::setSyncSourceConfig(WindowsSyncSourceConfig& wsc) {

    unsigned int i=0;
    for (i=0; i<winSourceConfigsCount; ++i) {
        if (strcmp(wsc.getName(), winSourceConfigs[i].getName()) == 0) {
            break;
        }
    }
    if (i >= winSourceConfigsCount) {
        // Not found! -> add the WindowsSyncSourceConfig.
        return addSyncSourceConfig(wsc);
    }

    // copy all values
    winSourceConfigs[i] = wsc;

    return TRUE;
}


bool OutlookConfig::addWindowsSyncSourceConfig(const wstring& sourceName) 
{

    unsigned int backupSourceConfigsCount = sourceConfigsCount;

    try {
        //
        // Set (add) the default SyncSourceConfig (common props)
        //
        SyncSourceConfig* sc = DefaultWinConfigFactory::getSyncSourceConfig(sourceName.c_str());
        DMTClientConfig::setSyncSourceConfig(*sc);
        delete sc;

        //
        // Check if we added a new SSourceConfig
        //
        if (sourceConfigsCount > backupSourceConfigsCount) {

            // The winSourceConfigs array is corrupted: "s" links point to free memory. 
            // So we recreate it and link common props again.
            if (winSourceConfigs) {
                delete [] winSourceConfigs;
            }
            winSourceConfigs = new WindowsSyncSourceConfig[sourceConfigsCount];
            for (unsigned int i=0; i<sourceConfigsCount; i++) {
                // Link internal pointer to sourceConfigs array
                winSourceConfigs[i].setCommonConfig(DMTClientConfig::getSyncSourceConfig(i));
            }
        }

        //
        // Set (add) the default WindowsSyncSourceConfig
        //
        char* name = toMultibyte(sourceName.c_str());
        sc = DMTClientConfig::getSyncSourceConfig(name);
        WindowsSyncSourceConfig* wsc = DefaultWinConfigFactory::getWinSyncSourceConfig(sourceName, sc);
        setSyncSourceConfig(*wsc);
        delete [] name;
        delete wsc;

    }
    catch (char* e) {
        setErrorF(getLastErrorCode(), ERR_DEFAULT_SSCONFIG, PICTURE_, e);
        safeMessageBox(getLastErrorMsg());
        return false;
    }
    return true;
}




/**
 * Adds the passed WindowsSyncSourceConfig.
 * It is added at the end of the 'winSourceConfig' array.
 * This method replaces the 'SyncManagerConfig::addSyncSourceConfig()'.
 *
 * @param wsc : the WindowsSyncSourceConfig passed by reference
 * @return      TRUE if no errors
 */
BOOL OutlookConfig::addSyncSourceConfig(WindowsSyncSourceConfig& wsc) {

    unsigned int i = 0;
    WindowsSyncSourceConfig* s = NULL;

    // Copy array in a tmp buffer
    if (winSourceConfigsCount>0) {
        s = new WindowsSyncSourceConfig[winSourceConfigsCount];
        for (i=0; i<winSourceConfigsCount; i++) {
            s[i] = winSourceConfigs[i];
        }
    }

    // Delete old one, create new (+1 element)
    if (winSourceConfigs) {
        delete [] winSourceConfigs;
    }
    winSourceConfigs = new WindowsSyncSourceConfig[winSourceConfigsCount+1];

    // Copy back.
    for (i=0; i<winSourceConfigsCount; i++)
        winSourceConfigs[i] = s[i];
    // Copy the new one.
    winSourceConfigs[winSourceConfigsCount] = wsc;

    if (s) {
        delete [] s;
        s = NULL;
    }

    winSourceConfigsCount ++;
    return TRUE;
}



void OutlookConfig::setWorkingDir(const char* v) {
    if (v) {
        if (workingDir) {
            delete [] workingDir;
        }
        workingDir = stringdup(v);
    }
}
const char* OutlookConfig::getWorkingDir() const {
    return workingDir;
}

void OutlookConfig::setLogDir(const char* v) {
    if (v) {
        if (logDir) {
            delete [] logDir;
        }
        logDir = stringdup(v);
    }
}
const char* OutlookConfig::getLogDir() const {
    return logDir;
}

void OutlookConfig::setFullSync(const bool v) {
    fullSync = v;
}
const bool OutlookConfig::getFullSync() const {
    return fullSync;
}

/**
 * Save the value to win registry (HKCU), because it can 
 * be required from a different instance of plugin.
 */
void OutlookConfig::setScheduledSync(const bool v) {
    
    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    char value[2];
    sprintf(value, "%d", v);

    // Save value.
    dmt = DMTreeFactory::getDMTree(APPLICATION_URI);
    if (!dmt) return;
    node = dmt->readManagementNode(APPLICATION_URI);
    if (!node) return;
    node->setPropertyValue(PROPERTY_SCHEDULED_SYNC, value);

    delete dmt;
    delete node;
}
/**
 * Retrieve from win registry (HKCU).
 */
const bool OutlookConfig::getScheduledSync() const {

    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    // Get value.
    dmt = DMTreeFactory::getDMTree(APPLICATION_URI);
    if (!dmt)   return false;
    node = dmt->readManagementNode(APPLICATION_URI);
    if (!node)  return false;
    char* value = node->readPropertyValue(PROPERTY_SCHEDULED_SYNC);
    delete dmt;
    delete node;
    if (!value) return false;

    bool ret = false;
    if (!strcmp(value, "1")) {
        ret = true;
    }
    if (value) {
        delete [] value;
    }
    return ret;
}

void OutlookConfig::setAbortSync(const bool v) {
    abortSync = v;
}
const bool OutlookConfig::getAbortSync() const {
    return abortSync;
}


/**
 * Returns a pointer to the currentTimezone internal structure.
 */
const TimeZoneInformation* OutlookConfig::getCurrentTimezone() const {
    return &currentTimezone;
}




// ------------------------------ Other methods ----------------------------------
/**
 * Creates a default configuration object.
 * Uses DefaultWinConfigFactory methods to populate config objects.
 */
void OutlookConfig::createDefaultConfig() {

    //
    // AccessConfig
    //
    AccessConfig* ac = DefaultWinConfigFactory::getAccessConfig();
    setAccessConfig(*ac);
    delete ac;

    //
    // DeviceConfig
    //
    DeviceConfig* dc = DefaultWinConfigFactory::getDeviceConfig();
    setDeviceConfig(*dc);
    delete dc;


    // Set a unique deviceID = "FOL-<pcName>:<userName>"
    setUniqueDevID();


    //
    // SyncSourceConfigs: create both 'sourceConfig' and 'winSourceConfig' arrays.
    //
    // NOTE: if config not existing for some sources, it will be created inside
    //       'setSyncSourceConfig()'. First we need to set all 'sourceConfig' array and
    //       then 'winSourceConfig' linking each object to the original SyncSourceConfig 
    //       object (inside constructor of WindowsSyncSourceConfig).
    // NOTE: create sources alphabetically sorted, because this will be the order of 
    //       nodes inside Win registry (and they must match)!
    WCHAR* sourceNames[5] = {APPOINTMENT, CONTACT, NOTE, PICTURE, TASK};
    for (int i=0; i<5; i++) {
        WCHAR* wname = sourceNames[i];
        SyncSourceConfig* sc = DefaultWinConfigFactory::getSyncSourceConfig(wname);
        DMTClientConfig::setSyncSourceConfig(*sc);
        delete sc;
    }
    for (int i=0; i<5; i++) {
        WCHAR* wname = sourceNames[i];
        char*   name = toMultibyte(wname);

        try {
            SyncSourceConfig* sc = DMTClientConfig::getSyncSourceConfig(name);
            WindowsSyncSourceConfig* wsc = DefaultWinConfigFactory::getWinSyncSourceConfig(wname, sc);
            setSyncSourceConfig(*wsc);
            delete wsc;
        }
        catch (char* e) {
            setErrorF(getLastErrorCode(), ERR_DEFAULT_SSCONFIG, name, e);
            safeMessageBox(getLastErrorMsg());
        }

        if (name) {
            delete name;
        }
    }

    
    // Reset flags
    fullSync  = false;
    abortSync = false;


    // Current working dir: read 'installDir' from HKLM
    char* installPath = readPropertyValue(APPLICATION_URI, PROPERTY_INSTALLDIR, HKEY_LOCAL_MACHINE);
    if (!installPath || strlen(installPath) == 0) {
        LOG.error(ERR_INSTALL_DIR);
        return;
    }
    setWorkingDir(installPath);


    // Current data dir: for LOG file and temporary files (no win registry).
    WCHAR* wlogPath = readAppDataPath();
    if (!wlogPath) {
        LOG.error(getLastErrorMsg());
        return;
    }
    char* logPath = toMultibyte(wlogPath);
    setLogDir(logPath);


    //
    // Update properties for portal build:
    // -----------------------------------
    //if (checkPortalBuild()) {

        // Set different url/userName/password.
        accessConfig.setSyncURL  (PORTAL_DEFAULT_SYNCURL);
        accessConfig.setUsername (PORTAL_DEFAULT_USERNAME);
        accessConfig.setPassword (PORTAL_DEFAULT_PASSWORD);

        // Tasks/Notes actually disabled on portal build.
    //    WindowsSyncSourceConfig* sc = NULL;
    //    sc = getSyncSourceConfig(NOTE_);
    //    if (sc)  sc->setSync("none");
    //}


    //
    // Also upgrade the config (swv / userAgent).
    //
    upgradeConfig();


    if (installPath) delete [] installPath;
    if (logPath)     delete [] logPath;
    if (wlogPath)    delete [] wlogPath;
}




/**
 * Checks whether the config has to be upgraded to a new version.
 * It checks the difference between:
 *   swv from HKLM: this is set by installer, current version
 *   swv from HKCU: previous version of this config
 *
 * @return  'true' if the config needs to be upgraded.
 *          'false' if no upgrade is necessary.
 */
bool OutlookConfig::checkToUpgrade() {

    bool ret = false;
    const char* newSwv = readCurrentSwv();
    const char* oldSwv = getClientConfig().getSwv();
    if (strcmp(oldSwv, newSwv)) {
        ret = true;
    }
    if (newSwv) delete [] newSwv;
    return ret;
}


/**
 * ---- Update config with values from HKLM (set by installer) ----
 * This is useful when the client has just been upgraded to a new version,
 * only some properties (like 'swv' and 'userAgent') must be corrected.
 */
void OutlookConfig::upgradeConfig() {

    // Backup old Swv.
    oldSwv = getBuildNumberFromVersion(getClientConfig().getSwv());

    // Set the new Swv.
    const char* newSwv = readCurrentSwv();
    getClientConfig().setSwv(newSwv);

    // Set the new User Agent = "Funambol Outlook Sync Client v. x.y.z"
    char* userAgent = new char[strlen(PROGRAM_NAME) + strlen(newSwv) + 5];
    sprintf(userAgent, "%s v. %s", PROGRAM_NAME, newSwv);
    accessConfig.setUserAgent(userAgent);


    // Old version < 6.6.0: upgrade supportedTypes and version for each source.
    //  vTodo and vNote are supported, SIF version has been added.
    if (oldSwv < 60600) {
        WindowsSyncSourceConfig* ssc = getSyncSourceConfig(CONTACT_);
        if (ssc) {
            ssc->setSupportedTypes("text/x-s4j-sifc:1.0,text/x-vcard:2.1");
            if (!strcmp(ssc->getType(), "text/x-s4j-sifc")) {
                ssc->setVersion("1.0");
            }
        }
        ssc = getSyncSourceConfig(APPOINTMENT_);
        if (ssc) {
            ssc->setSupportedTypes("text/x-s4j-sife:1.0,text/x-vcalendar:1.0");
            if (!strcmp(ssc->getType(), "text/x-s4j-sife")) {
                ssc->setVersion("1.0");
            }
        }
        ssc = getSyncSourceConfig(TASK_);
        if (ssc) {
            ssc->setSupportedTypes("text/x-s4j-sift:1.0,text/x-vcalendar:1.0");
            if (!strcmp(ssc->getType(), "text/x-s4j-sift")) {
                ssc->setVersion("1.0");
            }
        }
        ssc = getSyncSourceConfig(NOTE_);
        if (ssc) {
            ssc->setSupportedTypes("text/x-s4j-sifn:1.0,text/x-vnote:1.1");
            if (!strcmp(ssc->getType(), "text/x-s4j-sifn")) {
                ssc->setVersion("1.0");
            }
        }
    }

    // Old version < 7.1.1: add default filtering to events.
    if (oldSwv < 70101) {
        WindowsSyncSourceConfig* ssc = getSyncSourceConfig(APPOINTMENT_);
        if (ssc) {
            DateFilter& filter = ssc->getDateFilter();
            filter.setDirection(DateFilter::DIR_OUT);
            filter.setRelativeLowerDate(DateFilter::LAST_MONTH);
            filter.setUpperDate(NULL);
        }
    }

    // Old version < 7.1.2: only vCard is used for contacts.
    if (oldSwv < 70102) {
        WindowsSyncSourceConfig* ssc = getSyncSourceConfig(CONTACT_);
        if (ssc) {
			// Don't change the source URI. If we were using "scard", it will be preserved during
			// upgrade. This is required to keep the anchors Server side and avoid a 1st time slow-sync.
            //ssc->setURI("card");
            ssc->setType("text/x-vcard");
            ssc->setVersion("2.1");
            ssc->setEncoding("bin");
        }
    }

    // Old version < 7.1.4: Client name has changed.
    if (oldSwv < 70104) {
        DeviceConfig& dc = getClientConfig();
        dc.setMod(PROGRAM_NAME);
    }

        // Old version < 7.2.0: Pictures source added.
    if (oldSwv < 70200) {
        if (!addWindowsSyncSourceConfig(PICTURE)) {
            LOG.error("upgradeConfig - error adding the config for %s source", PICTURE_);
        }
    }

        
    // Set the flag to specify that config has been upgraded.
    upgraded = true;
    
    if (newSwv)    delete [] newSwv;
    if (userAgent) delete [] userAgent;
}


/**
 * Returns true if config has been upgraded from a previous version.
 */
bool OutlookConfig::isUpgraded() {
    return upgraded;
}

/**
 * Returns the old installed swv (for upgrades). 
 * '0' if not an upgrade.
 */
int OutlookConfig::getOldSwv() {
    return oldSwv;
}


/**
 * Returns the current software version, read it from HKLM registry.
 * This value is set and updated ONLY by installer.
 * Returns a new allocated buffer, must be deleted by the caller.
 */
char* OutlookConfig::readCurrentSwv() {
    return readPropertyValue(APPLICATION_URI, PROPERTY_SOFTWARE_VERSION, HKEY_LOCAL_MACHINE);
}




/**
 * Creates and set a unique 'devID' property for current configuration.
 * The devID depends on:
 *   %1: the local machine name
 *   %2: the Windows current user name
 *   %3: the current Outlook profile name (now disabled)
 *
 * If one of these parameters has changed, the devID generated is different.
 * The devID property is then set into current configuration, in
 * the format "fol-%1:%2:%3" where %1 %2 %3 are the parameters already described.
 * Parameters are also encoded in base64.
 *
 * @return  0 if no errors occurred
 */
int OutlookConfig::setUniqueDevID() {
    
    int ret = 0, len = 0;
    DWORD code = 0;
    char* msg  = NULL;
    DWORD bufSize = 128;
    char computerName[128], userName[128];
    //ClientApplication* ol;
    wstring wprofileName;

    //
    // NetBIOS name of the local computer.
    //
    if (!GetComputerNameA(computerName, &bufSize)) {
        code = GetLastError();
        msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_MACHINE_NAME, code, msg);
        delete [] msg;
        return 1;
    }
    len += bufSize;

    //
    // UserName of the owner of current thread.
    //
    bufSize = 128;
    if (!GetUserNameA(userName, &bufSize)) {
        code = GetLastError();
        msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_USER_NAME, code, msg);
        delete [] msg;
        return 1;
    }
    len += bufSize;

    //
    // Name of current Outlook profile used (SHOULD be logged in!) 
    // ------------------ disabled by now -----------------------
    //try {
    //    ol = ClientApplication::getInstance();
    //    wprofileName = ol->getCurrentProfileName();
    //}
    //catch (ClientException* e) {
    //    manageClientException(e);
    //    ret = 1;
    //}
    //char* profileName = toMultibyte(wprofileName.c_str());
    //len += strlen(profileName);


    //
    // compose devID property -> encode to b64.
    //
    char* id = new char[len+2];
    sprintf(id, "%s:%s", computerName, userName);
    len = strlen(id);
    char* enc = new char[(len/3 + 1)*4 + 1];
    len = b64_encode(enc, id, len);
    enc[len] = 0;

    char* devID = new char[len+5];
    sprintf(devID, "fol-%s", enc);


    // Set it to configuration.
    getClientConfig().setDevID(devID);
    LOG.info(INFO_CONFIG_DEVID_SAVED, devID);


    //if (profileName)  delete [] profileName;
    if (id)           delete [] id;
    if (enc)          delete [] enc;
    if (devID)        delete [] devID;

    return ret;
}



/*
 * Returns the value of the given property, from rootKey tree (read only).
 * The value is returned as a new char array and must be freed by the user.
 *
 * @param context      : full context of key, under rootKey
 * @param propertyName : name of property to retrieve
 * @param rootKey      : one of  HKEY_LOCAL_MACHINE
 *                               HKEY_CLASSES_ROOT
 *                               HKEY_CURRENT_USER
 *                               HKEY_USERS
 *                               ...
 * @return               the property value (new allocated buffer) - 
 *                       if key not found, returns an empty string.
 */
char* OutlookConfig::readPropertyValue(const char* context, const char* propertyName, HKEY rootKey) {
    
    DWORD res = 0;  	
    long  err = 0;
    ULONG dim = 0;
    HKEY  key = NULL;
    char* ret = NULL;

    // Need to convert all '/' into '\'.
    char* fullContext = new char[strlen(context) + 10];
    sprintf(fullContext, "%s/%s", "Software", context);
    toWindows(fullContext);

    RegCreateKeyExA(
            rootKey,
            fullContext,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,                           // Read only: could be from a limited rights user.
            NULL,
            &key,
            &res
            );

    if (key == 0) {
        setErrorF(ERR_INVALID_CONTEXT, ERR_INVALID_REG_PATH, fullContext);
        goto finally;
    }

    // Get value length
    err = RegQueryValueExA(
            key,
            propertyName,
            NULL,
            NULL,  // we currently support only strings
            NULL,
            &dim
            );

    if (err == ERROR_SUCCESS) {
		if (dim > 0) {
            char* buf = new char[dim + 1];

			err = RegQueryValueExA(
					key,
					propertyName,
					NULL,
					NULL,  // we currently support only strings
					(UCHAR*)buf,
					&dim 
                    );
            if (err == ERROR_SUCCESS) {
                ret = stringdup(buf);
            }
            delete [] buf;
		}
    }

finally:

    if (!ret) {
        // Always return an empty string if key not found!
        ret = stringdup(EMPTY_STRING);
    }
    if (fullContext) {
        delete [] fullContext;
    }
    if (key != 0) {
        RegCloseKey(key);
    }
    return ret;
}



/// Check if it's a normal/portal build (from HKLM keys).
bool OutlookConfig::checkPortalBuild() {
    
    bool ret = false;

    char* portal = readPropertyValue(APPLICATION_URI, PROPERTY_SP, HKEY_LOCAL_MACHINE);
    if (portal && *portal == '1') {
        ret = true;
    }

    if (portal) delete [] portal;
    return ret;
}



/**
 * Decrypt private data (Username/Password/Proxy username/Proxy password).
 * Data is stored encrypted (B64(DES(data)) since version 6.0.9.
 * Data must be in clear text into the config, as config is used by API.
 *
 * @return  0 if data decrypted.
 *          1 if no decryption was necessary (data not encrypted before v.6.0.9).
 */
int OutlookConfig::decryptPrivateData() {

    // Check if previous version is < 6.0.9.
    const char* installedSwv = getClientConfig().getSwv();
    int version = getBuildNumberFromVersion(installedSwv);
    
    // to handle the new upgrade to the new key with the new password for portal
    // if it is empty then do nothing, otherwise it rewrites the values 
    // This modification is introduced in the 7.0.1 version    
    StringBuffer pass_key = PASS_KEY;    
    if (version < 60009) {
        // No decryption is necessary.
        return 1;
    } else if (version < 70001 && (checkPortalBuild() || CARED_KEY)) {
        pass_key = NULL;        
    }

    // Username
    char* decData = decryptData(accessConfig.getUsername(), pass_key);
    if (decData) {
        accessConfig.setUsername(decData);
        delete [] decData; decData = NULL;
    }
    // Password
    decData = decryptData(accessConfig.getPassword(), pass_key);
    if (decData) {
        accessConfig.setPassword(decData);
        delete [] decData; decData = NULL;
    }
    // Proxy username
    decData = decryptData(accessConfig.getProxyUsername(), pass_key);
    if (decData) {
        accessConfig.setProxyUsername(decData);
        delete [] decData; decData = NULL;
    }
    // Proxy password
    decData = decryptData(accessConfig.getProxyPassword(), pass_key);
    if (decData) {
        accessConfig.setProxyPassword(decData);
        delete [] decData; decData = NULL;
    }
    return 0;
}


/**
 * Encrypt private data (Username/Password/Proxy username/Proxy password).
 * Data is stored encrypted (B64(DES(data)) since version 6.0.9.
 * Note:
 * Data must be in clear text into the config, as config is used by API, 
 * so this method should be called only during save() operation.
 */
void OutlookConfig::encryptPrivateData() {

    // Username
    char* encData = encryptData(accessConfig.getUsername(), PASS_KEY);
    if (encData) {
        accessConfig.setUsername(encData);
        delete [] encData; encData = NULL;
    }
    // Password
    encData = encryptData(accessConfig.getPassword(), PASS_KEY);
    if (encData) {
        accessConfig.setPassword(encData);
        delete [] encData; encData = NULL;
    }
    // Proxy username
    encData = encryptData(accessConfig.getProxyUsername(), PASS_KEY);
    if (encData) {
        accessConfig.setProxyUsername(encData);
        delete [] encData; encData = NULL;
    }
    // Proxy password
    encData = encryptData(accessConfig.getProxyPassword(), PASS_KEY);
    if (encData) {
        accessConfig.setProxyPassword(encData);
        delete [] encData; encData = NULL;
    }
}

