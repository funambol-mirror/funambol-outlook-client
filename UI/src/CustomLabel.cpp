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


// CustomLabel.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "CustomLabel.h"

IMPLEMENT_DYNAMIC(CCustomLabel, CStatic)

CCustomLabel::CCustomLabel()
	: CStatic()
{
    brush.CreateSolidBrush(COLOR_BRUSH);
    bMouseCaptured = false;
    clrLinkText = COLOR_LINK_NORMAL;
}

CCustomLabel::~CCustomLabel()
{
}

void CCustomLabel::DoDataExchange(CDataExchange* pDX)
{
	CStatic::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCustomLabel, CStatic)
    ON_WM_CTLCOLOR()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()


// CCustomLabel message handlers

void CCustomLabel::OnPaint(){
    CStatic::OnPaint();
}

HBRUSH CCustomLabel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
  HBRUSH hbr = CStatic::OnCtlColor(pDC, pWnd, nCtlColor);

  hbr = (HBRUSH) brush.GetSafeHandle();
   
  return hbr;
}

void CCustomLabel::OnMouseMove(UINT nFlags, CPoint point ){
    if(! bMouseCaptured){
        TRACKMOUSEEVENT Tme;
        Tme.cbSize = sizeof(TRACKMOUSEEVENT);
        Tme.dwFlags = TME_LEAVE;
        Tme.hwndTrack = m_hWnd;
        int Result = TrackMouseEvent(&Tme); 

        clrLinkText = COLOR_LINK_HOVER;
        SetFont(&fontHover);
        bMouseCaptured = true;
        Invalidate();
    };    
    CStatic::OnMouseMove(nFlags, point);
}

LRESULT CCustomLabel::OnMouseLeave(WPARAM wParam, LPARAM lParam){
    bMouseCaptured = false;
    clrLinkText = COLOR_LINK_NORMAL;
    SetFont(&fontNormal);
    Invalidate();
    return 0;
}

void CCustomLabel::init(){
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));      
    GetFont()->GetLogFont(&lf);
    lf.lfWeight = FW_BOLD; lf.lfUnderline = TRUE;
    VERIFY(fontNormal.CreateFontIndirect(&lf));  

    VERIFY(fontHover.CreateFontIndirect(&lf));  

    SetFont(&fontNormal);
    
}