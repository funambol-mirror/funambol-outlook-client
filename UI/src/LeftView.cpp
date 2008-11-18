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
#include "afxext.h"

#include "OutlookPluginDoc.h"
#include "LeftView.h"
#include "ConfigFrm.h"
#include "SyncForm.h"
#include "AccountSettings.h"
#include "SyncSettings.h"
#include "MainSyncFrm.h"

#include "winmaincpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CFormView)

BEGIN_MESSAGE_MAP(CLeftView, CFormView)
	//{{AFX_MSG_MAP(CLeftView)
	//}}AFX_MSG_MAP
    ON_WM_NCPAINT()
    ON_WM_CTLCOLOR()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LEFT_LIST, &CLeftView::OnLvnItemchangedLeftList)
    //ON_NOTIFY(LVN_GETDISPINFO, IDC_LEFT_LIST, &CLeftView::OnLvnGetdispinfoLeftList)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_LEFT_LIST, &CLeftView::OnLvnItemchangingLeftList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeftView construction/destruction

CLeftView::CLeftView() : CFormView(CLeftView::IDD), m_target(NULL)
{
    previousSelectedItem = -1;
    currentSelectedItem = -1;
    yOffset = 0;
}

CLeftView::~CLeftView()
{
    brush.DeleteObject();
}

void CLeftView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLeftView)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_LEFT_LIST, lstConfig);
}

void CLeftView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
    brush.CreateSolidBrush(RGB(255,255,255));
    populateList();
    lstConfig.SetSelectionMark(0);


    int dpiX = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiX();
    int dpiY = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiY();

    if((dpiX != 96) || (dpiY != 96)){
        // nonstandard dpi, we move around the separators
        CRect rect;
        GetDlgItem(IDC_SEP1)->GetWindowRect(&rect);
        ScreenToClient(&rect);
        GetDlgItem(IDC_SEP1)->MoveWindow(rect.TopLeft().x, 64, 
            rect.Width(), rect.Height(), TRUE);

        GetDlgItem(IDC_SEP2)->GetWindowRect(&rect);
        ScreenToClient(&rect);
        GetDlgItem(IDC_SEP2)->MoveWindow(rect.TopLeft().x, 128, rect.Width(), rect.Height(), TRUE);

        GetDlgItem(IDC_SEP1)->Invalidate();
        GetDlgItem(IDC_SEP2)->Invalidate();
    }
        
    // Default go to Sync-settings (set from showSettingsWindow())
    //selectItem(1);
}


/////////////////////////////////////////////////////////////////////////////
// CLeftView diagnostics

#ifdef _DEBUG
COutlookPluginDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COutlookPluginDoc)));
	return (COutlookPluginDoc*)m_pDocument;
}
#endif //_DEBUG


BOOL CLeftView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(m_target)
	{		
		m_target->SendMessage(WM_COMMAND, wParam, lParam);
	}
	else
	{
		CFormView::OnCommand(wParam, lParam);
	}
	return true;	
}

void CLeftView::SetTarget(CWnd* m_cwnd)
{
	m_target = m_cwnd;
}

void CLeftView::OnNcPaint(){
    CFormView::OnNcPaint();
    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));   
}

void CLeftView::populateList(){
    CString s1;
    imgList.Create(32, 32, ILC_COLOR32, 0, 0);
    
    HICON ic1, ic2;
    ic1 = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ACCOUNT));
    ic2 = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC));
    imgList.Add(ic1);   
    imgList.Add(ic2);

    lstConfig.SetIconSpacing(64,36);

    // resize the list so it would have the same width as the window holding it
    if( ((CMainSyncFrame*)AfxGetMainWnd())->getDpiX() != 96){
        // if it's normal 96dpi we leave as it is
        CRect rectLst,rectWnd;
        lstConfig.GetClientRect(rectLst);
        GetClientRect(rectWnd);

        lstConfig.SetWindowPos(&CWnd::wndTop, rectLst.TopLeft().x, rectLst.TopLeft().y,
            rectWnd.Width(), rectLst.Height(), SWP_SHOWWINDOW);
    }
        
    lstConfig.SetItemCount(2);
    lstConfig.SetImageList(&imgList, LVSIL_NORMAL);

    // uncomment this for OnGetDispInfo event
    lstConfig.SetCallbackMask(lstConfig.GetCallbackMask() | LVIS_SELECTED);

    s1.LoadString(IDS_ACCOUNT);
    lstConfig.InsertItem(0,s1, 0);
    s1.LoadString(IDS_SYNC);
    lstConfig.InsertItem(1,s1, 1);
    lstConfig.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

    // Correct position of first item.
    POINT pt;
    pt.x = 15;
    pt.y = 10;
    lstConfig.SetItemPosition(0, pt);

    // Set bkground image on first item.
    setBkgImage(0);
}


