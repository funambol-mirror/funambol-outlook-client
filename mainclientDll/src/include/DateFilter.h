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

#ifndef INCL_DATE_FILTER
#define INCL_DATE_FILTER

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#include "base/fscapi.h"
#include "base/util/ArrayList.h"
#include "base/util/StringBuffer.h"
#include "outlook/ClientItem.h"


/**
 * Rapresents the filter entity on the start/end dates, for appointments.
 * Client can use this class to filter out appointment items that are not
 * included in a specific range of time: events that verifies this filter must exist
 * in the range between 2 dates: 'lowerDate' and 'upperDate'.
 *
 * These dates must be set by the Client before using the filter; if one of them (or both) is NULL,
 * it will not be used for the check. The 'lowerDate' can also be set through a 
 * RelativeLowerDate code, to specify a value that is always relative to the current time.
 *
 * Use the method updateNow() to refresh the filter's members (e.g. if using relative dates).
 * This method should be called before every sync session.
 * Use the method execute() to check if a ClientItem verifies the filter or not.
 */
class  DateFilter {

public:

    /**
     * Filter direction, bitmask value (MSb is IN, LSb is OUT):
     *   - NONE  = filter disabled
     *   - OUT   = filter only outgoing items
     *   - IN    = filter only incoming items
     *   - INOUT = filter both incoming and outgoing items
     *
     * TODO: this enum should be defined in a more generic 'ClientFilter' class.
     */
    typedef enum FilterDirection {
        DIR_NONE  = 0,
        DIR_OUT   = 1,
        DIR_IN    = 2,
        DIR_INOUT = 3
    } FilterDirection;

    /**
     * Codes to set the lowerDate, relatively to the current time.
     * NONE      means we filter out all items before now
     * LAST_WEEK means we filter out all items before last week
     * ALL       means we don't filter anything
     */
    typedef enum RelativeLowerDate {
        NONE          = 0,
        LAST_WEEK     = 1,
        LAST_2_WEEKS  = 2,
        LAST_MONTH    = 3,
        LAST_3_MONTHS = 4,
        LAST_6_MONTHS = 5,
        ALL           = 6
    } RelativeLowerDate;


private:

    /// The filter direction, one of FilterDirection enum.
    /// 0 means the filter is not enabled.
    FilterDirection direction;

    /// The lower limit of startDate. In local time.
    DATE lowerDate;

    /// The upper limit of startDate. In local time.
    DATE upperDate;

    /// A code, to identify the lowerDate referring to now. One of RelativeLowerDate enum.
    RelativeLowerDate relativeLowerDate;

    /// Internal use: it's set to 'true' if we set the lowerDate through the relativeLowerDate.
    bool usingRelativeLowerDate;

    /**
     * Internal use, to subrtract x months from a DATE value.
     * We use SYSTEMTIME structure, it's easier (http://msdn.microsoft.com/en-us/library/aa908737.aspx)
     * @param inputTime  the input date (VariantTime)
     * @param numMonths  number of months to subtract (MUST NOT be > 12)
     * @return           the output date (VariantTime)
     */
    DATE subtractMonths(const DATE inputTime, const unsigned int numMonths);


public:

    DateFilter();
    ~DateFilter();

    void setDirection(const FilterDirection dir) { direction = dir;   }
    void setLowerDate(const DATE lower);
    void setUpperDate(const DATE upper) { upperDate = upper; }

    /// Setting this property will also set 'lowerDate' accordingly.
    _declspec(dllexport) void setRelativeLowerDate(const RelativeLowerDate value);

    
    FilterDirection getDirection() { return direction; }
    DATE getLowerDate() { return lowerDate; }
    DATE getUpperDate() { return upperDate; }
    _declspec(dllexport) RelativeLowerDate getRelativeLowerDate() { return relativeLowerDate; }

    //std::wstring getLowerDate();
    //std::wstring getUpperDate();

    /// Returns true if filter enabled at least in one direction.
    bool isEnabled();

    /**
     * Checks whether the item passed verifies the current filter.
     * Two checks are done, both MUST be verified to return true:
     *   1. event MUST NOT END before lowerDate
     *   2. event MUST NOT START after upperDate
     *
     * @note   This method is less performing than 'execute(_AppointmentItemPtr&)' as it
     *         checks the string values stored by the wrapper objects, so we need to convert
     *         them into Variant DATE values. 
     *         But it's necessary for items not yet saved in Outlook (if filtering incoming items)
     *         because in this case the COMPtr does not reflect the real values.
     * @return true  if the filter is verified (event within the range)
     *         false if not
     */
    bool execute(ClientItem* cItem);

    /**
     * Checks whether the item passed verifies the current filter.
     * Like 'execute(ClientItem*)', but uses the COM pointer object directly:
     *   - better performance
     *   - independent on changes to getProperty() methods / format of dates
     *   - avoid reading all rec props that are not used here
     *
     * @note This method should be used for outgoing items, because the COMPtr is already set.
     *       Incoming items have an incomplete COMPtr until the item is effectively saved!
     * @return true  if the filter is verified (event within the range)
     *         false if not
     */
    bool execute(_AppointmentItemPtr& app);

    /**
     * Refreshes filter's parameters, based on the current time.
     */
    void updateNow();

};

/** @} */
/** @endcond */
#endif
