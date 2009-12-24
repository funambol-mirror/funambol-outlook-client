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
/** @addtogroup UI */
/** @{ */

#include "afxwin.h"
#include "afxcmn.h"
#include "AnimatedIcon.h"

#if !defined(AFX_FORM1_H__FA98B71B_D0B7_11D3_BC39_00C04F602FEE__INCLUDED_)
#define AFX_FORM1_H__FA98B71B_D0B7_11D3_BC39_00C04F602FEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "CustomPane.h"


/**
 * Form of the main window.
 * Contains objects on the main screen of the UI (panes, labels, ...).
 * TODO - refactoring: please use arrays for sources (sourceStatusLabel, iconSource,...)
 */
class CSyncForm : public CFormView
{

private:
    // true if the UI buttons are locked
    bool lockedUI;

    /// The current number of panes displayed (4 or 5)
    int panesCount;

    // Buffers for source panes status labels
    CString contactsStatusLabel;
    CString calendarStatusLabel;
    CString tasksStatusLabel;
    CString notesStatusLabel;
    CString picturesStatusLabel;

    // Buffers for source panel titles (fixed)
    CString contactsLabel;
    CString calendarLabel;
    CString tasksLabel;
    CString notesLabel;
    CString picturesLabel;


protected:

    // protected constructor used by dynamic creation
	CSyncForm();
	DECLARE_DYNCREATE(CSyncForm)

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnDraw(CDC* pDC);
    virtual ~CSyncForm();

#ifdef _DEBUG
     virtual void AssertValid() const;
     virtual void Dump(CDumpContext& dc) const;
#endif

   // Generated message map functions
   DECLARE_MESSAGE_MAP()


public:

	//{{AFX_DATA(CSyncForm)
	enum { IDD = IDD_SYNC_FORM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

    CFont fontBold;
    CFont fontNormal;
    CAnimatedIcon butStart;

    CAnimatedIcon iconContacts;
    CAnimatedIcon iconCalendar;
    CAnimatedIcon iconTasks;
    CAnimatedIcon iconNotes;
    CAnimatedIcon iconPictures;

    CBrush  brushHollow;

    // right hand status icons
    CAnimatedIcon iconStatusContacts;
    CAnimatedIcon iconStatusCalendar;
    CAnimatedIcon iconStatusTasks;
    CAnimatedIcon iconStatusNotes;
    CAnimatedIcon iconStatusPictures;
    CAnimatedIcon iconStatusSync;

    // panes
    CCustomPane paneSync;
    CCustomPane paneContacts;
    CCustomPane paneCalendar;
    CCustomPane paneTasks;
    CCustomPane paneNotes;
    CCustomPane panePictures;

    // sync source states {SYNCSOURCE_STATE_OK, SYNCSOURCE_STATE_NOT_SYNCED, SYNCSOURCE_STATE_CANCELED}
    // defined in winmaincpp.h        
    int syncSourceContactState;
    int syncSourceCalendarState;
    int syncSourceTaskState;
    int syncSourceNoteState;
    int syncSourcePictureState;

    
    /**
    * refresh UI info about a source
    * @param sourceId : the source type id {SYNCSOURCE_CONTACTS,..}, defined in ClientUtil.h
    */
    void refreshSource(int sourceId);

    // refresh UI info about all sources, calls refreshSource for every sync source
    void refreshSources();
    
    /**
    * repaints sync controls from a pane associated to a source
    * @param paneType : the pane type id {PANE_TYPE_CONTACTS, ..}, defined in CustomPane.h
    */
    void repaintPaneControls(int paneType);

    // shows/hides the sync controls
    void showSyncControls(BOOL show);

    void lockButtons();
    void unlockButtons();

    // change UI status text for a source
    void changeContactsStatus(CString& status);
    void changeCalendarStatus(CString& status);
    void changeTasksStatus   (CString& status);
    void changeNotesStatus   (CString& status);
    void changePicturesStatus(CString& status);

    afx_msg LRESULT OnInitForm(WPARAM, LPARAM);
    afx_msg void OnBnClickedMainButSync();
    afx_msg void OnNcPaint( );
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

    // when main sync pane is clicked
    afx_msg void OnStnClickedMainBkSync();

    // when sync source panes are clicked
    afx_msg void OnStnClickedMainBkContacts();
    afx_msg void OnStnClickedMainBkCalendar();
    afx_msg void OnStnClickedMainBkTasks();
    afx_msg void OnStnClickedMainBkNotes();
    afx_msg void OnStnClickedMainBkPictures();
};


/** @} */
/** @endcond */
#endif
