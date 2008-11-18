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

#ifndef INCL_CLIENT_EXC
#define INCL_CLIENT_EXC

/** @cond OLPLUGIN */
/** @addtogroup outlook */
/** @{ */

#include "base/fscapi.h"
#include "base/errors.h"

#include <string>


/**
 ******************************************************************
 * Defines errors for Client operations.
 ******************************************************************
*/
class ClientException {

private:

    /// pointer to ClientException instance
    static ClientException* pinstance;


    /// The message to display
    char*  errorMsg;
    /// The same message in WCHAR
    WCHAR* werrorMsg;

    /// The error code
    DWORD errorCode;

    /// is this error critic for the application?
    bool fatal;

    /// do we need a messageBox displayed?
    bool useMessageBox;


protected:

    /// Constructor
    ClientException();


public:

    /// Method to create the sole instance of ClientException
    static ClientException* getInstance();
    
    /// Destructor
    ~ClientException();


    /// Set data of exception
    void setExceptionData(const char* msg,
                          DWORD code       = NULL, 
                          bool fatalError  = false, 
                          bool needDisplay = false);


    /// Clear data of exception
    void clear();


    /// Methods to get exception informations
    const char*         getErrorMsg();
    const WCHAR*        wgetErrorMsg();
    const std::wstring  wstrgetErrorMsg();

    DWORD  getErrorCode();
    bool   isFatal();
    bool   needMessageBox();

};


// ---------- Global functions -----------

/// use these functions to throw Client exceptions.
void throwClientException(const char* msg, 
                           DWORD code       = NULL, 
                           bool fatalError  = NULL,
                           bool needDisplay = NULL);

/// To throw a fatal outlook exception.
void throwClientFatalException(const char* msg, 
                                DWORD code = NULL);

/// Actions to execute when a Client exception occurs.
void manageClientException(ClientException* e);

/** @} */
/** @endcond */
#endif
