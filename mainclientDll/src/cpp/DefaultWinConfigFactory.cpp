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

#include "base/util/utils.h"
#include "winmaincpp.h"
#include "customization.h"
#include "DefaultWinConfigFactory.h"
#include "DateFilter.h"
#include <string>

using namespace std;


DefaultWinConfigFactory::DefaultWinConfigFactory() : DefaultConfigFactory() {
}

DefaultWinConfigFactory::~DefaultWinConfigFactory() {
}


AccessConfig* DefaultWinConfigFactory::getAccessConfig() {

    AccessConfig* ac = new AccessConfig();

    ac->setUsername             (DEFAULT_USERNAME);
    ac->setPassword             (DEFAULT_PASSWORD);
    ac->setSyncURL              (DEFAULT_URL);
    ac->setFirstTimeSyncMode    (SYNC_NONE);
    ac->setUseProxy             (FALSE);
    ac->setProxyHost            ("");
    ac->setProxyPort            (8080);
    ac->setProxyUsername        ("");
    ac->setProxyPassword        ("");
    ac->setBeginSync            (0);
    ac->setEndSync              (0);
    ac->setServerAuthRequired   (FALSE);
    ac->setClientAuthType       ("syncml:auth-basic");
    ac->setServerAuthType       ("syncml:auth-basic");
    ac->setServerPWD            ("funambol");
    ac->setServerID             ("funambol");
    ac->setServerNonce          ("");
    ac->setClientNonce          ("");
    ac->setMaxMsgSize           (MAX_SYNCML_MSG_SIZE);                  // 125kB = ~50 contacts
    ac->setReadBufferSize       (0);
    ac->setUserAgent            (PROGRAM_NAME);                         // This is replaced during config.upgradeConfig()
    ac->setCheckConn            (FALSE);
    ac->setResponseTimeout      (RESPONSE_TIMEOUT);                     // [seconds] timeout on server = 15 min
	ac->setCompression			(ENABLE_COMPRESSION);
    return ac;
}



DeviceConfig* DefaultWinConfigFactory::getDeviceConfig() {

    DeviceConfig* dc = new DeviceConfig();

    dc->setMan                  ("Funambol");
    dc->setMod                  (PROGRAM_NAME);
    dc->setOem                  ("");
    dc->setFwv                  ("");
    dc->setSwv                  ("");                                   // This is replaced during config.upgradeConfig()
    dc->setHwv                  ("");
    dc->setDevID                ("fol-default");                        // This is generated unique with config.setUniqueDevID()
    dc->setDevType              ("workstation");
    dc->setDsV                  ("");
    dc->setUtc                  (TRUE);
    dc->setLoSupport            (TRUE);
    dc->setNocSupport           (TRUE);
    dc->setLogLevel             (LOG_LEVEL_INFO);
    dc->setMaxObjSize           (0);
    dc->setDevInfHash           ("");

    return dc;
}


// *** TODO: get sif/vCard version and types from API (vocl/winItem.h) ***
SyncSourceConfig* DefaultWinConfigFactory::getSyncSourceConfig(const wstring& wname) {

    SyncSourceConfig* sc = new SyncSourceConfig();
    char* name = toMultibyte(wname.c_str());

    sc->setName                 (name);
    sc->setSyncModes            ("slow,two-way,one-way-server,one-way-client,refresh-server,refresh-client");
    sc->setLast                 (0);
    sc->setSync                 ("two-way");
    sc->setEncryption           ("");

    if (wname == CONTACT){
        // since 7.1.2: default is vCard. Both still supported (for backw compaibility).
        sc->setURI              ("card");
        sc->setType             ("text/x-vcard");
        sc->setVersion          ("2.1");
        sc->setEncoding         ("bin");
        sc->setSupportedTypes   ("text/x-s4j-sifc:1.0,text/x-vcard:2.1");
        sc->setIsEnabled        (CONTACT_SOURCE_ENABLED);
    }
    else if (wname == APPOINTMENT){
        sc->setURI              ("event");
        sc->setType             ("text/x-vcalendar");
        sc->setVersion          ("1.0");
        sc->setEncoding         ("bin");
        sc->setSupportedTypes   ("text/x-vcalendar:1.0,text/x-s4j-sife:1.0");
        sc->setIsEnabled        (APPOINTMENT_SOURCE_ENABLED);
    }
    else if (wname == TASK){
        sc->setURI              ("task");
        sc->setType             ("text/x-vcalendar");
        sc->setVersion          ("1.0");
        sc->setEncoding         ("bin");
        sc->setSupportedTypes   ("text/x-vcalendar:1.0,text/x-s4j-sift:1.0");
        sc->setIsEnabled        (TASK_SOURCE_ENABLED);
    }
    else if (wname == NOTE){
        sc->setURI              ("snote");
        sc->setType             ("text/x-s4j-sifn");
        sc->setVersion          ("1.0");
        sc->setEncoding         ("b64");
        sc->setSupportedTypes   ("text/x-s4j-sifn:1.0,text/x-vnote:1.1");
        sc->setIsEnabled        (NOTE_SOURCE_ENABLED);
    }
    else if (wname == PICTURE){
        sc->setURI              ("picture");
        sc->setType             ("application/vnd.omads-file+xml");      // not really used, as it's detected from each item received
        sc->setVersion          ("");
        sc->setEncoding         ("bin");                                 // not really used, as it's detected from each item received
        sc->setSupportedTypes   ("application/vnd.omads-file+xml:,application/*:");
        sc->setSync             ("one-way-from-server");                // FIXED for pictures
        sc->setIsEnabled        (PICTURE_SOURCE_ENABLED);
    }

    if (name) delete [] name;
    return sc;
}




WindowsSyncSourceConfig* DefaultWinConfigFactory::getWinSyncSourceConfig(const wstring& wname, SyncSourceConfig* sc) {

    WindowsSyncSourceConfig* wsc = new WindowsSyncSourceConfig(sc);

    wsc->setUseSubfolders       (true);
    wsc->setFolderPath          ("");
    wsc->setEndTimestamp        (0);

    // Date filtering, set defaults.
    if (wname == APPOINTMENT) {
        DateFilter& filter = wsc->getDateFilter();
        filter.setDirection(DateFilter::DIR_OUT);
        filter.setRelativeLowerDate(DateFilter::LAST_MONTH);
        filter.setUpperDate(NULL);
    }

    return wsc;
}

