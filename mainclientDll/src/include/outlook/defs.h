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

#ifndef INCL_OUTLOOKDEFS
#define INCL_OUTLOOKDEFS

/** @cond OLPLUGIN */
/** @addtogroup outlook */
/** @{ */


// Outlook parameters
#define OL_APPLICATION                     L"Outlook.Application"
#define OL_PROFILE                         L""                              /**< default current user's profile = ""                                */
#define OL_PASSWORD                        L""                              /**< default = ""                                                       */
#define OL_SHOW_DIALOG                      true                            /**< default = true (if required, prompt dialog to choose profile)      */
#define OL_NEW_SESSION                      false                           /**< default = false                                                    */
#define MAPI                               L"MAPI"

// Date/time definitions
#define USE_UTC                             TRUE                            /**< Use time format in UTC = YYYYMMDDThhmmssZ                          */
#define USE_CHANGE_DAY                      FALSE                           /**< Use change-day option for Recurrence pattern properties            */
#define REFERRED_MAX_DATE                   949998.000000                   /**< this is "4501-01-01" in double format: the error date of Outlook   */
#define LIMIT_MAX_DATE                      767011.000000                   /**< this is "4000-01-01" in double format: the max date accepted       */


// Redemption parameters
/** @cond DEV */
#define RED_SAFEMAIL                        "Redemption.SafeMailItem"
#define RED_SAFEAPPOINTMENT                 "Redemption.SafeAppointmentItem"
#define RED_SAFECONTACT                     "Redemption.SafeContactItem"
#define RED_SAFETASK                        "Redemption.SafeTaskItem"
#define RED_SAFENOTE                        "Redemption.SafeNoteItem"

#define OUTLOOK_2010                       L"Outlook 2010"
#define OUTLOOK_2007                       L"Outlook 2007"
#define OUTLOOK_2003                       L"Outlook 2003"
#define OUTLOOK_XP                         L"Outlook XP (2002)"
#define OUTLOOK_2000                       L"Outlook 2000"
#define OUTLOOK_98                         L"Outlook 98"
#define OUTLOOK_97                         L"Outlook 97"


// Error messages for Outlook
#define ERR_OUTLOOK                         "Outlook Error."
#define ERR_OUTLOOK_OPEN                    "Unable to instantiate Microsoft Outlook.\nPlease check if Outlook is installed, and correctly configured."
#define ERR_OUTLOOK_ATTACH                  "Unable to attach to Microsoft Outlook.\nPlease check if Outlook is installed, open, and correctly configured."
#define ERR_OUTLOOK_LOGOFF                  "Outlook Error: unable to log off."
#define ERR_OUTLOOK_CLEANUP                 "Outlook Error cleaning up."
#define ERR_OUTLOOK_RELEASE_COMOBJECTS      "Error releasing COM pointers."
#define ERR_OUTLOOK_FOLDER_ASSIGN           "Unable to initialize Outlook Folder."
#define ERR_OUTLOOK_ITEM_ASSIGN             "Unable to initialize Outlook Item."
#define ERR_OUTLOOK_SAFEITEM                "Unable to initialize Redemption safe item."
#define ERR_OUTLOOK_MAPIUTILS               "Error accessing Redemption.MAPIUtils object"
#define ERR_OUTLOOK_MAPIUTILS_BODY          "Could not get 'body' value from Redemption.MAPIUtils"
#define ERR_OUTLOOK_RDOSESSION              "Error accessing Redemption.RDOSession object"
#define ERR_OUTLOOK_RDOSESSION_ADDRESS      "Error retrieving address from Redemption.RDOSession AddressList"
#define ERR_OUTLOOK_CONTACT_NOT_SET         "ClientContact not correctly initialized"
#define ERR_OUTLOOK_APPOINTMENT_NOT_SET     "ClientAppointment not correctly initialized"
#define ERR_OUTLOOK_TASK_NOT_SET            "ClientTask not correctly initialized"
#define ERR_OUTLOOK_NOTE_NOT_SET            "ClientNote not correctly initialized"
#define ERR_OUTLOOK_MAIL_NOT_SET            "ClientMail not correctly initialized"
#define ERR_OUTLOOK_NOT_LOGGED              "Not yet logged on Outlook application."
#define ERR_OUTLOOK_GET_PROFILENAME         "Could not retrieve name associated to current Outlook profile."
#define ERR_OUTLOOK_BAD_ITEM                "Item #%d in folder \"%ls\" is not a %ls item. Please check if Outlook is working properly!"
#define ERR_OUTLOOK_BAD_ITEM_MSG            "Outlook is not responding correctly. Please reboot your machine and retry."


