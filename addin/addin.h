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



#ifndef __ADDIN_H_
#define __ADDIN_H_

#include "resource.h"      // main symbols
#include "customization.h"

// ----------------------- definitions -------------------------------
#define PLUGIN_UI_CLASSNAME                 "FunambolApp"                   // The UI windows classname
#define FUN                                 "FUN"
#define PARAM_OUTLOOK_SYNC                  "sync"                          // The command-line parameter passed to PROGRAM_NAME_EXE application
                                                                            // It's used to start automatically the sync.
#define PARAM_OUTLOOK_OPTIONS               "options"                       // It's used to open automatically the options dialog.


#define BUTTON_SYNCHRONIZE                 L"&Sync All        Ctrl+F7"
#define BUTTON_GOTO_PLUGIN                 L"&Go to...        Ctrl+F8"
#define BUTTON_CONFIGURATION               L"Op&tions...     Ctrl+F9"
#define TOOLTIP                            L"Sync All"


//
// Win registry keys:
// ------------------
// HKLM keys -> general (read only) settings
// HKCU keys -> current user settings
//
#define PROPERTY_STATE                      "State"                     // The state of addin
#define PROPERTY_PATH                       "installDir"                // The path of application
#define PROPERTY_NUM_INSTANCES              "numInstances"              // #instances of Addin for different users
#define PROPERTY_SW_VERSION                 "swv"                       // Software version


// Possible addin states:
#define ADDIN_STATE_OK                      "ok"
#define ADDIN_STATE_INSTALLING              "installing"
#define ADDIN_STATE_IN_PROGRESS             "in progress"
#define ADDIN_STATE_FAILED                  "failed"
#define ADDIN_STATE_UNINSTALLED             "uninstalled"


#define LOG_FILENAME                        "FunambolAddin.log"
#define LOG_DEFAULT_PATH                    "C:"                // Normally not used, see openlog()
#define TEMP_ENV                            "TEMP"
#define MAX_ADDIN_LOG_SIZE                  1000000                     // 1 MB

// These is the standard Funambol product label.
// It's used to double-check if a Funambol addin menu is still there (it may happen
// if the current addin has a customized label and a standard Funambol addin was istalled before)
// DON'T CHANGE these unless the standard product defines have changed!
#define ADDIN_MENU_LABEL_FUNAMBOL           L"Funa&mbol"

// This is the commandbar icon caption. It's used to create/retrieve the addin icon, 
// from the Outlook toolbar. THIS VALUE IS NOT VISIBLE TO THE USER, so it's not necessary
// to change/customize it (reccomended: don't change it, to ensure a correct uninstall).
#define ADDIN_COMMAND_BAR_CAPTION           L"Funambol Outlook Sync Client"


// Error messages:
#define ERR_OPEN_EXPLORER                   "Error opening Outlook UI."
#define ERR_COM_POINTER                     "COM Pointer Error. Code = %08lx: %ls"
#define ERR_OPEN_APPLICATION                "Error opening Outlook application."
#define ERR_GET_COMMANDBARS                 "Error getting CommandBars."
#define ERR_UNREG_DLL                       "Could not unregister Addin DLL."
#define ERR_ADD_NEW_MENUBAR                 "Error adding a new MenuBar."
#define ERR_ADD_NEW_COMMANDBAR              "Error adding a new CommandBar."
#define ERR_GET_COMMANDBAR                  "Error getting CommandBar."
#define ERR_GET_MENUBAR                     "Error getting MenuBar."
#define ERR_ADD_BUTTON1                     "Error adding first Button."
#define ERR_ADD_BUTTON2                     "Error adding second Button."
#define ERR_ADD_BUTTON3                     "Error adding third Button."
#define ERR_LINK_ICON                       "Error linking Commandbar icon Button."
#define ERR_LINK_BUTTON1                    "Error inking Menubar first Button."
#define ERR_LINK_BUTTON2                    "Error inking Menubar second Button."
#define ERR_LINK_BUTTON3                    "Error inking Menubar third Button."
#define ERR_UNLINK_ICON                     "Error un-linking Commandbar icon Button."
#define ERR_UNLINK_BUTTON1                  "Error un-inking Menubar first Button."
#define ERR_UNLINK_BUTTON2                  "Error un-inking Menubar second Button."
#define ERR_UNLINK_BUTTON3                  "Error un-inking Menubar third Button."
#define ERR_REMOVING_ADDIN                  "Error occurred removing Addin from Outlook."
#define ERR_INSTALL_DIR_KEY                 "Could not retrieve path of %s under reg key %s/%s."

#define ERR_CODE_BAD_POINTER                0x80004003



// -----------------------------------------------------------------

