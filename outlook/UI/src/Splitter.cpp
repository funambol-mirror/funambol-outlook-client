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

// Splitter.cpp : implementation file
//

#include "stdafx.h"
#include "Splitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//In the function CSplitter::ReplaceView pDoc->AutoDelete has to be changed to prevent
//the one and only doc object from being deleted.

CSplitter::CSplitter()
{
    m_bBarLocked=TRUE; 
    bSplitterVisible = FALSE;
}

CSplitter::~CSplitter()
{
}


BEGIN_MESSAGE_MAP(CSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitter)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
    ON_WM_PAINT()
    //ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSplitter message handlers

void CSplitter::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bBarLocked)
		CSplitterWnd::OnLButtonDown(nFlags, point);
}

void CSplitter::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bBarLocked)
		CSplitterWnd::OnMouseMove(nFlags, point);
	else
		CWnd::OnMouseMove(nFlags, point);
}

BOOL CSplitter::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (!m_bBarLocked)
		return CWnd::OnSetCursor(pWnd, nHitTest, message);

	return CSplitterWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CSplitter::ReplaceView(int row, int col,CRuntimeClass * pViewClass,SIZE size)
{
  CCreateContext context;
  BOOL bSetActive;

  if ((GetPane(row,col)->IsKindOf(pViewClass))==TRUE)
       return FALSE;				    
   
   // Get pointer to CDocument object so that it can be used in the creation 
   // process of the new view
   CDocument * pDoc= ((CView *)GetPane(row,col))->GetDocument();
   CView * pActiveView=GetParentFrame()->GetActiveView();
   if (pActiveView==NULL || pActiveView==GetPane(row,col))
      bSetActive=TRUE;
   else
      bSetActive=FALSE;

    // set flag so that document will not be deleted when view is destroyed
	pDoc->m_bAutoDelete=FALSE;    
    // Delete existing view 
   ((CView *) GetPane(row,col))->DestroyWindow();
    // set flag back to default 
    pDoc->m_bAutoDelete=TRUE;
 
    // Create new view                      
   
   context.m_pNewViewClass=pViewClass;
   context.m_pCurrentDoc=pDoc;
   context.m_pNewDocTemplate=NULL;
   context.m_pLastView=NULL;
   context.m_pCurrentFrame=NULL;
   
   CreateView(row,col,pViewClass,size, &context);
   
   CView * pNewView= (CView *)GetPane(row,col);
   
   if (bSetActive==TRUE)
      GetParentFrame()->SetActiveView(pNewView);
   
   RecalcLayout(); 
   GetPane(row,col)->SendMessage(WM_PAINT);

   return TRUE;
}


void CSplitter::OnDrawSplitter(CDC *pDC, CSplitterWnd::ESplitType nType, const CRect &rect)
{
    // here we choose if we want to show the splitter or not
    if(! this->bSplitterVisible){
        m_cxBorder = 0;
        m_cxSplitterGap = 1;
    }
    else{
        m_cxBorder = 2;
        m_cxSplitterGap = 3;
    }
    
    CSplitterWnd::OnDrawSplitter(pDC, nType,&rect);
}


int CSplitter::OnCreate(LPCREATESTRUCT lpCreateStruct){
    return 0;
}

void CSplitter::OnPaint(){
    Invalidate();
    CSplitterWnd::OnPaint();
}
