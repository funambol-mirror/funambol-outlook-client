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

#ifndef INCL_CUSTOMIZATION
#define INCL_CUSTOMIZATION

#include "vocl/appdefs.h"
#include <string>
#include "winmaincpp.h"

// The application name
#define APP_NAME                            "Funambol"

// The application full name, used also in About screen title
#define PROGRAM_NAME                        "Funambol Outlook Sync Client"
#define WPROGRAM_NAME                       TEXT(PROGRAM_NAME)

// UI windows titles
#define PLUGIN_UI_TITLE                     PROGRAM_NAME
#define CONFIG_WINDOW_TITLE                 _T(PROGRAM_NAME) _T(" Options")
#define MSGBOX_ERROR_TITLE                  PROGRAM_NAME " Error"
#define WMSGBOX_ERROR_TITLE                 TEXT(PROGRAM_NAME) TEXT(" Error")

// Default values for Account settings
#define DEFAULT_URL                         "http://my.funambol.com/sync"
#define DEFAULT_USERNAME                    ""
#define DEFAULT_PASSWORD                    ""

// Addin customization
// This macro is used into the Outlook menu. The & is the value used to create a shortcut to open the client
#define ADDIN_MENU_LABEL                    L"Funa&mbol"
#define LAST_COMPATIBLE_VERSION             80207                       /**< "8.2.6" is the latest version compatible with this addin - change this value when addin need to be reinstalled */

// The program folder
#define FUNAMBOL_DIR_NAME                   TEXT("Funambol")

// About screen customization
#define ABOUT_SCREEN_SHOW_COPYRIGHT         1                       /**< if 1, will show the copyright text below */
#define ABOUT_SCREEN_TEXT_COPYRIGHT         "Copyright © 2003 - 2011 Funambol, Inc.\nAll rights reserved."

#define ABOUT_SCREEN_SHOW_MAIN_WEB_SITE     1                       /**< if 1, will show the main web address below */
#define ABOUT_SCREEN_TEXT_MAIN_WEB_SITE     "www.funambol.com"

#define ABOUT_SCREEN_SHOW_LICENSE           1                       /**< if 1, will show the AGPL license text */
#define ABOUT_SCREEN_SHOW_POWERED_BY        0                       /**< if 1, will show the "powered by" image instead of the AGPL license */


// Others
#define VIEW_USER_GUIDE_LINK                0                           /**< 1 to display the 'view userguide' in the UI menu (hidden by default) */
#define USER_GUIDE_LINK                     "http://funambol.com/docs/v80/funambol-outlook-sync-client-user-guide.pdf"
#define PROGRAM_NAME_EXE                    "OutlookPlugin.exe"             // The application to run
#define SCHED_COMMENT                       TEXT(PROGRAM_NAME) TEXT(" scheduler")
#define OL_PLUGIN_LOG_NAME                  "synclog.txt"
#define ENABLE_ENCRYPTION_SETTINGS          1                           /**< 0 to hide the encryption UI check in the Settings screen */
#define SHOW_ADVANCED_SETTINGS              1                           /**< 0 to hide the advanced source settings (remote URIs) */
#define DISPLAY_SLOWSYNC_WARNING            0                           /**< 1 to display a timed-msgbox if Server requests a SLOW SYNC */
#define ASK_SLOW_TIMEOUT                    25                          /**< 25 seconds    */
#define TIME_OUT_ABORT                      8                           /**< 8  seconds    */
#define SCHED_DEFAULT_REPEAT_MINS           15                          /**< 15 minutes    */
#define SCHED_DURATION_DAYS                 1                           /**< 1 day         */
#define SYNC_TIMEOUT                        120                         /**< 120 minutes   */
#define MAX_LOG_SIZE                        3000000                     /**< 3 MB          */
#define MAX_SYNCML_MSG_SIZE                 125000                      /**< [bytes], the max syncML message size. default = 125KB */
#define RESPONSE_TIMEOUT                    900                         /**< [seconds], the HTTP timeout on Server response. default = 15 minutes */
#define DYNAMICALLY_SHOW_PICTURES           0                           /**< if 1, will automatically show/hide the pictures panel, at the end of sync */
#define MAX_IMAGE_SIZE                      0                           /**< max size of pictures to upload [KBytes]. 0 means unlimited. */
#define MAX_VIDEO_SIZE                      0                           /**< max size of videos to upload [KBytes]. 0 means unlimited. */
#define MAX_FILE_SIZE                       0                           /**< max size of files to upload [KBytes]. 0 means unlimited. */

#define SOURCE_ORDER_IN_REGISTRY            "contact,appointment,task,note,picture,video,files"

// set if the sources are enabled/disabled (meaning they can be enabled by settings)
#define CONTACT_SOURCE_ENABLED              true
#define APPOINTMENT_SOURCE_ENABLED          true
#define TASK_SOURCE_ENABLED                 true
#define NOTE_SOURCE_ENABLED                 true
#define PICTURE_SOURCE_ENABLED              true
#define VIDEO_SOURCE_ENABLED                true
#define FILE_SOURCE_ENABLED                 true

