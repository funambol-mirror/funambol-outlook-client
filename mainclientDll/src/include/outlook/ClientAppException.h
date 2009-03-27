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

#ifndef INCL_CLIENTAPPEXCEPTION
#define INCL_CLIENTAPPEXCEPTION

/** @cond OLPLUGIN */
/** @addtogroup outlook_rec */
/** @{ */

#include "outlook/defs.h"

#include <string>



/**
 * Wraps the Outlook appointment exception object.
 * To get appointment exceptions (READ) call 'read()' and use 'getException(index)' 
 * methods of ClientRecurrence and browse through the list of exceptions.
 * To set an appointment exception (WRITE) set all properties and then call
 * 'saveAllExceptions()' method of ClientRecurrence.
*/
class ClientAppException {

private:

    // Pointer to microsoft outlook objects.
    ExceptionPtr          pException;                   /**< the exception */
    _AppointmentItemPtr   pAppointment;                 /**< the occurrence of the exception (an appointment item) */


    // Occurrence properties:
    std::wstring  subject;
    std::wstring  body;
    std::wstring  location;
    std::wstring  start;
    std::wstring  end;
    std::wstring  allDayEvent;
    std::wstring  busyStatus;
    std::wstring  reminderSet;
    std::wstring  reminderMinutesBeforeStart;
    std::wstring  importance;

    // Exception properties:
    DATE          originalDate;
    BOOL          deleted;


    /// Internal use: true if values are updated with Outlook.
    /// Used because all values need to be retrieved together, so this flag
    /// notifies when it's necessary to update properties.
    bool isUpdated;

    /// Internal use: true if this exception has been saved to Outlook.
    bool saved;


public:

    /// Constructor
    ClientAppException();

    /// Destructor
    ~ClientAppException();


    /// Set COM pointers to this object.
    /// One of these two method MUST be called before using the class
    void setCOMPtr(ExceptionPtr& ptr);
    void setCOMPtr(_AppointmentItemPtr& ptr);


    /// Read ALL properties from Outlook.
    int read();
    /// Save the exception to Outlook: save the modified occurrence.
    int saveOccurrence();
    /// Save the exception to Outlook: delete the occurrence.
    int deleteOccurrence();


    /// Returns true if this exception has already been saved.
    bool isSaved();


    //
    // Methods to manage the exception properties. 
    // -------------------------------------------
    //
    // get/set exception properties
    const BOOL getDeleted     ();
    const DATE getOriginalDate();
    void       setDeleted     (const BOOL);
    void       setOriginalDate(DATE val);


    /// To quickly get occurrence properties defined in 'exAppointmentFields[]'
    const std::wstring  getAppProperty(const std::wstring& propertyName);
    /// To quickly set occurrence properties defined in 'exAppointmentFields[]'
    int                 setAppProperty(const std::wstring& propertyName, const std::wstring& propertyValue);


    // get/set single occurrence properties
    const std::wstring   getSubject                   ();
    const std::wstring   getBody                      ();
    const std::wstring   getLocation                  ();
    const std::wstring   getStart                     ();
    const std::wstring   getEnd                       ();
    const std::wstring   getAllDayEvent               ();
    const std::wstring   getBusyStatus                ();
    const std::wstring   getReminderSet               ();
    const std::wstring   getReminderMinutesBeforeStart();
    const std::wstring   getImportance                ();

    void setSubject                   (const std::wstring& val);
    void setBody                      (const std::wstring& val);
    void setLocation                  (const std::wstring& val);
    void setStart                     (const std::wstring& val);
    void setEnd                       (const std::wstring& val);
    void setAllDayEvent               (const std::wstring& val);
    void setBusyStatus                (const std::wstring& val);
    void setReminderSet               (const std::wstring& val);
    void setReminderMinutesBeforeStart(const std::wstring& val);
    void setImportance                (const std::wstring& val);


    /// Returns "OriginalDate" property in string format.
    const std::wstring  formatOriginalDate(BOOL isAllDay, const std::wstring& start);
    const std::wstring  formatOriginalDate();

};

/** @} */
/** @endcond */
#endif