#define ERR_OUTLOOK_DEFFOLDER_NOT_FOUND     "Could not find default %ls folder."
#define ERR_OUTLOOK_IDFOLDER_NOT_FOUND      "Could not find folder: ID = %ls."
#define ERR_OUTLOOK_SUBFOLDER_NOT_FOUND     "Subfolder not found (index %d) inside folder %ls."
#define ERR_OUTLOOK_SUBFOLDER_NAME          "Subfolder not found: %ls inside folder %ls."
#define ERR_OUTLOOK_SUBFOLDER_CREATE        "Could not create subfolder %ls of type %ls inside folder %ls: type mismatch."
#define ERR_OUTLOOK_SUBFOLDER_COUNT         "Error retrieving the number of Subfolders from folder %ls."
#define ERR_OUTLOOK_NO_ROOTFOLDER           "Could not find any root folder: please create at least one Outlook data file."
#define ERR_OUTLOOK_ROOTFOLDER_NAME         "Folder not found: '%ls' under root folder."
#define ERR_OUTLOOK_ROOTFOLDER_NOT_FOUND    "Folder not found (index %d) under root folder."

#define ERR_OUTLOOK_ITEM_NOT_FOUND          "Item not found (index %d) inside folder %ls."
#define ERR_OUTLOOK_ITEM_CREATE             "Could not create item (index %d) inside folder %ls."
#define ERR_OUTLOOK_ITEM_SAVE               "Could not save %ls item."
#define ERR_OUTLOOK_ITEM_DELETE             "Could not delete %ls item: %ls."
#define ERR_OUTLOOK_IDITEM_NOT_FOUND        "Could not find item: ID = %ls."
#define ERR_OUTLOOK_ITEMS_COUNT             "Error retrieving the number of Items from folder %ls."

#define ERR_OUTLOOK_INVALID_VERSION         "Warning: Outlook version not supported: %ls"
#define ERR_OUTLOOK_BAD_FOLDER_TYPE         "Selected folder is not a %ls folder."
#define ERR_OUTLOOK_BAD_ITEMTYPE            "Invalid item type: %ls."
#define ERR_OUTLOOK_BAD_OLTYPE              "Invalid Outlook item type: %d."
#define ERR_OUTLOOK_BAD_PATH                "Invalid Folder path: %ls."
#define ERR_OUTLOOK_PATH_TYPE_MISMATCH      "Folder %ls is not a %ls folder: type mismatch."

#define ERR_OUTLOOK_PROP_MAP                "Error creating %ls propertyMap - stopped at property #%d."
#define ERR_OUTLOOK_PROP_VALUE              "Error retrieving %ls item property: %ls."
#define ERR_OUTLOOK_PROP_VALUE_SET          "Error setting property %ls = %ls for item %ls."
#define ERR_OUTLOOK_PROP_NOT_FOUND          "Property '%ls' not found for item %ls."
#define ERR_OUTLOOK_PROP_VALUE_REQUIRED     "Property '%ls' cannot be empty for %ls."
#define ERR_OUTLOOK_REC_PROP_NOT_FOUND      "Recurrence property '%ls' not found."

