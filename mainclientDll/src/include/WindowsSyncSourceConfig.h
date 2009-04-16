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

#ifndef INCL_WINSYNC_SOURCE_CONFIG
#define INCL_WINSYNC_SOURCE_CONFIG

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "spds/SyncSourceConfig.h"
#include "DateFilter.h"

using namespace Funambol;

/**
 ***************************************************************************
 * This class groups all configuration properties for a WindowsSyncSource.
 * WindowsSyncSourceConfig has an external link to SyncSourceConfig ('s'), where all common
 * properties are stored. Client-specific properties of WindowsSyncSource are members
 * of this class.
 * (see spds/SyncSourceConfig.h for common members)
 ***************************************************************************
 */
class WindowsSyncSourceConfig {

private:

    /// The full path of correspondent Outlook folder.
    char* folderPath;

    /// true if the correspondent Outlook folder will be synced with
    /// all subfolders included.
    bool  useSubfolders;

    /// 'true' if the source has been synced (TODO: use the SOURCE_STATE).
    bool isSynced;

    /// Timestamp of the last finished sync.
    long endTimestamp;


    /// Pointer to (external) original SyncSourceConfig object, to retrieve
    /// all common properties: we MUST get/set common properties from 
    /// a unique place, so get/set methods for common properties are overrided
    /// and linked to  methods of original SyncSourceConfig 's'.
    SyncSourceConfig* s;

    /**
     * The filter on events startDate and endDate.
     * It's filled by OutlookConfig when reading configuration.
     * Can be retrieved with getDateFilter() method.
     */
    DateFilter dateFilter;


    /// Initialize all members.
    void initialize();
    

public:

    /// Default constructor - please note that 's' pointer MUST be set!!
    WindowsSyncSourceConfig();
    ///Constructs a new WindowsSyncSourceConfig object.
    /// Initialize members and link the SyncSourceConfig pointer passed (mandatory not NULL).
    WindowsSyncSourceConfig(SyncSourceConfig* sc);

    /// Copy constructor
    WindowsSyncSourceConfig(const WindowsSyncSourceConfig& wsc);

     ///Destructor
    ~WindowsSyncSourceConfig();

    /// Operator =
    WindowsSyncSourceConfig& operator = (const WindowsSyncSourceConfig& wsc); 

    /// Assign the internal SyncSourceConfig* pointer.
    void setCommonConfig(SyncSourceConfig* sc);

    /// Return the pointer to external SyncSourceConfig object 
    /// used for common properties.
    SyncSourceConfig* getCommonConfig();


    //
    // ----------------------------- set/get methods -----------------------------
    //
    _declspec(dllexport) const char*   getFolderPath()     const;
    _declspec(dllexport) bool          getUseSubfolders()  const;
    _declspec(dllexport) long          getEndTimestamp()   const;

    _declspec(dllexport)void setFolderPath    (const char*   v);
    _declspec(dllexport)void setUseSubfolders (bool          v);
    _declspec(dllexport)void setEndTimestamp  (long          v);

    // Common properties: get original values in SSconfig 's'
    const char*   getName()           const     { return s->getName()          ; }
    const char*   getURI()            const     { return s->getURI()           ; }
    const char*   getSyncModes()      const     { return s->getSyncModes()     ; }
    const char*   getType()           const     { return s->getType()          ; }
    const char*   getSync()           const     { return s->getSync()          ; }
    const char*   getEncoding()       const     { return s->getEncoding()      ; }
    const char*   getVersion()        const     { return s->getVersion()       ; }
    const char*   getSupportedTypes() const     { return s->getSupportedTypes(); }
    //CTCap       getCtCap()          const     { return s->getCtCap()         ; }
    unsigned long getLast()           const     { return s->getLast()          ; }
    const char*   getEncryption()     const     { return s->getEncryption()    ; }
    const bool    isEnabled()         const     { return s->isEnabled()        ; }

    // Common properties: set original values in SSconfig 's'
    void setName          (const char*   v)     { s->setName(v)                ; }
    void setURI           (const char*   v)     { s->setURI(v)                 ; }
    void setSyncModes     (const char*   v)     { s->setSyncModes(v)           ; }
    void setType          (const char*   v)     { s->setType(v)                ; }
    void setSync          (const char*   v)     { s->setSync(v)                ; }
    void setEncoding      (const char*   v)     { s->setEncoding(v)            ; }
    void setVersion       (const char*   v)     { s->setVersion(v)             ; }
    void setSupportedTypes(const char*   v)     { s->setSupportedTypes(v)      ; }
    //void setCtCap       (CTCap         v)     { s->setCtCap(v)               ; }
    void setLast          (unsigned long v)     { s->setLast(v)                ; }
    void setEncryption    (const char*   v)     { s->setEncryption(v)          ; }
    void setIsEnabled     (const bool    v)     { s->setIsEnabled(v)           ; }


    bool getIsSynced() const;
    void setIsSynced(bool v);

    /// Returns a reference to DateFilter (internally owned).
    _declspec(dllexport) DateFilter& getDateFilter() { return dateFilter; }

};

/** @} */
/** @endcond */
#endif
