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

#include "base/util/StringBuffer.h"
#include "outlook/defs.h"

// Forward declarations
class ClientFolder;
class ClientItem;
class ClientMail;
class ClientContact;
class ClientAppointment;
class ClientTask;
class ClientNote;

#include <string>

// documented MS structure that represent the timezone info
// in the TZI registry
typedef struct _REG_TZI_FORMAT
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

//
// custom structure derived from REG_TZI_FORMAT. It is created
// looking inside the MAPI property of the outlook timezone.
// 
//
typedef struct _REG_TZI_FORMAT_FOR_OUTLOOK
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    char TWO_BYTE_SEP1[2];
    SYSTEMTIME StandardDate;
    char TWO_BYTE_SEP2[2];
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT_FOR_OUTLOOK;


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
    
    // It decides how if we want to use the timezone in rec apps
    // for outgoing appointment. Set by the client in afterPrepareSync method
    bool useOutgoingTimezone;
    
    /**
     * For outgoing timezone. It transforms the retrieved timezone of the event
     * (see the REG_TZI_FORMAT_OUTLOOK struct) into a standard timezone information
     *
     * @param   rtf the reference to a REG_TZI_FORMAT_FOR_OUTLOOK struct
     *
     * @return a new REG_TZI_FORMAT_FOR_OUTLOOK. It must be freed bu the caller
     */
    TIME_ZONE_INFORMATION* convertRegTziOutlookFormat2TimezoneInformation(REG_TZI_FORMAT_FOR_OUTLOOK& rtf);
    

    /**
    * it converts the TIME_ZONE_INFORMATION in a similar one REG_TZI_FORMAT to be used 
    * inside the application
    *
    *@param     the TIME_ZONE_INFORMATION from which calculate the REG_TZI_FORMAT
    *
    *@return    the REG_TZI_FORMAT (MS structure that store a subset of TIME_ZONE_INFORMATION.
    *           It is used inside the windows registry editor
    */
    REG_TZI_FORMAT convertTimezoneInformation2RegTziFormat(const TIME_ZONE_INFORMATION& rtf);
    
    /**
    * it converts the TIME_ZONE_INFORMATION in a similar one but customized to be used 
    * inside the application with Redemption object. The structure has been created
    * studying the property bit a bit...
    *
    *@param     the TIME_ZONE_INFORMATION from which calculate the one for outlook
    *
    *@return    the REG_TZI_FORMAT_FOR_OUTLOOK 
    */
    REG_TZI_FORMAT_FOR_OUTLOOK convertTimezoneInformation2RegTziOutlookFormat(const TIME_ZONE_INFORMATION& rtf);
    
    /**
    * It retrieves the name that is in the Timezone registry. It is language dependent.
    * @param tz     (IN) the REG_TZI_FORMAT that it is used to be compared with the ones got from the 
                    registry. If the blob is the same, then the name property is used
    * @param        display (OUT) is a string buffer provided bu the caller that has the name that
    *               is visualized in the appointment (i.e.  (GMT - 7.00 h) Arizona.)
    *
    *@return        true if a match between the REG_TZI_FORMAT is retrieved, false otherwise
    */
    bool getDisplayTimezone(REG_TZI_FORMAT& tz, Funambol::StringBuffer* display);

protected:

    // Constructor
    ClientApplication(bool checkAttach = false);


public:

    // Method to get the sole instance of ClientApplication
    static ClientApplication* getInstance(bool checkAttach = false);

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

    /**
    * It gets a StringBuffer containing the timezone converted in 
    * hex. It is used to be set into the recurring appointment
    *
    * @param buf the char array whose character has to be converted
    * @param len the length of the buf array
    *
    * @return StringBuffer the hex converted
    */
    Funambol::StringBuffer getHexTimezone(const char *buf, int len);
    
    /**
    * It set the timezone in the recurring appointment
    *
    * @param the Appointment pointer
    */
    void setTimezone(ClientAppointment* cApp);
    
    /**
    * It returns the timezone information of the current recurrent appointment
    * @param cApp a valid ClientAppointment with the COM ptr set
    *
    * @return the TIME_ZONE_INFORMATION struct of the current 
    *         recurrent appointment
    */
    TIME_ZONE_INFORMATION* getTimezone(ClientAppointment* cApp);
    
    void setOutgoingTimezone(bool v) { useOutgoingTimezone = v; }
    bool getOutgoingTimezone() { return useOutgoingTimezone; }
    
    /**
    * It sets at low level the start and end date of the original appointment.
    * This is needed because when setting an appointment params and then the recurrence,
    * the start/end are changed according with the local patternstart and patternend date/time.
    * Then changing the timezone, the Start and End are no more consistent with them.
    * So changing the appointment timezone needs to change the start/end date/time with the 
    * original one sent by the server
    *
    * @param cApp   the clientAppointment instance
    * @param start  the start date (it is a DATE value. It is the local time)
    * @param end    the end   date (it is a DATE value. It is the local time)
    */
    bool setStartAndEnd(ClientAppointment* cApp, DATE start, DATE end);

    /**
    * It checks if the timezone from the appointment sent by the server is the same as is set
    * on the OS. If it is the same, there is no needed to set all the timezone info too.
    *
    * @param tz the REG_TZI_FORMAT to be compared with the current timezone
    * @param standardName if not null, il will contain the standard name found in the OS registry
    */
    bool isTheSameTimezoneRule(REG_TZI_FORMAT& tz, std::wstring* standardName);

    /**
    * It checks if the timezone (in TIME_ZONE_INFORMATION structure) is the same
    * as the OS.
    *
    * @param tzi the TIME_ZONE_INFORMATION structure to be compared with the OS one
    */
    bool isTheSameTimezoneRule(const TIME_ZONE_INFORMATION& tzInfo);

};

/** @} */
/** @endcond */
#endif

