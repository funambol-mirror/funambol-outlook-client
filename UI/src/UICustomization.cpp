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
#include "UICustomization.h"

const bool UICustomization::forceUseSubfolders             = false;
const bool UICustomization::clearAnchorsOnFolderChange     = false;
const bool UICustomization::hideDataFormats                = false;

const bool UICustomization::shared                         = false;

const bool UICustomization::lockCalendarFilter             = false;
const int  UICustomization::lockCalendarFilterValue        = 3;

const bool UICustomization::showWarningOnChangeFromOneWay  = false;

const int  UICustomization::syncAllTextRed                 = 255;
const int  UICustomization::syncAllTextGreen               = 255;
const int  UICustomization::syncAllTextBlue                = 255;

const bool   UICustomization::attachOption                 = false;

const bool   UICustomization::defaultFullSyncFromClient    = true;
const bool   UICustomization::confirmOnRefreshFromClient   = false;
const bool   UICustomization::confirmOnRefreshFromServer   = true;

// Log options
const bool   UICustomization::logRotateOptions             = false;
const bool   UICustomization::sendLogs                     = false;


const bool   UICustomization::showWelcomeMessage           = false;
const bool   UICustomization::showUpgradingMessage         = false;