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

#ifndef INCL_CLIENTFOLDER
#define INCL_CLIENTFOLDER

/** @cond OLPLUGIN */
/** @addtogroup outlook_items */
/** @{ */

#include "outlook/defs.h"
#include "outlook/ClientItem.h"
#include <string>

// Forward declarations for items
class ClientContact;
class ClientTask;
class ClientAppointment;
class ClientMail;
class ClientNote;


/**
 * Wraps the Outlook folder object.
 * Contains pointers of eventual items and folders inside
 * this folder.
 */
class ClientFolder {

private:

    std::wstring ID;                    /**< The object entry-ID. */
    std::wstring itemType;              /**< The item-type of all items contained in the folder (contact/task/mail...) */
    std::wstring name;
    std::wstring path;


    /// Subfolders for this folder.
    int subfoldersCount;
    int subfoldersIndex;

    ///Items inside folder.
    int itemsCount;
    int itemsIndex;


    /// COM Pointers to microsoft outlook objects.
    MAPIFolderPtr       pFolder;
    _FoldersPtr         pSubFolders;
    MAPIFolderPtr       pSubFolder;
    _ItemsPtr           pItems;
    IDispatchPtr        pItem;
    _ContactItemPtr     pContact;
    _AppointmentItemPtr pAppointment;
    _MailItemPtr        pMail;
    _NoteItemPtr        pNote;
    _TaskItemPtr        pTask;


    /// Internal Objects: 
    /// 'get...' methods always return references to these objects
    ClientFolder*       subFolder;
    ClientMail*         mail;
    ClientContact*      contact;
    ClientAppointment*  appointment;
    ClientTask*         task;
    ClientNote*         note;


    /// Result of COM pointers operations.
    HRESULT hr;

    /// Set the appropriate internal item, based on the item type.
    ClientItem* setInternalItem(IDispatchPtr& pItem);


public:

    /// Constructor
    ClientFolder();
    ClientFolder(ClientFolder& f);
    ClientFolder operator=(ClientFolder& f);

    /// Destructor
    ~ClientFolder();


    /// Set a COM pointer to this object: refresh all members.
    void setCOMPtr(MAPIFolderPtr& f, const std::wstring& type);
    void setCOMPtr(MAPIFolderPtr& f);

    /// Returns a reference to the internal COM pointer.
    MAPIFolderPtr& getCOMPtr();


    const std::wstring& getID();
    const std::wstring& getType();
    const std::wstring& getName();
    const std::wstring& getPath();


    //
    // Methods to menage subfolders.
    //
    const int getSubfoldersCount();
    const int getSubfoldersIndex();

    ClientFolder* getFirstSubfolder();
    ClientFolder* getNextSubfolder();
    ClientFolder* getPreviousSubfolder();
    ClientFolder* getLastSubfolder();
    ClientFolder* getSubfolder(const int index);
    ClientFolder* getSubfolderFromName(const std::wstring& subName);

    ClientFolder* addSubFolder(const std::wstring& subName, const std::wstring& type);


    //
    // Methods to menage items.
    //
    const int getItemsCount();
    const int getItemsIndex();  

    ClientItem* getFirstItem();
    ClientItem* getNextItem();
    ClientItem* getPreviousItem();
    ClientItem* getLastItem();
    ClientItem* getItem(const int index);

    ClientItem* addItem();

    void deleteFolder();
};

/** @} */
/** @endcond */
#endif
