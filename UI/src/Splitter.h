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

#if !defined(AFX_Splitter_H__61D2F7E7_7AAA_11D1_9F4C_008029E98A75__INCLUDED_)
#define AFX_Splitter_H__61D2F7E7_7AAA_11D1_9F4C_008029E98A75__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif 


class CSplitter : public CSplitterWnd
{
// Construction
public:
	CSplitter();

private:
    BOOL m_bBarLocked;
    
public:
    BOOL bSplitterVisible;
    BOOL IsBarLocked(){return m_bBarLocked;}
    void LockBar(BOOL bState=TRUE){m_bBarLocked=bState;}
    BOOL ReplaceView(int row, int col,CRuntimeClass * pViewClass,SIZE size);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitter)
	//}}AFX_VIRTUAL

public:
	virtual ~CSplitter();

protected:
	//{{AFX_MSG(CSplitter)
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();

    virtual void OnDrawSplitter( CDC* pDC, ESplitType nType,  const CRect& rect );

	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_Splitter_H__61D2F7E7_7AAA_11D1_9F4C_008029E98A75__INCLUDED_)
