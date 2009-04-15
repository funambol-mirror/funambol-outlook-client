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

//addin
#define PROGRAM_NAME                        "Funambol Outlook Sync Client"
#define WPROGRAM_NAME                      L"Funambol Outlook Sync Client"

#define DEFAULT_URL                         "http://my.funambol.com/sync"
#define DEFAULT_USERNAME                    ""
#define DEFAULT_PASSWORD                    ""

#define PLUGIN_UI_TITLE                     PROGRAM_NAME  // The UI windows title (must be unique!)
//This macro is used into the Outlook menu. The & is the value used to create a shortcut to open the client
#define FUNAMBOL                           L"Funa&mbol"
#define CAPTION                            WPROGRAM_NAME
#define LAST_COMPATIBLE_VERSION             70104                       // "7.1.4" is the latest version compatible with this addin

#define OL_PLUGIN_LOG_NAME                  "outlook-client-log.txt"
#define TIME_OUT_ABORT                      8                           /**< 8  seconds    */

#define APP_NAME                            "Funambol"

#define FUNAMBOL_DIR_NAME                  L"Funambol"
#define SCHED_COMMENT                      L"Funambol Outlook Sync Client scheduler"
#define WMSGBOX_ERROR_TITLE                L"Funambol Outlook Client Error"
#define MSGBOX_ERROR_TITLE                  "Funambol Outlook Client Error"

#define CONFIG_WINDOW_TITLE         _T("Funambol Outlook Sync Client Options")

#endif