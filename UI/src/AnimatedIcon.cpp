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

// AnimatedIcon.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "AnimatedIcon.h"
#include "SyncForm.h"
#include "MainSyncFrm.h"

IMPLEMENT_DYNAMIC(CAnimatedIcon, CStatic)

CAnimatedIcon::CAnimatedIcon()
	: CStatic()
{
    counterAnim = 0;
}

CAnimatedIcon::~CAnimatedIcon()
{
}

void CAnimatedIcon::DoDataExchange(CDataExchange* pDX)
{
	CStatic::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAnimatedIcon, CStatic)
    ON_WM_TIMER()
    ON_WM_PAINT()
END_MESSAGE_MAP()


// CAnimatedIcon message handlers

void CAnimatedIcon::OnTimer(UINT_PTR nIDEvent ){
    // set icon depending on animation stage
    this->SetIcon(NULL);

    if(counterAnim == 4) {
        counterAnim = 0;
    }
    switch(counterAnim){
        case 0:
            this->SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32A)));
            break;
        case 1:
            this->SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32B)));
            break;
        case 2:
            this->SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32C))); 
            break;
        case 3:
            this->SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32D)));
            break;
    }
    counterAnim++;
}


void CAnimatedIcon::Animate(){
    // set arrow icon, then start timer for animation
    SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32A)));
    state = STATE_ICON_SYNC;
    this->SetTimer(ANIM_ICON_ARROWS, ANIM_ICON_DELAY, NULL);
}

void CAnimatedIcon::StopAnim(){
    // kill timer for this icon, stopping the animation
    state = STATE_DONE;
    this->KillTimer(ANIM_ICON_ARROWS);
}

void CAnimatedIcon::OnPaint(){
    // paint the icon, so it will be transparent over the background bitmap
    CPaintDC dc(this);        
    dc.DrawIcon(0,0, GetIcon());
}