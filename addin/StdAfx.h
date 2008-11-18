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


// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__0FE99A0F_E50B_4FF2_9F55_85C581DF38F7__INCLUDED_)
#define AFX_STDAFX_H__0FE99A0F_E50B_4FF2_9F55_85C581DF38F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>



// Import libraries: Outlook Object Model and Redemption.
// --------------------------------------------------------
// Type Libraries are referenced by their unique LIBIDs.
// Note:
// LIBRARIES MUST BE REGISTERED IN THE SYSTEM.
// mso.dll, msoutl.olb, msaddndr.dll: are registered during Microsoft Outlook installation

// This is LIBID for 'mso.dll'
#import "libid:2DF8D04C-5BFA-101B-BDE5-00AA0044DE52" rename_namespace("Office") named_guids \
        rename("DocumentProperties", "OlDocumentProperties") \
        rename("RGB", "OlRGB")

// This is LIBID for 'msoutl.olb'
#import "libid:00062FFF-0000-0000-C000-000000000046" rename_namespace("Outlook") \
        raw_interfaces_only, named_guids \
        rename("CopyFile", "OlCopyFile")

// This is LIBID for 'MSADDNDR.DLL'
#import "libid:AC0714F2-3D04-11D1-AE7D-00A0C90F26F4" raw_interfaces_only, \
        raw_native_types, no_namespace, named_guids

using namespace Office;
using namespace Outlook;



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0FE99A0F_E50B_4FF2_9F55_85C581DF38F7__INCLUDED)
