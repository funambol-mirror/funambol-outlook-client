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
#define ABOUT_SCREEN_TEXT_COPYRIGHT         "Copyright © 2003 - 2010 Funambol, Inc.\nAll rights reserved."

#define ABOUT_SCREEN_SHOW_MAIN_WEB_SITE     1                       /**< if 1, will show the main web address below */
#define ABOUT_SCREEN_TEXT_MAIN_WEB_SITE     "www.funambol.com"

#define ABOUT_SCREEN_SHOW_LICENSE           1                       /**< if 1, will show the AGPL license text */
#define ABOUT_SCREEN_SHOW_POWERED_BY        0                       /**< if 1, will show the "powered by" text below instead of the AGPL license */
#define ABOUT_SCREEN_TEXT_POWERED_BY        "Powered by Funambol"


// Others
#define VIEW_USER_GUIDE_LINK                0                           /**< 1 to display the 'view userguide' in the UI menu (hidden by default) */
#define USER_GUIDE_LINK                     "http://funambol.com/docs/v80/funambol-outlook-sync-client-user-guide.pdf"
#define PROGRAM_NAME_EXE                    "OutlookPlugin.exe"             // The application to run
#define SCHED_COMMENT                       TEXT(PROGRAM_NAME) TEXT(" scheduler")
#define OL_PLUGIN_LOG_NAME                  "outlook-client-log.txt"
#define ENABLE_ENCRYPTION_SETTINGS          1                           /**< 0 to hide the encryption UI check in the Settings screen */
#define SHOW_ADVANCED_SETTINGS              1                           /**< 0 to hide the advanced source settings (remote URIs) */
#define DISPLAY_SLOWSYNC_WARNING            0                           /**< 1 to display a timed-msgbox if Server requests a SLOW SYNC */
#define ASK_SLOW_TIMEOUT                    25                          /**< 25 seconds    */
#define TIME_OUT_ABORT                      8                           /**< 8  seconds    */
#define SCHED_DEFAULT_REPEAT_MINS           15                          /**< 15 minutes    */
#define SCHED_DURATION_DAYS                 1                           /**< 1 day         */
#define SYNC_TIMEOUT                        120                         /**< 120 minutes   */
#define MAX_LOG_SIZE                        10000000                    /**< 10 MB         */
#define MAX_SYNCML_MSG_SIZE                 125000                      /**< [bytes], the max syncML message size. default = 125KB */
#define RESPONSE_TIMEOUT                    900                         /**< [seconds], the HTTP timeout on Server response. default = 15 minutes */
#define DYNAMICALLY_SHOW_PICTURES           1                           /**< if 1, will automatically show/hide the pictures panel, at the end of sync */


// Win registry root context.
// This is NOT intended to be customized: we need to use the same registry keys to ensure correct checks
// between different versions of the client (i.e. avoid installing 2 plugins, addin cleanup)
// Note: in case of change, please make sure at least one "/" exist.
#define PLUGIN_ROOT_CONTEXT                 "Funambol/OutlookClient"


#endif