#define ERR_OUTLOOK_REC_SAVE                "Error inserting recurrence property '%ls' into Outlook."
#define ERR_OUTLOOK_REC_NOT_SET             "Could not %ls recurrence pattern: recurrence not yet set."
#define ERR_OUTLOOK_EXC_PROP_VALUE          "Error retrieving appointment exception property: %ls."
#define ERR_OUTLOOK_CLEAR_REC               "Clould not clear the recurrence pattern of %ls item \"%ls\""

#define ERR_OUTLOOK_EXAPP_INIT              "Error getting appointment item pointer from appointment exception."
#define ERR_OUTLOOK_EXAPP_SAVE              "Error saving the occurrence of appointment exception."
#define ERR_OUTLOOK_EXAPP_DELETE            "Error deleting the occurrence of appointment exception."
#define ERR_OUTLOOK_EXAPP_PROP_SET          "Error setting appointment exception property \"%ls\"."

#define ERR_OUTLOOK_EXCEPTION               "Outlook Exception. - %s"
#define ERR_OUTLOOK_FATAL_EXCEPTION         "Outlook Fatal Exception! - %s"
#define ERR_COM_POINTER                     "COM Pointer Error. Code = %08lx: %ls"

// Other messages:
#define INFO_OUTLOOK_OPENED                 "Outlook session opened successfully! Using: %ls."
#define INFO_OUTLOOK_CLOSED                 "Outlook session closed successfully."
#define INFO_OUTLOOK_REMINDER_RESET         "Cannot save reminder time for task NOT in the default folder! Reminder has been reset on item '%ls'."
#define DBG_OUTLOOK_FOLDER_NOT_SELECTED     "Folder not correctly selected."
#define DBG_OUTLOOK_DLIST_ITEM              "Found a Distribution_list item in %ls folder \"%ls\" (index #%d) -> Item ignored."
#define DBG_OUTLOOK_BAD_NOTE_ITEM           "Found an item in notes folder \"%ls\" which is not a text note (index #%d) -> Item ignored."
#define DBG_OUTLOOK_NOT_LOGGED              "No Logoff: %ls (COM error %08lx)"
/** @endcond */     // cond DEV

#define WMSG_BOX_NO_DATA_ITEM               L"Warning: " WPROGRAM_NAME L" detected a lot of deleted %ls data.\nIf this is not correct, please press no, shutdown outlook, and try again. If this message appears incorrectly again, contact support.\nIf you press yes, a large portion of your %ls data will be deleted on the server.\n\nContinue Sync?"
#define ERR_NO_DATA_ITEM                    PROGRAM_NAME" detected mass delete of %ls data. Verifying with user."

// Import libraries: Outlook Object Model and Redemption.
// --------------------------------------------------------
// Type Libraries are referenced by their unique LIBIDs.
// Redemption.dll is used to bypass Outlook security patch
// (see www.dimastr.com/redemption for details)
// Note:
// LIBRARIES MUST BE REGISTERED IN THE SYSTEM.
// - mso.dll, msoutl.olb: are registered during Microsoft Outlook installation
// - Redemption.dll: is registered during Funambol Outlook Plugin installation
//                   (or manually launching "regsvr32 Redemption.dll")

#define outlookNamespace                    no_namespace
#define redemptionNamespace                 rename_namespace("Redemption")


/// This is LIBID for 'mso.dll'
#import "libid:2DF8D04C-5BFA-101B-BDE5-00AA0044DE52" outlookNamespace \
        rename("DocumentProperties", "OlDocumentProperties") \
        rename("RGB", "OlRGB")

/// This is LIBID for 'msoutl.olb'
#import "libid:00062FFF-0000-0000-C000-000000000046" outlookNamespace \
        rename("CopyFile", "OlCopyFile") \
        rename("Folder", "FunambolFolder")

/// This is LIBID for 'Redemption.dll'
#import "libid:2D5E2D34-BED5-4B9F-9793-A31E26E6806E" redemptionNamespace


/** @} */
/** @endcond */
#endif
