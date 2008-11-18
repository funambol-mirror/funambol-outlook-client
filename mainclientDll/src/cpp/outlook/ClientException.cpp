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

#include "base/Log.h"
#include "base/util/utils.h"
#include "winmaincpp.h"
#include "utils.h"
#include "outlook/ClientException.h"
#include "outlook/defs.h"
#include "SyncException.h"

#include <string>

using namespace std;

// Init static pointer.
ClientException* ClientException::pinstance = NULL;


//------------------------------- Static functions --------------------------------------

/*
 * Throw a client exception.
 * Get the instance of ClientException, set the data and throw it.
 */
void throwClientException(const char* msg, DWORD code, bool fatalError, bool needDisplay) {

    ClientException* e = ClientException::getInstance();
    e->setExceptionData(msg, code, fatalError, needDisplay);
    throw e;
}


/* 
 * Used to throw a fatal exception.
 * (with members: fatal = true, useMessageBox = true)
 */
void throwClientFatalException(const char* msg, DWORD code) {

    ClientException* e = ClientException::getInstance();
    e->setExceptionData(msg, code, true, true);
    throw e;
}



/*
 * Actions to execute when a Client exception occurs.
 */
void manageClientException(ClientException* e) {

    setError(e->getErrorCode(), e->getErrorMsg());
    
    if (e->isFatal()) {
        LOG.error(ERR_OUTLOOK_FATAL_EXCEPTION, e->getErrorMsg());
    }
    else {
        LOG.error(ERR_OUTLOOK_EXCEPTION, e->getErrorMsg());
    }

    if (e->needMessageBox()) {
        safeMessageBox(e->getErrorMsg(), ERR_OUTLOOK);
    }

    if (e->isFatal()) {

        endSync();
        // (Code 3 = client fatal exception)
        throwSyncException(e->getErrorMsg(), 3);
    }
}




//--------------------------------- Class Methods ---------------------------------------

// Method to create the sole instance of ClientException
ClientException* ClientException::getInstance() {
    if (pinstance == NULL) {
        pinstance = new ClientException;
    }
    return pinstance;
}


// Constructor
ClientException::ClientException() {
    errorMsg  = NULL;
    werrorMsg = NULL;
    clear();
}


// Destructor
ClientException::~ClientException() {

    if (errorMsg) {
        delete []  errorMsg;  errorMsg = NULL;
    }
    if (werrorMsg) {
        delete [] werrorMsg; werrorMsg = NULL;
    }
}



// Set members of exception
void ClientException::setExceptionData(const char* msg, DWORD code, bool fatalError, bool needDisplay) {

    if (msg) {
        if (errorMsg) {
            delete []  errorMsg;  errorMsg = NULL;
        }
        if (werrorMsg) {
            delete [] werrorMsg; werrorMsg = NULL;
        }
        errorMsg  = stringdup(msg);
        werrorMsg = toWideChar(msg);
    }
    errorCode = code;
    fatal = fatalError;
    useMessageBox = needDisplay;
}



// Clear data of exception
void ClientException::clear() {

    if (errorMsg) {
        delete []  errorMsg;  errorMsg = NULL;
    }
    if (werrorMsg) {
        delete [] werrorMsg; werrorMsg = NULL;
    }
    errorMsg      = stringdup("");
    werrorMsg     = wstrdup (L"");
    errorCode     = NULL;
    fatal         = false;
    useMessageBox = false;
}




//
// Methods to get exception informations
//
const char* ClientException::getErrorMsg() {
    return errorMsg;
}

const WCHAR* ClientException::wgetErrorMsg() {
    return werrorMsg;
}

const wstring ClientException::wstrgetErrorMsg() {
    wstring ws = werrorMsg;
    return ws;
}



DWORD ClientException::getErrorCode() {
    return errorCode;
}

bool ClientException::isFatal() {
    return fatal;
}

bool ClientException::needMessageBox() {
    return useMessageBox;
}
