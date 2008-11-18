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

#ifndef INCL_CLIENTAPPLICATION
#define INCL_CLIENTAPPLICATION

/** @cond OLPLUGIN */
/** @addtogroup outlook */
/** @{ */


#include "outlook/defs.h"
#include "outlook/ClientFolder.h"
#include "outlook/ClientItem.h"
#include "outlook/ClientMail.h"
#include "outlook/ClientContact.h"
#include "outlook/ClientAppointment.h"
#include "outlook/ClientTask.h"
#include "outlook/ClientNote.h"

#include <string>


/**
******************************************************************************
* The main class of Outlook wrapper, used to wrap Outlook Application
* instance, MAPI namespace and Redemption utility methods.
* Start from the unique instance of this class (it's a singleton) to get the
* desired ClientFolder, and the desired ClientItem.
* Class methods automatically catch and manage COM pointers exceptions.
* Class methods throw ClientException pointer in case of error.
******************************************************************************
*/
class ClientApplication {

private:

    /// pointer to ClientApplication instance
    static ClientApplication* pinstance;

    /// Version of Client used.
    std::wstring        version;
    /// Name of Client used.
    std::wstring        programName;


    // Pointers to microsoft outlook objects.
    _ApplicationPtr                   pApp;
    _NameSpacePtr                     pMAPI;
    MAPIFolderPtr                     pFolder;

    // Pointer to Redemption safe objects.
    Redemption::IMAPIUtilsPtr         pRedUtils;
    Redemption::IRDOSessionPtr        rdoSession;


    // Internal ClientObjects: 
    // 'get..()' methods always return references to these objects
    ClientFolder*       folder;
    ClientMail*         mail;
    ClientContact*      contact;
    ClientAppointment*  appointment;
    ClientTask*         task;
    ClientNote*         note;


    /// Result of COM pointers operations.
    HRESULT hr;

    void createSafeInstances();


protected:

    // Constructor
    ClientApplication();


public:

    // Method to get the sole instance of ClientApplication
    static ClientApplication* getInstance();

    // Returns true if static instance is not NULL.
    static bool isInstantiated();

    // Destructor
    ~ClientApplication();


    const std::wstring& getVersion();
    const std::wstring& getName();
    

    ClientFolder* getDefaultFolder     (const std::wstring& itemType);
    ClientFolder* getFolderFromID      (const std::wstring& folderID);
    ClientFolder* pickFolder           ();
    ClientFolder* pickFolder           (const std::wstring& itemType);
    ClientFolder* getFolderFromPath    (const std::wstring& itemType,   const std::wstring& path);
    ClientFolder* getDefaultRootFolder ();
    ClientFolder* getRootFolder        (const int index);
    ClientFolder* getRootFolderFromName(const std::wstring& folderName);

    ClientItem*   getItemFromID   (const std::wstring& itemID, const std::wstring& itemType);


    // Utility to release shared objects of Outlook session.
    HRESULT cleanUp();

    // Utility to convert an Exchange mail address into a SMTP address.
    std::wstring getSMTPfromEX(const std::wstring& EXAddress);

    // Utility to get body of a specified item (used for notes body which is protected).
    std::wstring getBodyFromID(const std::wstring& itemID);

    // Utility to retrieve the userName of current profile used.
    std::wstring getCurrentProfileName();

    // Returns true if Outlook MAPI object is logged on.
    const bool isLoggedOn();
};

/** @} */
/** @endcond */
#endif
