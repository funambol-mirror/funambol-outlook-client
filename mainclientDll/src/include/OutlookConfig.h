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

#include <string>


/// Client specific properties 
/// (see spdm/costants.h for common props)
#define PROPERTY_USE_SUBFOLDERS                 "useSubfolders"
#define PROPERTY_FOLDER_PATH                    "folderPath"
#define PROPERTY_SCHEDULED_SYNC                 "isScheduled"

/// This is stored in HKLM during install
#define PROPERTY_SP                             "portal"
#define PROPERTY_INSTALLDIR                     "installDir"

/// Path in DMTree
#define APPLICATION_URI                         "Funambol/OutlookClient"

/// Default portal settings
// leave te <blanks> as they are in the PORTAL_DEFAULT_SYNCURL definition
#define PORTAL_DEFAULT_SYNCURL                  "http://my.funambol.com/sync"
#define PORTAL_DEFAULT_USERNAME                 ""
#define PORTAL_DEFAULT_PASSWORD                 ""

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

    /// The structure with current timezone informations.
    TimeZoneInformation currentTimezone;

    /// Array of specific SSConfig.
    WindowsSyncSourceConfig* winSourceConfigs;

    /// Counter for winSourceConfigs array.
    /// (internal use, should be equal to 'sourceConfigsCount' member)
    unsigned int winSourceConfigsCount;



    void readWinSourceConfig(unsigned int i);
    void saveWinSourceConfig(unsigned int i);


    // Returns the value of the given property, from rootKey tree (read only).
    char* readPropertyValue(const char* context, const char* propertyName, HKEY rootKey);

    // Username/Password are stored encrypted (new since 6.0.9).
    int  decryptPrivateData();
    void encryptPrivateData();

    int readCurrentTimezone();


protected:

    /// Constructor
    OutlookConfig();


public:
    
    /// Method to get the sole instance of OutlookConfig
    static OutlookConfig* getInstance();

    /// Returns true if static instance is not NULL.
    static bool isInstantiated();

    /// Destructor
    ~OutlookConfig();


    // Override read/save methods of DMT (use specific winSourceConfig)
    bool read();
    bool save();
    bool save(SyncReport* report);


    /// Read all sources timestamps from win registry.
    void readSourcesTimestamps();


    /// Replace getSyncSourceConfig() of DMT (return specific winSourceConfig)
    _declspec(dllexport) WindowsSyncSourceConfig* getSyncSourceConfig(const char* name);
    WindowsSyncSourceConfig* getSyncSourceConfig(unsigned int i);

    // Replace setSyncSourceConfig() of SyncManagerConfig (set specific winSourceConfig)
    BOOL setSyncSourceConfig(WindowsSyncSourceConfig& wsc);
    BOOL addSyncSourceConfig(WindowsSyncSourceConfig& wsc);


    // get/set of internal members
    void setWorkingDir   (const char* v);
    void setLogDir       (const char* v);
    void setFullSync     (const  bool v);
    void setScheduledSync(const  bool v);
    void setAbortSync    (const  bool v);

    const bool  getScheduledSync() const;
    const bool  getAbortSync()     const;
    _declspec(dllexport) const char* getWorkingDir()    const;
    _declspec(dllexport) const char* getLogDir()        const;
    _declspec(dllexport) const bool  getFullSync()      const;

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

    /// Returns the current software version, read it from HKLM registry.
    char* readCurrentSwv();

    ///Creates and set a unique 'devID' property for current configuration.
    int setUniqueDevID();

    /// Check if it's a normal/portal build (from HKLM keys).
    bool checkPortalBuild();

    /// Save only "beginSync" property to win registry.
    void saveBeginSync();
    
    /// Save only "sync" properties of each source, to win registry.
    void saveSyncModes();
    /// Reads only "sync" properties of each source, to win registry.
    _declspec(dllexport) void readSyncModes();
};

/** @} */
/** @endcond */
#endif