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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
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
#pragma once

class Popup : public CDialog
{
	DECLARE_DYNAMIC(Popup)

public:
	Popup(CWnd* pParent = NULL);   
	virtual ~Popup();

    void setTitle       (CString value)     { title         = value;}
    void setMessage     (CString value)     { msg           = value;}
    void setbutton1     (CString value)     { buttonText1   = value;}
    void setbutton2     (CString value)     { buttonText2   = value;}
    void setbutton3     (CString value)     { buttonText3   = value;}
    int  getResult      ()                  { return ret;           }

	enum { IDD = IDD_POPUP };

private:
    CString title;
    CString msg;
    CString buttonText1;
    CString buttonText2;
    CString buttonText3;

    int ret;
    
    

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnInitDialog();
    HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedCancel2();
    void drawPopup();
   
    
};


/**
 * CMessageBox is a method that creates a Modal Dialog, not full screen, 
 * with three buttons.
 * To create a new cmessagebox provide the message to be display and the names
 * of the three buttons. The method returns an int to point the button choosen 
 * by the user. 0 for button1, 1 for button 2 ant 2 for button 3.

 * @param CString msg - the message to display
 * @param CString button1 - the label of button1
 * @param CString button2 - the label of button2
 * @param CString button3 - the label of button3
 * @return int - 0 for button1, 1 for button 2, 2 for button 3.
 */
int CMessageBox(CString msg, CString button1, CString button2, CString button3);

/*
int LoadIconModal();
void RemoveIconModal();
*/