// List of available sync modes for each source (comma separated values).
// These are the values available from the client's settings for each source.
#define CONTACTS_SYNC_MODES                 SYNC_MODE_TWO_WAY
#define APPOINTMENTS_SYNC_MODES             SYNC_MODE_TWO_WAY
#define TASKS_SYNC_MODES                    SYNC_MODE_TWO_WAY
#define NOTES_SYNC_MODES                    SYNC_MODE_TWO_WAY
#define PICTURES_SYNC_MODES                 SYNC_MODE_TWO_WAY "," SYNC_MODE_ONE_WAY_FROM_CLIENT "," SYNC_MODE_ONE_WAY_FROM_SERVER
#define VIDEOS_SYNC_MODES                   SYNC_MODE_TWO_WAY "," SYNC_MODE_ONE_WAY_FROM_CLIENT "," SYNC_MODE_ONE_WAY_FROM_SERVER
#define FILES_SYNC_MODES                    SYNC_MODE_TWO_WAY "," SYNC_MODE_ONE_WAY_FROM_CLIENT "," SYNC_MODE_ONE_WAY_FROM_SERVER

// Default sync mode for each source.
// It MUST be one of the values specified in the list of available sync modes above.
#define DEFAULT_CONTACTS_SYNC_MODE          SYNC_MODE_TWO_WAY
#define DEFAULT_APPOINTMENTS_SYNC_MODE      SYNC_MODE_TWO_WAY
#define DEFAULT_TASKS_SYNC_MODE             SYNC_MODE_TWO_WAY
#define DEFAULT_NOTES_SYNC_MODE             SYNC_MODE_TWO_WAY
#define DEFAULT_PICTURES_SYNC_MODE          SYNC_MODE_TWO_WAY
#define DEFAULT_VIDEOS_SYNC_MODE            SYNC_MODE_TWO_WAY
#define DEFAULT_FILES_SYNC_MODE             SYNC_MODE_TWO_WAY

#define SCHEDULED_MINUTES_VALUES            "5,10,15(default),30,45"
#define SCHEDULED_HOURS_VALUES              "1,2,4,6,8,12,24"

#define ENABLE_COMPRESSION					true

// Win registry root context.
// This is NOT intended to be customized: we need to use the same registry keys to ensure correct checks
// between different versions of the client (i.e. avoid installing 2 plugins, addin cleanup)
// Note: in case of change, please make sure at least one "/" exist.
#define PLUGIN_ROOT_CONTEXT                 "Funambol/OutlookClient"

// Auto-update feature
#define UP_URL_RESOURCE                     "/sapi/profile/client?action=get-update-info"
#define CLIENT_PLATFORM                     "outlook"


#include "base/util/StringBuffer.h"


class DLLCustomization {
public:
    static const bool defaultUseSubfolders              = true;     /**< include subfolders check (PIM) */
    static const bool removeFilteredDataOnCleanup       = true;     /**< send filtered out items as Delete items, in case of refresh sync */
    static const bool sendMovedAsNew                    = false;    /**< send items as new instead of updated, if moved to another folder */
    static const bool dontSendFilteredItemsAsDeleted    = true;     /**< don't send items out of the calendar filter as Deleted items */
    static const bool neverSendPhotos                   = false;    /**< don't send contact's photo */
    static const bool saveFileAs                        = false;    /**< save the contact's "fileAs" field, otherwise it's auto generated by Outlook */
    static const bool syncAttendees                     = true;     /**< to sync calendar's attendees */
    static const bool sendTimezone                      = true;     /**< to avoid sending events tz */
    static const bool continueOnSlowWithOneWay          = false;    /**< will not auto-continue a slow if sync direction is set to one-way */
    static const bool warnOnLargeDelete                 = false;    /**< If 50% of items or more are deleted, displays a warning */

    /// to validate custom X-foo properties
    static bool validateExtraProperty(const std::wstring & name) {
        const wchar_t * n = name.c_str();
        return
            (!wcsncmp(n,X_PREFIX,wcslen(X_PREFIX)) && // any X- Properties
            wcsncmp(n,X_FUNAMBOL_PREFIX,wcslen(X_FUNAMBOL_PREFIX)) && // any non-funambol X- Properties
            wcsncmp(n,X_MICROSOFT_PREFIX,wcslen(X_MICROSOFT_PREFIX)) && // any non-microsoft properties
            wcscmp(n,X_WM_CLIENT_CONTAINER_ID) && wcscmp(n,X_WM_CLIENT_CONTAINER_NAME)) // not certain WM props
            ;
    }

    // Source defaults
    static const char *  sourceDefaultEncoding;         /**< the default encoding for synsources */
    static const bool    sourceNotesDefaultSif;         /**< default notes data format (true = SIF, false = vnote) */
    static const char *  sourceNotesSifUri;             /**< the default sources URI for SIF notes */
    static const char *  sourceNotesVnoteUri;           /**< the default sources URI for vNote */
    static const char *  sourceTasksVcalUri;            /**< the default sources URI for vEvent */
    static const char *  sourceCalendarVcalUri;         /**< the default sources URI for vTodo */
    static const char *  sourceContactsVcardUri;        /**< the default sources URI for vCard */
    static const char *  sourcePicturesUri;             /**< the default sources URI for pictures */
    static const char *  sourceVideosUri;               /**< the default sources URI for videos */
    static const char *  sourceFilesUri;                /**< the default sources URI for files */

    // For upgrades
    static const bool shouldFakeOldFunambolSwv = false; /**< if true, the installed version is replaced by a custom value */
    static const int fakeOldFunambolSwv = 80100;        /**< the custom value (see shouldFakeOldFunambolSwv) */

};

#endif