void CLeftView::OnLvnItemchangedLeftList(NMHDR *pNMHDR, LRESULT *pResult) {

    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    CConfigFrame *pConfigFrame=(CConfigFrame*) ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame;

    //
    // First time
    //
    if (currentSelectedItem == -1) {
        currentSelectedItem = pNMLV->iItem;
        if (currentSelectedItem == 0) {
            pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CAccountSettings),CSize(100,100));
            setBkgImage(0);
        }
        else if (currentSelectedItem == 1) {
            pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CAccountSettings),CSize(100,100));
            setBkgImage(0);
        }
        return;
    }


    previousSelectedItem = currentSelectedItem;
    if(pNMLV->uNewState != 0) {
        currentSelectedItem = pNMLV->iItem;
    }
    else {
        // Not an event for a new state -> exit.
        return;
    }

    if (previousSelectedItem == currentSelectedItem) {
        // State not changed: nothing to do -> exit.
        return;
    }


    if (currentSelectedItem == 0) {
        if(! ((CSyncSettings*)pConfigFrame->wndSplitter.GetPane(0,1))->saveSettings(false)){
            // stay to sync settings pane
            currentSelectedItem = 1;
        }
        else{
            // show account settings window
            pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CAccountSettings),CSize(100,100));
            setBkgImage(0);
        }
    }
    else if (currentSelectedItem == 1) {
        if(! ((CAccountSettings*)pConfigFrame->wndSplitter.GetPane(0,1))->saveSettings(false)){
            // stay to account pane
            currentSelectedItem = 0;
        }
        else{
            // show settings pane
           pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CSyncSettings),CSize(100,100));
           setBkgImage(1);
        }
    }
    *pResult = 0;
}



void CLeftView::selectItem(const int index) {

    currentSelectedItem = index;
    if (previousSelectedItem == currentSelectedItem) {
        return;
    }

    CConfigFrame *pConfigFrame=(CConfigFrame*) ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame;


    if (currentSelectedItem == 0) {
        if ( (previousSelectedItem != -1) && 
             ((CSyncSettings*)pConfigFrame->wndSplitter.GetPane(0,1))->saveSettings(false) ) {
            // stay to sync settings pane
            setBkgImage(1);
            lstConfig.SetSelectionMark(1);
        }
        else{
            // show account settings window            
            pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CAccountSettings),CSize(100,100));
            setBkgImage(0);
        }
    }
    else if (currentSelectedItem == 1) {
        if ( (previousSelectedItem != -1) && 
             ((CAccountSettings*)pConfigFrame->wndSplitter.GetPane(0,1))->saveSettings(false) ) {
            // stay to account pane
            setBkgImage(0);
            lstConfig.SetSelectionMark(0);
        }
        else{
            // show settings pane
           pConfigFrame->wndSplitter.ReplaceView(0,1,RUNTIME_CLASS(CSyncSettings),CSize(100,100));
           setBkgImage(1);
        }
    }
}




HBRUSH CLeftView::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor){
    // white background
    pDC->SetBkColor(RGB(255,255,255));
    return (HBRUSH) (brush.GetSafeHandle());
}


void CLeftView::OnLvnGetdispinfoLeftList(NMHDR *pNMHDR, LRESULT *pResult) {
    // empty
}

void CLeftView::OnLvnItemchangingLeftList(NMHDR *pNMHDR, LRESULT *pResult) {
    // empty
    *pResult = 0;
}


BOOL CLeftView::PreTranslateMessage(MSG* pMsg){
    bool bProcessed =false;
    if(pMsg->message == WM_KEYDOWN){
        // catch all keyboard messages
        // check for special keys
        if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_TAB || 
           pMsg->wParam == VK_RIGHT){
            ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->wndSplitter.GetPane(0,1)->SetFocus();
            bProcessed = true;
        }

        if(pMsg->wParam == VK_ESCAPE){
            getConfig()->read();
            ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
            bProcessed = true;
        }

    };

    if(pMsg->message == WM_SYSKEYDOWN){
        // if ALT was pressed set focus to right panel
        ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->wndSplitter.GetPane(0,1)->SetFocus();
        bProcessed = true;
    }

    if(bProcessed)
      return TRUE;
    else
      return CFormView::PreTranslateMessage(pMsg);
}


/**
 * Set the background image under the desired item of imageList [0 - n].
 */
void CLeftView::setBkgImage(int itemNumber) { 

    int dpiX = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiX();
    int dpiY = ((CMainSyncFrame*)AfxGetMainWnd())->getDpiY();

    // compute bk image offset
    CRect rect1, rect2;
    GetDlgItem(IDC_LEFT_LIST)->GetWindowRect(&rect1);
    ScreenToClient(&rect1);
    GetDlgItem(IDC_SEP1)->GetWindowRect(&rect2);
    ScreenToClient(&rect2);

    yOffset =  itemNumber * (int)(rect2.BottomRight().y / ((double)rect1.Height() -64) * 100.0);

    
   //lstConfig.SetBkImage(((CMainSyncFrame*)AfxGetMainWnd())->hBkgImageConfig, 0, 3, yOffset);
   HBITMAP hBmpBkg = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LEFT_BUTTON));
   lstConfig.SetBkImage(hBmpBkg, 0, 3, yOffset);
   DeleteObject(hBmpBkg);
   
}