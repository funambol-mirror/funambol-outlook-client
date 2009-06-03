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

#ifndef INCL_CLIENTRECURRENCE
#define INCL_CLIENTRECURRENCE

/** @cond OLPLUGIN */
/** @addtogroup outlook_rec */
/** @{ */

#include "outlook/defs.h"
#include "outlook/ClientAppException.h"

#include <string>
#include <list>

typedef std::list<ClientAppException>     clientExceptionList;
typedef clientExceptionList::iterator     clientExceptionIterator;


/**
 **********************************************************************************
 * Wraps the Outlook recurrence object (for tasks and events).
 * This object is the same contained by ClientAppointment
 * and ClientTask objects.
 * Contains all recurrence properties, MUST read and save them all
 * together (read() and save()), so they're internally stored.
 *
 * A recurrence object has a list of appointment exceptions inside it.
 * - CLIENT to SERVER:
 *   Exceptions are populated into list during 'read()' method, each exception can
 *   be retrieved calling 'getException(index)'.
 * - SERVER to CLIENT:
 *   Exceptions can be appended to list calling 'addException()' method. Then all
 *   exceptions will be saved all together during 'ClientAppointment::save()'.
 **********************************************************************************
 */
class ClientRecurrence {

private:

    // Rec properties:
    int          recurrenceType;
    int          interval;
    int          monthOfYear;
    int          dayOfMonth;
    int          dayOfWeekMask;
    int          instance;
    std::wstring patternStartDate;
    BOOL         noEndDate;
    std::wstring patternEndDate;
    int          occurrences;
    
    std::wstring startTime;         // the start time of the recurrence
    std::wstring endTime;           // the end time of the recurrence
    
    bool         hasTimezone;       // the recurring appointment contains the
                                    // TIMEZONE_INFORMATION structure. The times
                                    // must remain in local time

    std::wstring start;             /**< for the change-day option */
    BOOL      isAllDay;             /**< for the change-day option */

    
    std::wstring end;               // for recurring appointment with timezone


    /// Pointer to microsoft outlook objects.
    RecurrencePatternPtr  pRec;

    /// Internal list of appointment exceptions.
    clientExceptionList appExceptions;


    /// Is this rec pattern active?
    /// this is linked to the property 'recurring' of Outlook event 
    /// (it's set at constructor and each time setting property IsRecurring)
    bool recurring;

    /// Internal use: true if values are updated with Outlook.
    /// Used because all values need to be retrieved together, so this flag
    /// notifies when it's necessary to update properties.
    bool isUpdated;

    /// Internal use: to avoid deadlocking into safeSaveException() recursive call.
    int numRecursions;



    /// @todo  Verify if some props are not correct for the rec type.
    void checkIfRecIsCorrect();

    /// Change-day of rec props, according on UTC <-> Local time
    bool changeDay(const std::wstring dest);


    //
    // Internal methods to manage exceptions.
    //
    /// used during 'read()' method (get data from client)
    ClientAppException* getExceptionOnClient(const int index);
    const int           getExceptionsCountOnClient();
    int                 saveException(ClientAppException* cException);
    //void              freeDestinationDays(const DATE startDate, const DATE originalDate);
    //int               safeSaveException(ClientAppException* cException);
    

    /**
    * The timezone information of the recurring appointment
    * It is used ONLY when the app is recurring
    */
    TIME_ZONE_INFORMATION timeZoneInfo;

public:

    /// Constructor
    ClientRecurrence();

    /// Destructor
    ~ClientRecurrence();

    /// Set a COM pointer to this object.
    void setCOMPtr(RecurrencePatternPtr& ptr);



    /**
     * Retrieve all properties from Outlook -> set isUpdated = true.
     * If UTC is used, all props are converted to correct values.
     * 
     * @return  0 if no errors
     */
    int read();
    
    /**
     * Refresh the values of the internal reference with the data
     * sent from the server.
     *
     * @return  0 if no errors
     */
    int refresh();

    /**
     * Save all the properties to Outlook.
     * Properties are verified to be consistent all together, 
     * as Outlook doesn't accept wrong values.
     * If UTC is used, all props are converted to correct values.
     * 
     * @return  0 if no errors
     */
    int save();



    /// Return true if the rec pattern is active.
    bool isRecurring();

    /// Set 'recurring' to true. 
    /// This is called by 'ClientAppointment.setProperty("IsRecurring", "1")'.
    void setRecurrence();

    /// Reset all members, set 'recurring' to false.
    /// This is called by 'ClientAppointment.setProperty("IsRecurring", "0")'.
    void clearRecurrence();



    //
    // Methods to get recurrence properties.
    // -------------------------------------
    // The first time a property is needed (isUpdated = false), all values 
    // are retrieved from Outlook calling 'read()'.
    //
    const int           getRecurrenceType  ();
    const int           getInterval        ();
    const int           getMonthOfYear     ();
    const int           getDayOfMonth      ();
    const int           getDayOfWeekMask   ();
    const int           getInstance        ();
    const std::wstring& getPatternStartDate();
    const BOOL          getNoEndDate       ();
    const std::wstring& getPatternEndDate  ();
    const int           getOccurrences     ();

    const std::wstring& getStartTime       ();
    const std::wstring& getEndTime         ();

    bool                getHasTimezone     () { return hasTimezone; }
    void                setHasTimezone     (bool v) { hasTimezone = v; }

    // Get property value (wstring) from its name.
    const std::wstring getProperty(const std::wstring& propertyName);


    //
    // Methods to set recurrence properties.
    // -------------------------------------
    // Values passed are stored in internal members, calling 'save()' all properties 
    // are put together into Outlook rec pattern (this is for UTC transformations).
    // No test on value content is done at this level.
    // 'isUpdated' is set to false, cause setting a property the rec pattern is no
    // more updated with Outlook.
    //
    void setRecurrenceType  (const int           val);
    void setInterval        (const int           val);
    void setMonthOfYear     (const int           val);
    void setDayOfMonth      (const int           val);
    void setDayOfWeekMask   (const int           val);
    void setInstance        (const int           val);
    void setPatternStartDate(const std::wstring& val);
    void setNoEndDate       (const BOOL          val);
    void setPatternEndDate  (const std::wstring& val);
    void setOccurrences     (const int           val);
    
    void setStartTime       (const std::wstring& val);
    void setEndTime         (const std::wstring& val);   

    /// for the change-day option
    void setStart           (const std::wstring& val);
    void setIsAllDay        (const BOOL          val);
    
    // for recurring appointment with timezone
    void setEnd             (const std::wstring& val);
    

    // Set property value from its name: set internal object values.
    int setProperty(const std::wstring& propertyName, const std::wstring& propertyValue);



    //
    // To manage appointment exceptions.
    //
    ClientAppException* getException(const int index);
    const int           getExceptionsCount();
    int                 addException(ClientAppException* cException);
    void                resetExceptions();
    int                 saveAllExceptions();
    int                 removeDuplicatedExceptions();

    _AppointmentItemPtr getOccurrence(const DATE originalDate);

    // set the timezone information of the appointment
    void setRecurringTimezone(const TIME_ZONE_INFORMATION* tz) {timeZoneInfo = *tz; }

    const TIME_ZONE_INFORMATION& getRecurringTimezone() const { return timeZoneInfo; }
    

};

/** @} */
/** @endcond */
#endif
