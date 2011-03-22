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



#include "WindowsSyncSourceConfig.h"
#include "base/util/utils.h"
#include "winmaincpp.h"


WindowsSyncSourceConfig::WindowsSyncSourceConfig() {

    initialize();
    // Please note that 's' pointer MUST be set!!
}

WindowsSyncSourceConfig::WindowsSyncSourceConfig(SyncSourceConfig* sc) {

    initialize();

    // Link SyncSourceConfig pointer with common properties:
    if (!sc) {
        setError(getLastErrorCode(), "WindowsSyncSourceConfig initialized with a NULL pointer!");
        throw getLastErrorMsg();
    }
    s = sc;
	populateCTCap();
}

WindowsSyncSourceConfig::~WindowsSyncSourceConfig() {
    
}


// Return the pointer to external SyncSourceConfig object 
// used for common properties.
SyncSourceConfig* WindowsSyncSourceConfig::getCommonConfig() {
    return s;
}

_declspec(dllexport) const char* WindowsSyncSourceConfig::getFolderPath() const  {        
    return s->getProperty(PROPERTY_FOLDER_PATH);    
}
void WindowsSyncSourceConfig::setFolderPath(const char* v) {
    s->setProperty(PROPERTY_FOLDER_PATH, v);        
}

_declspec(dllexport) bool WindowsSyncSourceConfig::getUseSubfolders() const {
    bool err;
    bool ret = s->getBoolProperty(PROPERTY_USE_SUBFOLDERS, &err);
    if (err) {
        ret = false;
    }
    return ret;
}
void WindowsSyncSourceConfig::setUseSubfolders(bool v) {
    s->setBoolProperty(PROPERTY_USE_SUBFOLDERS, v);
}


bool WindowsSyncSourceConfig::getIsSynced() const {
    return isSynced;
}
void WindowsSyncSourceConfig::setIsSynced(bool v) {
    isSynced = v;
}



/**
 * Copy Constructor
 */
WindowsSyncSourceConfig::WindowsSyncSourceConfig(const WindowsSyncSourceConfig& wsc) {

    if (&wsc == this) {
        return;
    }

    initialize();

    // WARNING! pointer to the same object!
    s = wsc.s;

    setIsSynced(wsc.getIsSynced());
	populateCTCap();
}


/**
 * Operator =
 */
WindowsSyncSourceConfig& WindowsSyncSourceConfig::operator = (const WindowsSyncSourceConfig& wsc) {

    initialize();

    // WARNING! pointer to the same object!
    s = wsc.s;

    setIsSynced(wsc.getIsSynced());

    dateFilter = ((WindowsSyncSourceConfig&)wsc).getDateFilter();
	
	populateCTCap();

    return *this;
}



/**
 * Assign the internal SyncSourceConfig* pointer.
 */
void WindowsSyncSourceConfig::setCommonConfig(SyncSourceConfig* sc) {
    s = sc;
	populateCTCap();
}


bool WindowsSyncSourceConfig::getIsRefreshMode() const {
    bool result = false;
    const char * syncMode = getSync();
    if (syncMode) {
        result = !strcmp(syncMode, "refresh-from-client") || !strcmp(syncMode, "refresh-from-server");
    }

    return result;
}



/// Initialize members
void WindowsSyncSourceConfig::initialize() {

    s               = NULL;
    isSynced        = false;
}

void WindowsSyncSourceConfig::populateCTCap() {
	
    ArrayList* p = NULL;

	if (strcmp(s->getName(), CONTACT_) == 0) {    
        //adding vcard
        p = getVCardProperties();
        s->addCtCap(p, "text/x-vcard", "2.1");
        delete p; p = NULL;
    }
    if (strcmp(s->getName(), APPOINTMENT_) == 0) {       
        //adding vcalendar
        p = getVCalendarProperties();
        s->addCtCap(p, "text/x-vcalendar", "1.0");
        delete p; p = NULL;
    }
    if (strcmp(s->getName(), TASK_) == 0) {       
        //adding vTodo
        p = getVTodoProperties();
        s->addCtCap(p, "text/x-vcalendar", "1.0");
        delete p; p = NULL;
    }
    if (strcmp(s->getName(), NOTE_) == 0) {
		// sif
		p = getNoteProperties();
		s->addCtCap(p);
		p->clear();

		p = getVNoteProperties();
		s->addCtCap(p, "text/x-vnote", "1.1");        
        delete p; p = NULL;
    }
}