extern _ATL_FUNC_INFO OnClickButtonInfo;
extern _ATL_FUNC_INFO OnClickSynchronizationInfo;
extern _ATL_FUNC_INFO OnClickGotoInfo;
extern _ATL_FUNC_INFO OnClickConfigurationInfo;

static void toWindows(char* str);


//
// ------------------------------- Caddin class --------------------------------------------
//
class ATL_NO_VTABLE Caddin : public CComObjectRootEx<CComSingleThreadModel>,
                             public CComCoClass<Caddin, &CLSID_addin>,
                             public ISupportErrorInfo,
                             public IDispatchImpl<Iaddin, &IID_Iaddin, &LIBID_FUNAMBOLADDINLib>,
                             public IDispatchImpl<_IDTExtensibility2, &IID__IDTExtensibility2, &LIBID_AddInDesignerObjects>,
                             public IDispEventSimpleImpl<1,Caddin,&__uuidof(_CommandBarButtonEvents)>,
                             public IDispEventSimpleImpl<2,Caddin,&__uuidof(_CommandBarButtonEvents)>,
                             public IDispEventSimpleImpl<3,Caddin,&__uuidof(_CommandBarButtonEvents)>,
                             public IDispEventSimpleImpl<4,Caddin,&__uuidof(_CommandBarButtonEvents)>
{

public:

    typedef IDispEventSimpleImpl</*nID =*/ 1,Caddin, &__uuidof(_CommandBarButtonEvents)> ButtonSyncEvent;
    typedef IDispEventSimpleImpl</*nID =*/ 2,Caddin, &__uuidof(_CommandBarButtonEvents)> ItemSynchronizationEvent;
    typedef IDispEventSimpleImpl</*nID =*/ 3,Caddin, &__uuidof(_CommandBarButtonEvents)> ItemGotoEvent;
    typedef IDispEventSimpleImpl</*nID =*/ 4,Caddin, &__uuidof(_CommandBarButtonEvents)> ItemConfigurationEvent;

    Caddin(){}
    virtual ~Caddin() {}

    DECLARE_REGISTRY_RESOURCEID(IDR_ADDIN)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(Caddin)
        COM_INTERFACE_ENTRY(Iaddin)
        //DEL 	COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY2(IDispatch, Iaddin)
        COM_INTERFACE_ENTRY(_IDTExtensibility2)
    END_COM_MAP()

    BEGIN_SINK_MAP(Caddin)
        SINK_ENTRY_INFO(1, __uuidof(_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickButton,          &OnClickButtonInfo)
        SINK_ENTRY_INFO(2, __uuidof(_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickSynchronization, &OnClickSynchronizationInfo)
        SINK_ENTRY_INFO(3, __uuidof(_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickGoto,            &OnClickGotoInfo)
        SINK_ENTRY_INFO(4, __uuidof(_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickConfiguration,   &OnClickConfigurationInfo)
    END_SINK_MAP()

    // ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    // Iaddin
    void __stdcall OnClickButton         (IDispatch* /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);
    void __stdcall OnClickSynchronization(IDispatch* /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);
    void __stdcall OnClickGoto           (IDispatch* /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);
    void __stdcall OnClickConfiguration  (IDispatch* /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);


public:

    // _IDTExtensibility2
    STDMETHOD(OnConnection)     (IDispatch* Application, ext_ConnectMode ConnectMode, IDispatch* AddInInst, SAFEARRAY** custom);
    STDMETHOD(OnDisconnection)  (ext_DisconnectMode RemoveMode, SAFEARRAY** custom);
    STDMETHOD(OnStartupComplete)(SAFEARRAY** custom);
    STDMETHOD(OnBeginShutdown)  (SAFEARRAY** custom);
    STDMETHOD(OnAddInsUpdate)   (SAFEARRAY** custom){ return E_NOTIMPL; }

    HRESULT AddNewMenubar   (_CommandBars *pCmdBars);
    HRESULT AddNewCommandBar(_CommandBars *pCmdBars);

    HRESULT __stdcall Caddin::DispAdviseControls  (void);
    HRESULT __stdcall Caddin::DispUnadviseControls(void);

    void  launchSyncClientOutlook  (const char* parameter);
    char* readPropertyValueFromHKLM(const char* context, const char* prop);
    int   setCurrentSwv();
    char* readAddinSwv();
    bool  swvNotCompatible(const char* currentVersion, const char* oldVersion);

    HRESULT removeAddin();
    bool    isAddinInstalled();

    int     saveAddinState(char* state);
    bool    checkErrorsLastTime();
    void    errorMsgBox();
    void    openLog();
    void    manageComErrors(_com_error &e);

    bool    isLastInstance();
    void    updateAddinInstances(bool increment);


private:
    LPDISPATCH      m_pParentApp;
    _ApplicationPtr applicationPtr;

};

#endif //__ADDIN_H_
