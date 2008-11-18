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

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "CustomPane.h"
#include "MainSyncFrm.h"
#include "SyncForm.h"
#include "winmaincpp.h"


IMPLEMENT_DYNAMIC(CCustomPane, CStatic)

CCustomPane::CCustomPane(){
    bMouseCaptured = false;
    hPrevStatusIcon = NULL;
}

CCustomPane::~CCustomPane()
{
}

void CCustomPane::DoDataExchange(CDataExchange* pDX)
{
	CStatic::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCustomPane, CStatic)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_PAINT()
END_MESSAGE_MAP()


// CCustomPane message handlers
void CCustomPane::OnMouseMove(UINT nFlags, CPoint point){
    if( (state == STATE_SYNC) || (state == STATE_PANE_DISABLED) )
        return;

    if(! bMouseCaptured){
        bMouseCaptured = true;

        TRACKMOUSEEVENT Tme;
        Tme.cbSize = sizeof(TRACKMOUSEEVENT);
        Tme.dwFlags = TME_LEAVE;
        Tme.hwndTrack = m_hWnd;
        int Result = TrackMouseEvent(&Tme); 
    
        if(type == PANE_TYPE_SYNC) {
            SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpDarkBlue);            
        }
        else {
            SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpBlue);
        }

        // when the mouse cursor is over a source pane we show the arrow icons
        if(type == PANE_TYPE_CONTACTS){
            hPrevStatusIcon = ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusContacts.GetIcon();
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusContacts.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL)));
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusContacts.ShowWindow(SW_SHOW);
        };

        if(type == PANE_TYPE_CALENDAR){
             hPrevStatusIcon = ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusCalendar.GetIcon();
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusCalendar.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL)));
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusCalendar.ShowWindow(SW_SHOW);
        };

        if(type == PANE_TYPE_TASKS){
            hPrevStatusIcon = ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusTasks.GetIcon();
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusTasks.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL)));
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusTasks.ShowWindow(SW_SHOW);
        };

        if(type == PANE_TYPE_NOTES){
            hPrevStatusIcon = ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusNotes.GetIcon();
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusNotes.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL)));
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusNotes.ShowWindow(SW_SHOW);
        };

        Invalidate();
        
    };    
    CStatic::OnMouseMove(nFlags, point);
}

void CCustomPane::OnPaint(){
    CPaintDC dc(this);
    CDC tempdc;
    tempdc.CreateCompatibleDC(&dc);
    CRect rect;
    GetClientRect(&rect);

    double dpiX = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiX();
    double dpiY = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiY();
    
    HBITMAP pOldBitmap = (HBITMAP) tempdc.SelectObject( GetBitmap() );

    if(dpiX == 96.0){
        dc.BitBlt(0,0, rect.Width(), rect.Height(), &tempdc,  0, 0, SRCCOPY);
    }
    else{
        int dx = (int)(325.0*(dpiX/96.0));
        int dy = (int)(51.0*(dpiX/96.0));
        dc.StretchBlt(0,0,dx, dy, &tempdc,  0, 0,325,51, SRCCOPY);
    }
    
    dc.SelectObject(pOldBitmap);

    if (type == PANE_TYPE_SYNC){

        if(bMouseCaptured) {
            // Mouse move: show blue arrows
            ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusSync.ShowWindow(SW_SHOW);
        }
        else {
            if( ((CMainSyncFrame*)AfxGetMainWnd())->bSyncStarted && !getConfig()->getScheduledSync() ) {
                // Sync started, not scheduled: show blue arrows
                ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusSync.ShowWindow(SW_SHOW);
            }
            else {
                // Sync not started or sched sync: hide blue arrows
                ((CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusSync.ShowWindow(SW_HIDE);
            }
        }
    }

    tempdc.DeleteDC();
    // update all stuff above it
   ( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->repaintPaneControls(type);

}

LRESULT CCustomPane::OnMouseLeave(WPARAM wParam, LPARAM lParam){
// we restore the visual state of the pane, as it was when the mouse cursor entered it
    if(bMouseCaptured){
        bMouseCaptured = false;


        if(type == PANE_TYPE_SYNC) {
            SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpDark);
            Invalidate();
        }
        else {
            if( ((CMainSyncFrame*)AfxGetMainWnd())->bSyncStarted == false || getConfig()->getScheduledSync() ) {
                
                SetBitmap((HBITMAP)((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);

                if( (type == PANE_TYPE_CONTACTS) && (state != STATE_SYNC) ){
                    ( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusContacts.SetIcon(hPrevStatusIcon);
                    Invalidate();
                };

                if( (type == PANE_TYPE_CALENDAR) && (state != STATE_SYNC) ){
                    ( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusCalendar.SetIcon(hPrevStatusIcon);
                    Invalidate();
                }

                if( (type == PANE_TYPE_TASKS) && (state != STATE_SYNC) ){
                    ( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusTasks.SetIcon(hPrevStatusIcon);
                    Invalidate();
                }

                if( (type == PANE_TYPE_NOTES) && (state != STATE_SYNC) ){
                    ( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->iconStatusNotes.SetIcon(hPrevStatusIcon);
                    Invalidate();
                }
            }
        }

        // update all stuff above it
       //( (CSyncForm*)((CMainSyncFrame*)AfxGetMainWnd())->wndSplitter.GetPane(0,1))->repaintPaneControls(type);
    }
   
    return 0;
}