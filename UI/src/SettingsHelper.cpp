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

#include "SettingsHelper.h"
#include "stdafx.h"
#include "resource.h"
#include "winmaincpp.h"
#include "UICustomization.h"
#include "customization.h"
#include "ClientUtil.h"

using namespace std;

bool checkOneWayToTwoWay(int currentSyncType, int newSyncType) {
    int twoWaySyncType = 0;//getSyncTypeIndex("two-way");
    if (newSyncType == twoWaySyncType && currentSyncType != newSyncType) {
        CString s1;

        if (currentSyncType == 1) {//getSyncTypeIndex("one-way-from-server")) {
            s1.LoadString(IDS_WARN_ONE_WAY_FROM_SERVER_TO_TWO_WAY);
        } else {
            s1.LoadString(IDS_WARN_ONE_WAY_FROM_CLIENT_TO_TWO_WAY);
        }

        unsigned int flags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND| MB_TOPMOST;
        int res = IDOK;
        res = MessageBox(NULL, s1, WPROGRAM_NAME, flags);

        if (res == IDNO) {
            return true;
        }
    }

    return false;
}


CString processSharedCheckboxClick(WCHAR * remoteNameRoot, bool isChecked, CString currentValue, CString warningMessage) {
    CString result = remoteNameRoot;

    bool remoteNameIsShared = currentValue.Right(wcslen(SHARED_SUFFIX)).Compare(SHARED_SUFFIX) == 0;

    if (isChecked) {
        if (!remoteNameIsShared) {
            result.Append(SHARED_SUFFIX);
        }
    } else if (remoteNameIsShared && UICustomization::showWarningOnChangeFromOneWay) {
        unsigned int flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND| MB_TOPMOST;
        MessageBox(NULL, warningMessage, WPROGRAM_NAME, flags);
    }

    return result;
}

void moveItem(CWnd * window, CWnd * item, int dX, int dY) {
    CPoint pos = getRelativePosition(item, window);
    item->SetWindowPos(&CWnd::wndTop, pos.x + dX, pos.y + dY, NULL, NULL, SWP_SHOWWINDOW | SWP_NOSIZE);
}

void resizeItem(CWnd * item, int dX, int dY) {
    CRect rect;
    item->GetClientRect(&rect);
    item->SetWindowPos(&CWnd::wndTop, NULL, NULL, rect.Width() + dX, rect.Height() + dY, SWP_SHOWWINDOW | SWP_NOMOVE);
}

void setWindowHeight(CWnd * window, CWnd * bottomItem) {
    // Get sizes of elements
    CRect windowRect;
    CRect itemRect;
    window->GetClientRect(&windowRect);
    bottomItem->GetClientRect(&itemRect);

    CPoint pos = getRelativePosition(bottomItem, window);

    // Windows doesnt let us get the border/titlebar sizes, so we get it on the fly
    window->SetWindowPos(&CWnd::wndTop, NULL, NULL, windowRect.Width(), windowRect.Height(), SWP_SHOWWINDOW | SWP_NOMOVE);

    CRect windowRect2;
    window->GetClientRect(&windowRect2);
    int dW = windowRect.Width() - windowRect2.Width();
    int dH = windowRect.Height() - windowRect2.Height();

    // Adjust by border size
    int newWidth = windowRect.Width() + dW;
    // bottom element top + height of element + toolbar/border size + some space
    int newHeight = pos.y + itemRect.Height() + dH + 15;

    window->SetWindowPos(&CWnd::wndTop, NULL, NULL, newWidth, newHeight, SWP_SHOWWINDOW | SWP_NOMOVE);
}
