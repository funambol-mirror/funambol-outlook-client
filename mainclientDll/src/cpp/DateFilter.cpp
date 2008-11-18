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

#include "DateFilter.h"
#include "winmaincpp.h"
#include "utils.h"
#include "outlook/utils.h"
#include "outlook/ClientException.h"

using namespace Funambol;
using namespace std;


DateFilter::DateFilter() 
{
    // Init with default values.
    setDirection(DIR_NONE);
    setRelativeLowerDate(LAST_MONTH);   // will set 'lowerDate' too.
    setUpperDate(NULL);
}

DateFilter::~DateFilter() {}

void DateFilter::setLowerDate(const DATE lower) 
{ 
    usingRelativeLowerDate = false;
    lowerDate = lower;
}

void DateFilter::setRelativeLowerDate(const RelativeLowerDate value)
{
    usingRelativeLowerDate = true;
    relativeLowerDate = value;

    // Also set the lowerDate accordingly.
    updateNow();

}

bool DateFilter::isEnabled() 
{
    if (direction) {
        return true;
    }
    return false;
}



bool DateFilter::execute(ClientItem* cItem)
{
    if (!isEnabled()) {
        // Filter disabled.
        return true;
    }
    if (!lowerDate && !upperDate) {
        // Filter is trasparent, always verified.
        return true;
    }

    try {
        ClientAppointment* cApp = (ClientAppointment*)cItem;
        if (!cApp) {
            LOG.error("DateFilter::execute - not an appointment item");
            return true;
        }

        _AppointmentItemPtr& app = cApp->getCOMPtr();
        bool isRecurring = (cApp->getRecPattern())? true : false;
        

        // Check 1: event MUST NOT END before lowerDate.
        //          Use PatternEndDate for recurring events that have an end.
        if (lowerDate) {
            DATE end = NULL;
            if (isRecurring) {
                if (cApp->getRecPattern()->getNoEndDate()) {
                    end = REFERRED_MAX_DATE;
                }
                else {
                    const wstring& endString = cApp->getRecPattern()->getPatternEndDate();
                    systemTimeToDouble(endString, &end);
                }
            }
            else {
                wstring endString = cApp->getProperty(L"End");
                systemTimeToDouble(endString, &end);
            }
            
            if (end < lowerDate) {
                return false;
            }
        }
        
        // Check 2: event MUST NOT START after upperDate.
        //          Use PatternStartDate for recurring events.
        if (upperDate) {
            DATE start = NULL;
            if (isRecurring) {
                const wstring& startString = cApp->getRecPattern()->getPatternStartDate();
                systemTimeToDouble(startString, &start);
            } 
            else {
                wstring startString = cApp->getProperty(L"Start");
                systemTimeToDouble(startString, &start);
            }
            
            if (start > upperDate) {
                return false;
            }
        }

    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException("Error executing date filter");
    }

    // If here, all conditions are verified: ok.
    return true;
}



bool DateFilter::execute(_AppointmentItemPtr& app)
{
    if (!isEnabled()) {
        // Filter disabled.
        return true;
    }
    if (!lowerDate && !upperDate) {
        // Filter is trasparent, always verified.
        return true;
    }

    try {
        bool isRecurring = vBoolToBool(app->GetIsRecurring());

        // Check 1: event MUST NOT END before lowerDate.
        //          Use PatternEndDate for recurring events that have an end.
        if (lowerDate) {
            DATE end = NULL;
            if (isRecurring) {
                if (app->GetRecurrencePattern()->GetNoEndDate()) {
                    end = REFERRED_MAX_DATE;
                }
                else {
                    end = app->GetRecurrencePattern()->GetPatternEndDate();
                }
            }
            else {
                end = app->GetEnd(); 
            }
            
            if (end < lowerDate) {
                return false;
            }
        }
        
        // Check 2: event MUST NOT START after upperDate.
        //          Use PatternStartDate for recurring events.
        if (upperDate) {
            DATE start = NULL;
            if (isRecurring) { start = app->GetRecurrencePattern()->GetPatternStartDate(); } 
            else             { start = app->GetStart(); }
            
            if (start > upperDate) {
                return false;
            }
        }

    }
    catch(_com_error &e) {
        manageComErrors(e);
        throwClientException("Error executing date filter");
    }

    // If here, all conditions are verified: ok.
    return true;
}



void DateFilter::updateNow()
{
    if (!isEnabled()) {
        return;
    }

    if (usingRelativeLowerDate) {

        // In this case, the lowerDate depends on current time!
        DATE nowDate = NULL;
        SYSTEMTIME now;
        GetLocalTime(&now);
        SystemTimeToVariantTime(&now, &nowDate);
    
        switch (relativeLowerDate)
        {
            case NONE:
            {
                lowerDate = nowDate;
                break;
            }
            case LAST_WEEK:
            {
                lowerDate = nowDate - 7;    // 1 unit = 1 day
                break;
            }
            case LAST_2_WEEKS:
            {
                lowerDate = nowDate - 14;   // 1 unit = 1 day
                break;
            }
            case LAST_MONTH:
            {
                lowerDate = subtractMonths(nowDate, 1);
                break;
            }
            case LAST_3_MONTHS:
            {
                lowerDate = subtractMonths(nowDate, 3);
                break;
            }
            case LAST_6_MONTHS:
            {
                lowerDate = subtractMonths(nowDate, 6);
                break;
            }
            case ALL:
            {
                lowerDate = 0;
                break;
            }
        }
    }
}


DATE DateFilter::subtractMonths(const DATE inputTime, const unsigned int numMonths)
{
    SYSTEMTIME sysTime;
    VariantTimeToSystemTime(inputTime, &sysTime);

    if (numMonths > 12) {
        LOG.error("DateFilter::subtractMonths - numMonths = %d", numMonths);
        return inputTime;
    }

    if (sysTime.wMonth > numMonths) {
        sysTime.wMonth -= numMonths;
    }
    else {
        // Falling into last year
        sysTime.wMonth = sysTime.wMonth + 12 - numMonths;
        sysTime.wYear --;
    }

    DATE outTime = NULL;
    SystemTimeToVariantTime(&sysTime, &outTime);
    return outTime;
}


