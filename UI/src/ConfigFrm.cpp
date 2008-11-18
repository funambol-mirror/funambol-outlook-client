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

// ConfigFrm.cpp : implementation of the CConfigFrame class
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "OutlookPlugindoc.h"
#include "LeftView.h"
#include "SyncForm.h"
#include "ConfigFrm.h"
#include "AccountSettings.h"
#include "FullSync.h"
#include "LogSettings.h"
#include "MainSyncFrm.h"

#include "ClientUtil.h"


#include "winmaincpp.h"

#include "Tlhelp32.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigFrame

IMPLEMENT_DYNCREATE(CConfigFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CConfigFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CConfigFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
};

/////////////////////////////////////////////////////////////////////////////
// CConfigFrame construction/destruction

CConfigFrame::CConfigFrame() {
}

CConfigFrame::~CConfigFrame() {
}

int CConfigFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!wndStatusBar.Create(this) ||
		!wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

    // TODO: modify here the splitter
	EnableDocking(CBRS_ALIGN_ANY);
    wndSplitter.SetActivePane(0,1);
    RecalcLayout();

    bSyncStarted = false;

	return 0;
}

BOOL CConfigFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

    // TODO: set here main window size and style
    cs.hwndParent = AfxGetMainWnd()->GetSafeHwnd();
    cs.style = WS_SYSMENU  | WS_VISIBLE;
    cs.dwExStyle = WS_EX_DLGMODALFRAME;

    HDC hdc = ::GetDC(0);
    int dpiX = ::GetDeviceCaps(hdc,LOGPIXELSX);
    int dpiY = ::GetDeviceCaps(hdc,LOGPIXELSY);

    double dx = FRAME_CONFIG_X *(((double)dpiX)/96);
    double dy = FRAME_CONFIG_Y *(((double)dpiY)/96);

   if(dpiX > 96){
        // non standard dpi, make the width a little bit smaller
        dx-=10;
    } 
    else if (dpiX < 96){
        dx = FRAME_CONFIG_X;
        dy = FRAME_CONFIG_Y;
    }
    
    cs.cx = (int)dx;
    cs.cy = (int)dy;

    // Center window
    cs.x = (GetSystemMetrics(SM_CXSCREEN) - cs.cx)/2;
    cs.y = (GetSystemMetrics(SM_CYSCREEN) - cs.cy)/2;

    ::ReleaseDC(0,hdc);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CConfigFrame diagnostics

#ifdef _DEBUG
void CConfigFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CConfigFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CConfigFrame message handlers


BOOL CConfigFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{ 
    if (!wndSplitter.CreateStatic(this,1,2,WS_CHILD | WS_VISIBLE | WS_MINIMIZEBOX))
	{
		TRACE(_T("failed to create the splitter"));
		return FALSE;
	}
	
    if (!wndSplitter.CreateView(0,0,RUNTIME_CLASS(CLeftView),CSize(100,100),pContext))
	{
		TRACE(_T("Failed to create view in first pane"));
		return FALSE;
	}
    
    if (!wndSplitter.CreateView(0,1,RUNTIME_CLASS(CSyncForm),CSize(100,100),pContext))
	{
		TRACE(_T("failed to create view in second pane"));
		return FALSE;
	} 

    wndSplitter.bSplitterVisible = TRUE;
    SetWindowText(CONFIG_WINDOW_TITLE);
    SetMenu(NULL); // no menu
    return TRUE;
}

void CConfigFrame::OnClose(){
    ((CMainSyncFrame*)AfxGetMainWnd())->OnConfigClosed();

    CFrameWnd::OnClose();
}

void CConfigFrame::DoCancel(){
    this->DestroyWindow();
}

void CConfigFrame::OnDestroy(){
    ((CMainSyncFrame*)AfxGetMainWnd())->OnConfigClosed();
    Default();
}