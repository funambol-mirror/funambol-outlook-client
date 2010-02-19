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

/** @cond OLPLUGIN */
/** @addtogroup config */
/** @{ */


#ifndef INCL_OUTLOOKCONFIG
#define INCL_OUTLOOKCONFIG

#include "base/Log.h"
#include "spds/SyncReport.h"
#include "Client/DMTClientConfig.h"
#include "WindowsSyncSourceConfig.h"
#include "updater/UpdaterConfig.h"
#include <string>


/// Client specific properties 
/// (see spdm/costants.h for common props)
#define PROPERTY_USE_SUBFOLDERS                 "useSubfolders"
#define PROPERTY_FOLDER_PATH                    "folderPath"
#define PROPERTY_SCHEDULED_SYNC                 "isScheduled"
#define PROPERTY_SOURCE_ORDER                   "sourceOrder"

/// Filtering properties
#define PROPERTY_FILTER_DATE_LOWER              "filterDateLower"
#define PROPERTY_FILTER_DATE_UPPER              "filterDateUpper"
#define PROPERTY_FILTER_DATE_DIRECTION          "filterDateDirection"

/// This is stored in HKLM during install
#define PROPERTY_SP                             "portal"
#define PROPERTY_INSTALLDIR                     "installDir"
#define PROPERTY_FUNAMBOL_SWV                   "funambol_swv"
#define PROPERTY_CUSTOMER                       "Customer"

#define TIMEZONE_CONTEXT                       L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"

// it must be in the format
// #define PASS_KEY<4 blanks>NULL
#define PASS_KEY    NULL

// to use the pass key customized also if no portal
#define CARED_KEY    false

/// Timezone informations.
/// This is a more complete structure than 'TIME_ZONE_INFORMATION' because we need
/// a unique key value to recognize the timezones ('keyName').
typedef struct TimeZone {
    int           index;                 // Unique index of timezone
    std::wstring  keyName;               // Unique name of timezone (english)
    std::wstring  displayName;           // The display name
    bool          isDaylight;            // 'true' if currently under Daylight Saving Time (DST).
    LONG          bias;                  // The current bias for local time translation on this computer, in minutes.
    std::wstring  standardName;          // A description for standard time.
    SYSTEMTIME    standardDate;          // A SYSTEMTIME structure that contains a date and local time when the transition from daylight saving time to standard time occurs on this operating system.
    LONG          standardBias;          // The bias value to be used during local time translations that occur during standard time
    std::wstring  daylightName;          // A description for daylight saving time.
    SYSTEMTIME    daylightDate;          // A SYSTEMTIME structure that contains a date and local time when the transition from standard time to daylight saving time occurs on this operating system.
    LONG          daylightBias;          // The bias value to be used during local time translations that occur during daylight saving time.
} TimeZoneInformation;




/**
 *****************************************************************************
 * Represents the configuration of Outlook client.
 * This class is derivated from DMTClientConfig, which is derivate from
 * SyncMangerConfig.
 *******************************************************************************
 */
class OutlookConfig : public DMTClientConfig {

private:

    /// pointer to OutlookConfig instance
    static OutlookConfig* pinstance;

    char* workingDir;                           // The path of current working directory
    char* logDir;                               // The path of current log directory (under app data)
    bool  fullSync;                             // true if we are running a full sync (slow/refresh)
    bool  abortSync;                            // set to true when we want to (soft) abort the current sync
    bool  upgraded;                             // Flag to specify that we have upgraded the config.
    int   oldSwv;                               // Value of old software version installed (used during upgrades).
    StringBuffer funambolSwv;                   // The Funambol product sw version (can be different in branded clients).
    int   oldFunambolSwv;                       // The old Funambol product sw version value in case of upgrade.

    /// The structure with current timezone informations.
    TimeZoneInformation currentTimezone;

    /// Array of specific SSConfig.
    WindowsSyncSourceConfig* winSourceConfigs;

    /// Counter for winSourceConfigs array.
    /// (internal use, should be equal to 'sourceConfigsCount' member)
    unsigned int winSourceConfigsCount;

    /**
     * The list of sources visible in the Client's UI. Sources not listed
     * here are hidden to the user.
     * NOTE: "contact, calendar, task, note" cannot be hidden   *** TODO ***
     */
    ArrayList sourcesVisible;


    void readWinSourceConfig(unsigned int i);
    void saveWinSourceConfig(unsigned int i);


    // Returns the value of the given property, from rootKey tree (read only).
    char* readPropertyValue(const char* context, const char* propertyName, HKEY rootKey);

    // Username/Password are stored encrypted (new since 6.0.9).
    int  decryptPrivateData();
    void encryptPrivateData();

    int readCurrentTimezone();

    /**
     * Reads the 'sourceOrder' registry key and populates the sourcesVisible array.
     * The 'sourceOrder' value is a comma separated string of source names.
     * @param rootKey [OPTIONAL] the rootkey, default is HKEY_CURRENT_USER
     * @note  contacts,calendar,tasks,notes cannot be hidden for now
     */
    void readSourcesVisible(HKEY rootKey = HKEY_CURRENT_USER);

