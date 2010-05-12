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

#ifndef _CUSTOMIZATION_H_
#define _CUSTOMIZATION_H_

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */

class UICustomization {

private:
    UICustomization() {}

public:

    const static bool   forceUseSubfolders;             /**< hide the "include subfolders" UI checkbox, and force as it was enabled */
    const static bool   clearAnchorsOnFolderChange;     /**< forces a slow if sync folder changed from client's settings (PIM only) */
    const static bool   hideDataFormats;                /**< hides the "Data Format" section in the sources details screen */
    const static bool   shared;                         /**< Enables the Shared folder sync (checkbox in the sources details screen) */
    const static bool   lockCalendarFilter;             /**< Locks the calendar filtering on a value (cannot be changed from UI) */
    const static int    lockCalendarFilterValue;        /**< The index where the calendar is locked (used only if lockCalendarFilter = true) */
    const static bool   showWarningOnChangeFromOneWay;  /**< Shows a warning popup if sync mode changed from one-way to two-way (in sources details screen) */
    const static bool   defaultFullSyncFromClient;      /**< Recover screen: if true, the default direction is client-to-server */
    const static int    syncAllTextRed;                 /**< The "syncAll" text color, expressed in RGB values (RED value) */
    const static int    syncAllTextGreen;               /**< The "syncAll" text color, expressed in RGB values (GREEN value) */
    const static int    syncAllTextBlue;                /**< The "syncAll" text color, expressed in RGB values (BLUE value) */
    const static bool   attachOption;                   /**< Adds a "Only sync when Outlook is open" checkbox in the scheduler settings */
    const static bool   confirmOnRefreshFromClient;     /**< Recover screen: shows a warning message before starting a "refresh-from-client" sync */
    const static bool   confirmOnRefreshFromServer;     /**< Recover screen: shows a warning message before starting a "refresh-from-server" sync */
    const static bool   sendLogs;                       /**< Shows log rotation options in logging screen: log size (1<->20MB) and log files number (1<->20) */
    const static bool   logRotateOptions;               /**< Shows "Send Log" button in the logging screen */
    const static bool   showWelcomeMessage;             /**< Shows a welcome screen at first run or in case of upgrade */
    const static bool   showUpgradingMessage;           /**< Shows a small popup screen during the upgrade: "Upgrading Sync for Outlook" */

};

/** @} */
/** @endcond */

#endif //_CUSTOMIZATION_H_
