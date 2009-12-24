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

/** @cond OLPLUGIN */
/** @addtogroup UI_utils */
/** @{ */

#include "DateFilter.h"

#define SYNC_DEFAULT_TOTAL_ITEMS  20

// Default dimensions of frames (in case of 96dpi)
#define FRAME_CONFIG_X      440
#define FRAME_CONFIG_Y      420
#define FRAME_MAIN_X        350
#define FRAME_MAIN_Y        345
#define SOURCE_PANE_SIZE_Y  45      // height of each source pane on main screen

#define MAIN_PROGRESSBAR_COLOR RGB(255,255,255)

#define EDIT_TEXT_MAXLENGTH 255
#define EDIT_TEXT_SCHEDULER_MAXLENGTH 6

#define SYNCTYPE_NONE "none"

//#define LAST_SYNC_TIME_FORMAT TEXT("%A %#d %b %Y %#I:%M%p")
//#define LAST_SYNC_TIME_FORMAT TEXT("%#d %b %Y %#I:%M%p")
#define LAST_SYNC_TIME_FORMAT TEXT("%A, %#I:%M%p")


int getSyncTypeIndex(const char* syncType);

const char* getSyncTypeName    (int index);
const char* getFullSyncTypeName(int index);

int getSyncModeCode(const char* syncMode);

int getDateFilterIndex(const DateFilter::RelativeLowerDate value);
DateFilter::RelativeLowerDate getDateFilterValue(const int index);

void manageSyncErrorMsg(long code);

CPoint getRelativePosition(CWnd* wnd, CWnd* parentWnd);

/**
 * Checks the sources visible (array in config) to return
 * dynamically the size of the mains screen dialog.
 */
CPoint getMainWindowSize();

/** @} */
/** @endcond */