    /**
     * Reads the sourcesVisible array and saves the 'sourceOrder' registry key.
     * The 'sourceOrder' value is a comma separated string of source names.
     */
    void saveSourcesVisible();

    /**
     * Used to save a generic property into config (win registry, under HKCU node).
     * @param context  the full context (i.e. "Software/Funambol/OutlookClient/spds/syncml/DevDetail")
     * @param name     the property name
     * @param value    the property value to set
     */
    void savePropertyValue(const StringBuffer& context, const StringBuffer& name, const StringBuffer& value);

    /**
    * The parameters related to the update procedure
    */
    UpdaterConfig updaterConfig;

protected:

    /// Constructor
    OutlookConfig();

    // Replace setSyncSourceConfig() of SyncManagerConfig (set specific winSourceConfig)
    BOOL setSyncSourceConfig(WindowsSyncSourceConfig& wsc);
    BOOL addSyncSourceConfig(WindowsSyncSourceConfig& wsc);


public:
    
    /// Method to get the sole instance of OutlookConfig
    static OutlookConfig* getInstance();

    /// Returns true if static instance is not NULL.
    static bool isInstantiated();

    /// Destructor
    virtual ~OutlookConfig();


    // Override read/save methods of DMT (use specific winSourceConfig)
    bool read();
    bool save();
    bool save(SyncReport* report);


    /// Read all sources timestamps from win registry.
    void readSourcesTimestamps();


    /// Returns the ArrayList of sources visible.
    const ArrayList& getSourcesVisible();


    /// Replace getSyncSourceConfig() of DMT (return specific winSourceConfig)
    _declspec(dllexport) WindowsSyncSourceConfig* getSyncSourceConfig(const char* name);
    WindowsSyncSourceConfig* getSyncSourceConfig(unsigned int i);

    /**
     * Adds a new WindowsSyncSourceConfig to the array.
     * The config for the new source is generated by DefaultConfigFactory.
     * For this operation we have to delete and ricreate the winSourceConfigs array, it's
     * managed inside this method so it's hidden from outside.
     * TODO: rework the procedure to add a source (use ArrayList?)
     * @param sourceName the source name to add
     * @return true if no errors
     */
    bool addWindowsSyncSourceConfig(const std::wstring& sourceName);

    /**
     * Adds the passed source name to the sourcesVisible array, safely:
     * the source is not added if already exists in the array.
     * @return true if the element is added, false if not found
     */
    bool safeAddSourceVisible(const char* sourceName);

    /**
     * Removes the passed source name from the sourcesVisible array.
     * @return true if the source was found and removed, false if not found
     */
    bool removeSourceVisible(const char* sourceName);


    // get/set of internal members
    void setWorkingDir   (const char* v);
    void setLogDir       (const char* v);
    void setFullSync     (const  bool v);
    void setScheduledSync(const  bool v);
    void setAbortSync    (const  bool v);
    void setFunambolSwv  (const StringBuffer& v);

    const bool  getScheduledSync() const;
    const bool  getAbortSync()     const;
    _declspec(dllexport) const char* getWorkingDir()    const;
    _declspec(dllexport) const char* getLogDir()        const;
    _declspec(dllexport) const bool  getFullSync()      const;
    const StringBuffer& getFunambolSwv();


    const TimeZoneInformation* getCurrentTimezone() const;


    /// Creates a default configuration.
    void createDefaultConfig();

    /// Checks if the config is to upgrade.
    bool checkToUpgrade();

    /// Update config with values from HKLM (set by installer).
    void upgradeConfig();

    /// true if config has been upgraded from a previous version
    bool isUpgraded();

    /// Returns the old installed swv (for upgrades). '0' if not an upgrade.
    int getOldSwv();
    int getOldFunambolSwv();

    /// Returns the current software version, read it from HKLM registry.
    char* readCurrentSwv();

    /**
     * Returns the funambol product software version, read it from HKLM or HKCU registry.
     * For Funambol builds, this value is = swv.
     */
    StringBuffer readFunambolSwv(HKEY rootKey);

    ///Creates and set a unique 'devID' property for current configuration.
    int setUniqueDevID();

    /// Check if it's a normal/portal build (from HKLM keys).
    bool checkPortalBuild();

    /// Save only "beginSync" property to win registry.
    void saveBeginSync();

    /// Save the Funambol sw version to config ("_root_/syncML/devDetail/funambol_swv" key)
    void OutlookConfig::saveFunambolSwv();
    
    /// Save only "sync" properties of each source, to win registry.
    void saveSyncModes();

    /// Reads only "sync" properties of each source, to win registry.
    _declspec(dllexport) void readSyncModes();

    BOOL readUpdaterConfig(bool refresh);

    /**
    * Save data into the registry
    */
    void storeUpdaterConfig();

    UpdaterConfig& getUpdaterConfig();

};

/** @} */
/** @endcond */
#endif