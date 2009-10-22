 ;
 ; Funambol is a mobile platform developed by Funambol, Inc.
 ; Copyright (C) 2003 - 2007 Funambol, Inc.
 ;
 ; This program is free software; you can redistribute it and/or modify it under
 ; the terms of the GNU Affero General Public License version 3 as published by
 ; the Free Software Foundation with the addition of the following permission
 ; added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 ; WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE
 ; WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 ;
 ; This program is distributed in the hope that it will be useful, but WITHOUT
 ; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 ; FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 ; details.
 ;
 ; You should have received a copy of the GNU Affero General Public License
 ; along with this program; if not, see http://www.gnu.org/licenses or write to
 ; the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 ; MA 02110-1301 USA.
 ;
 ; You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite
 ; 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 ;
 ; The interactive user interfaces in modified source and object code versions
 ; of this program must display Appropriate Legal Notices, as required under
 ; Section 5 of the GNU Affero General Public License version 3.
 ;
 ; In accordance with Section 7(b) of the GNU Affero General Public License
 ; version 3, these Appropriate Legal Notices must retain the display of the
 ; "Powered by Funambol" logo. If the display of the logo is not reasonably
 ; feasible for technical reasons, the Appropriate Legal Notices must display
 ; the words "Powered by Funambol".
 ;

; customization params
!include "customization.ini"

; ------ defines ------
!define PRODUCT_NAME_EXE                        "OutlookPlugin.exe"
!define MICROSOFT_OUTLOOK                       "Microsoft Outlook"
!define MICROSOFT_OUTLOOK_CLASS_NAME            "rctrl_renwnd32"
!define PLUGIN_UI_CLASS_NAME                    "FunambolApp"

!define PRODUCT_UNINST_ROOT_KEY                 "HKLM"
!define PRODUCT_STARTMENU_REGVAL                "NSIS:StartMenuDir"
!define PLUGIN_REGKEY_CONTEXT                   "Software\${PLUGIN_ROOT_CONTEXT}"
!define ADDIN_REGKEY_CONTEXT                    "Software\Microsoft\Office\Outlook\Addins\FunambolAddin.Addin"
!define PRODUCT_DIR_REGKEY                      "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME_EXE}"
!define PRODUCT_UNINST_KEY                      "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define MSMAPIAPPS_REGKEY_CONTEXT               "Software\Microsoft\Windows Messaging Subsystem\MSMapiApps"
!define SHELLFOLDERS_CONTEXT                    "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders"
!define PROPERTY_PATH                           "installDir"
!define PROPERTY_ADDIN_NAME                     "FileName"
!define PROPERTY_SWV                            "swv"
!define PROPERTY_SP                             "portal"
!define PROPERTY_CUSTOMER                       "Customer"
!define PROPERTY_DESCRIPTION                    "Description"
!define PROPERTY_FUNAMBOL_SWV                   "funambol_swv"

; Before v.7.1.4 the product name was "Funambol Outlook Plug-in"
; We want to be able to upgrade the Client from versions < 7.1.4.
!define OLD_PRODUCT_NAME                        "Funambol Outlook Plug-in"
!define OLD_PRODUCT_UNINST_KEY                  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${OLD_PRODUCT_NAME}"
!define OLD_STARTMENU_CONTEXT                   "Funambol\Outlook Plug-in"
!define OLD_INSTALLDIR_CONTEXT                  "Funambol\Outlook Plug-in"
!define OLD_PLUGIN_UI_TITLE                     "Funambol Outlook Plug-in"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING                        ; Show a message box with a warning when the user wants to close the installer.
!define MUI_ABORTWARNING_CANCEL_DEFAULT         ; Set the Cancel button as the default button on the message box.
!define MUI_ICON                                "fileset\images\install.ico"
!define MUI_UNICON                              "fileset\images\uninstall.ico"

; Welcome page
!define MUI_WELCOMEFINISHPAGE_BITMAP            "${PRODUCT_WELCOME_BMP}"
!insertmacro MUI_PAGE_WELCOME

; License page
!ifdef SHOW_LICENSE
    !define MUI_LICENSEPAGE_CHECKBOX
    !insertmacro MUI_PAGE_LICENSE                   "fileset\LICENSES\License.txt"
!endif

; Directory page, the first two define check if the install dir is correct
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!insertmacro MUI_PAGE_DIRECTORY

var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER         "${STARTMENU_CONTEXT}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT         "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY          "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME    "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN                      "$INSTDIR\${PRODUCT_NAME_EXE}"
!ifdef FINISHPAGE_SHOW_README
    !define MUI_FINISHPAGE_SHOWREADME               "$INSTDIR\Readme.txt"
    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!endif
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!define MUI_UNABORTWARNING
UninstPage custom un.RemoveUserData             ; Custom page, to ask if deleting users files/settings.
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE                       "English"
; MUI end ------



Name              "${PRODUCT_NAME} ${PRODUCT_VERSION}"                       ; PRODUCT_VERSION passed as parameter by build.xml
OutFile           "..\output\${FILE_NAME}-${PRODUCT_VERSION}.exe"
InstallDir        "$PROGRAMFILES\${INSTALLDIR_CONTEXT}"

InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails   show
ShowUnInstDetails show
Icon              "${MUI_ICON}"



; Check if OUTLOOK.EXE process is running
Function CheckMicrosoftApp
        ; First try to close Outlook automatically.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "I need to close ${MICROSOFT_OUTLOOK} to proceed with the installation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 1500                                           ; wait 1500 ms for Outlook closing

        ; If Outlook still running, ask to close it manually.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "Could not close ${MICROSOFT_OUTLOOK}. Please close it manually to proceed with the installation of $(^Name)." IDCANCEL Cancel
        
  checkProcess:
        ; Check if OUTLOOK.EXE is still running.
        push "OUTLOOK.EXE"
        processwork::existsprocess
        pop $1
        IntCmp $1 0 done          ; exit only when OUTLOOK.EXE process is not found
        
        ; If OUTLOOK.EXE still running, it's probably stuck: kill the process silently!
        push "OUTLOOK.EXE"
        processwork::KillProcess
        Sleep 500
        goto checkProcess

  done:
        Return
  Cancel:
        MessageBox MB_OK "Installation aborted."
        Abort
FunctionEnd


; Check if "OUTLOOK.EXE" process is running (for uninstaller)
Function un.CheckMicrosoftApp
        ; First try to close Outlook automatically.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "I need to close ${MICROSOFT_OUTLOOK} to proceed with the uninstallation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 1500                                           ; wait 1500 ms for Outlook closing

        ; If Outlook still running, ask to close it manually.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "Could not close ${MICROSOFT_OUTLOOK}. Please close it manually to proceed with the uninstallation of $(^Name)." IDCANCEL Cancel

  checkProcess:
        ; Check if OUTLOOK.EXE is still running.
        push "OUTLOOK.EXE"
        processwork::existsprocess
        pop $1
        IntCmp $1 0 done          ; exit only when OUTLOOK.EXE process is not found

        ; If OUTLOOK.EXE still running, it's probably stuck: kill the process silently!
        push "OUTLOOK.EXE"
        processwork::KillProcess
        Sleep 500
        goto checkProcess

  done:
        Return
  Cancel:
        MessageBox MB_OK "Uninstallation failed."
        Abort
FunctionEnd



; Check if "OutlookPlugin.exe" is running
Function CheckFunClientApp
        ; First try to close Outlook plugin automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "I need to close ${PRODUCT_NAME} to proceed with the installation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If plugin still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "Could not close ${PRODUCT_NAME}. Please close it manually to proceed with the installation of $(^Name)." IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "Installation aborted."
        Abort
FunctionEnd


; Check if "OutlookPlugin.exe" is running - for OLD Client versions (< 7.1.4)
Function CheckOldFunClientApp
        ; First try to close Outlook plugin automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${OLD_PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "I need to close ${OLD_PRODUCT_NAME} to proceed with the installation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If plugin still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${OLD_PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "Could not close ${OLD_PRODUCT_NAME}. Please close it manually to proceed with the installation of $(^Name)." IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "Installation aborted."
        Abort
FunctionEnd


; Check if "OutlookPlugin.exe" is running (for uninstaller)
Function un.CheckFunClientApp
        ; First try to close Outlook plugin automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "I need to close ${PRODUCT_NAME} to proceed with the uninstallation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If plugin still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "Could not close ${PRODUCT_NAME}. Please close it manually to proceed with the uninstallation of $(^Name)." IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "Uninstallation failed."
        Abort
FunctionEnd



; Check if current user have Administrator rights to run the installer.
; If not, the installer will be aborted.
Function CheckUserRights

      ; If user can write this, it's an Admin ;)
      WriteRegStr  HKLM   "${PLUGIN_REGKEY_CONTEXT}"   "${PROPERTY_PATH}"   "$INSTDIR"
      IfErrors 0 +3
      MessageBox MB_OK "You need Administrator rights to install ${PRODUCT_NAME}."
      Abort
      
FunctionEnd


; Check if current user have Administrator rights to run the uninstaller.
; If not, the uninstaller will be aborted.
Function un.CheckUserRights

      ; If user can write this, it's an Admin ;)
      WriteRegStr  HKLM   "${PLUGIN_REGKEY_CONTEXT}"   "${PROPERTY_PATH}"   "$INSTDIR"
      IfErrors 0 +3
      MessageBox MB_OK "You need Administrator rights to uninstall ${PRODUCT_NAME}."
      Abort

FunctionEnd



;
; Test if the application is already installed.
; If yes, manage the upgrade of the plugin.
;
Function CheckAppInstalled

       ReadRegStr $R8 HKLM "${PLUGIN_REGKEY_CONTEXT}" "${PROPERTY_CUSTOMER}"
       ReadRegStr $R7 HKLM "${PLUGIN_REGKEY_CONTEXT}" "${PROPERTY_SWV}"
       
       StrCmp $R8 "" +1 +2 ;if empty we control the swv field to check if no app installed
       StrCmp $R7 "" +2 +1
       StrCmp $R8 "${CUSTOMER}" +1 customerAbort

       ReadRegStr $R0 HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
       StrCmp $R0 "" +3
       ReadRegStr $R1 HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion"         ; $R1 = installed version "x.y.z"
       Goto upgrade
       
       ; since v.7.1.4.
       ; Also check the old product name "Outlook Plug-in"
       ReadRegStr $R0 HKLM "${OLD_PRODUCT_UNINST_KEY}" "UninstallString"
       StrCmp $R0 "" done
       ReadRegStr $R1 HKLM "${OLD_PRODUCT_UNINST_KEY}" "DisplayVersion"     ; $R1 = installed version "x.y.z"
       
       
       ; ------------------- This is an UPGRADE ------------------
   upgrade:
   
       ; $9 (global) now contains the old installdir
       ; It's important because INSTDIR can be changed by the user, but we MUST delete the old files!
       StrCpy $9 $INSTDIR
   
       ;
       ; Compare installed version with current one. We suppose version string
       ; not longer than 12 chars, and in the format "x.y.z"
       ; Loop char by char and compare the version (major - minor - build).
       ;
       StrLen $R4 $R1                                                   ; $R4 = length of installed version
       StrLen $R5 ${PRODUCT_VERSION}                                    ; $R5 = length of current version
       
       Push 0
       Pop $R0                                                          ; $R0 = iterator of chars
   loop:
       StrCpy $R2 $R1 12 $R0                                            ; $R2 = installed version from offset $R0
       StrCpy $R3 ${PRODUCT_VERSION} 12 $R0                             ; $R3 = current version from offset $R0

       IntCmp $R2 $R3   0  newerVersion  olderVersion                   ; (installed v. = current v.)?

       IntOp $R0 $R0 + 1
       IntCmp $R0 $R4 sameVersion                                       ; out if one of the 2 strings has finished
       IntCmp $R0 $R5 sameVersion
       Goto loop


  ; 1. Upgrade to a newer version.
  newerVersion:
       ; (installed v. = 6.0)?
       IntCmp $R1 6   0  from_v3  0

       MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
                  "A previous version of ${PRODUCT_NAME} is already installed (version $R1). $\nPress OK to proceed with the upgrade." \
                  IDCANCEL cancel

       StrCpy $R9 "uninstForUpgrade"     ; Cannot call now: user can cancel installation!

       ; New name since v.7.1.4.
       ; If the old installDir folder is the old default one, let's change it with the new naming.
       StrCmp $INSTDIR "$PROGRAMFILES\${OLD_INSTALLDIR_CONTEXT}" +1 +2
       StrCpy $INSTDIR "$PROGRAMFILES\${INSTALLDIR_CONTEXT}"
       Goto done


  ; 2. Reinstall the same version.
  sameVersion:
       MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
                  "The same version of ${PRODUCT_NAME} is already installed. $\nPress OK to repair the plugin (all users settings will be preserved)." \
                  IDCANCEL cancel

       StrCpy $R9 "uninstForUpgrade"     ; Cannot call now: user can cancel installation!
       Goto done


  ; 3. Downgrade to an older version -> avoid.
  olderVersion:
       MessageBox MB_OK|MB_ICONEXCLAMATION \
                  "A more recent version of ${PRODUCT_NAME} is already installed (version $R1). $\nPlease uninstall it first."
       Abort


  ; 4. Upgrade from a version < v6 (3.0.x) -> avoid.
  from_v3:
       MessageBox MB_OK|MB_ICONEXCLAMATION \
                  "A previous version of ${PRODUCT_NAME} is already installed (version $R1). $\nPlease uninstall it first."
       Abort

  ; 5 Customer abort
  customerAbort:
       MessageBox MB_OK \
                  "A different version of this client is present on this machine. Please unistall it first."
       Abort
  done:
       Return
  cancel:
       Abort
FunctionEnd



; Uninstall plugin for an upgrade.
; Uregister DLLs and delete files required.
; Note: $9 (global) contains the old installdir (may be = $INSTDIR)
Function uninstForUpgrade

     ;
     StrCmp $9 "" +1 +2
     StrCpy $9 $INSTDIR

     ; Unregister DLLs.
     UnRegDLL "$9\Redemption.dll"
     UnRegDLL "$9\FunambolAddin.dll"

     ; Delete files from old installDir.
     Delete "$9\*.*"
     RMDir /r "$9"

     ; Delete application registered from System (version could be changed).
     DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
     DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
     
     ; Addin need to be reinstalled for ALL users (replace menu/bars).
     Call resetUsersAddinState
     
     ;
     ; Versions < 7.1.4.
     ; TODO: better to do the following actions ONLY if $R1 is < 7.1.4.
     ; (now we're always doing it)
     ;
     DeleteRegKey HKLM "${OLD_PRODUCT_UNINST_KEY}"

     ; Delete old startMenu shortcuts
     SetShellVarContext all
     Delete "$SMPROGRAMS\${OLD_STARTMENU_CONTEXT}\*.*"
     RMDir  "$SMPROGRAMS\${OLD_STARTMENU_CONTEXT}"
     


     ; NOTE:
     ; $R1 is the installed product version.
     ; Insert here code to manage upgrades from specific versions...
     ; (e.g. if paths of file names changed)

FunctionEnd


;
; Reset to 'installing' all users registry keys for Outlook Addin.
; Need to cycle on all users trees, as each user stores its keys under HKCU.
;
Function resetUsersAddinState

     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0
  loop:
     EnumRegKey  $R1  HKU  ""  $R0
     StrCmp $R1 "" done                                  ; empty string when finished

     WriteRegStr  HKU   "$R1\${ADDIN_REGKEY_CONTEXT}"   "State"   "installing"

     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd



;
; Install DLL into System.
;
Function installDll

      ; First, try to unregister previous dll (if any).
      UnRegDLL "$WINDIR\FunambolAddin.dll"
      UnRegDLL "$WINDIR\OutlookAddin.dll"
      UnRegDLL "$WINDIR\OutlookAddinRemover.dll"

      ; Register DLL, abort install on error.
      ClearErrors
      SetOutPath "$INSTDIR"
      RegDLL "$INSTDIR\FunambolAddin.dll"
      IfErrors errorDLL1
      RegDLL "$INSTDIR\Redemption.dll"
      IfErrors errorDLL2

      Return

  errorDLL1:
      MessageBox MB_OK "Some error occurred registering FunambolAddin.dll. Installation failed."
      Goto end

  errorDLL2:
      MessageBox MB_OK "Some error occurred registering Redemption.dll. Installation failed."
      Goto end

  end:
      UnRegDLL "$INSTDIR\FunambolAddin.dll"
      UnRegDLL "$INSTDIR\Redemption.dll"
      
      Delete "$INSTDIR\*.*"
      SetOutPath "$WINDIR"
      RMDir /r "$INSTDIR"
      Quit
  
FunctionEnd


Function .onInit
      Call CheckUserRights
      Call CheckAppInstalled
      Call CheckMicrosoftApp
      Call CheckFunClientApp
      Call CheckOldFunClientApp
FunctionEnd


; --------------------------------- MAIN SECTION -----------------------------------
Section "MainSection" SEC01

      ; Uninstall previous version if necessary
      StrCmp $R9 "uninstForUpgrade"  0  +2
      Call uninstForUpgrade

      ; Check if in the instdir path there is some slash
      Push $INSTDIR
      Push "/"
      Call StrSlash
      Pop  $R0
      StrCpy $INSTDIR $R0

      ; Check if in the start menu path there is some slash
      Push $ICONS_GROUP
      Push "/"
      Call StrSlash
      Pop  $R0
      StrCpy $ICONS_GROUP $R0
  
  
      ; --- Extract files ---
      SetOutPath "$INSTDIR"
      SetOverwrite on
      File "fileset\*.*"
      
      SetOutPath "$INSTDIR\LICENSES\Redemption"
      File "fileset\LICENSES\*.*"
      File "fileset\LICENSES\Redemption\*.*"
      
      SetOutPath "$INSTDIR\docs\"
      File "fileset\docs\*.*"
      
      SetOutPath "$INSTDIR\images"
      File "fileset\images\*.*"
      
      SetOutPath "$INSTDIR"
      
      File /r "fileset\redist\*.*"


      ; --- Register dll ---
      ; If registration errors, installation fails
      Call installDll
  
      ; Delete the dll of some previous installation
      Delete "$WINDIR\FunambolAddin.dll"
      ; (These 3 could exist for dirty installation of plugins previous then 3.0 stable...)
      Delete "$WINDIR\winmainclientdll.dll"
      Delete "$WINDIR\OutlookAddin.dll"
      Delete "$WINDIR\OutlookAddinRemover.dll"


      ; --- StartMenu shortcuts ---
      SetShellVarContext all
      !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
      CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\${PRODUCT_NAME}.lnk"               "$INSTDIR\${PRODUCT_NAME_EXE}" "" "" "" "" "" "Launch $(^Name)"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"                     "$INSTDIR\uninst.exe"          "" "" "" "" "" "Uninstall $(^Name)"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\Readme.lnk"                        "$INSTDIR\Readme.txt"          "" "" "" "" "" "${PRODUCT_NAME} Readme"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\User Guide.lnk"                    "$INSTDIR\userguide.url"       "" "" "" "" "" "${PRODUCT_NAME} User Guide"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\${PRODUCT_PUBLISHER} Web Site.lnk" "$INSTDIR\website.url"         "" "" "" "" "" "${PRODUCT_PUBLISHER} Web Site"
      !insertmacro MUI_STARTMENU_WRITE_END


      ; Write registry keys
      Call writeRegistry

SectionEnd



; Install application on System
Section -Post
     WriteUninstaller  "$INSTDIR\uninst.exe"
     WriteRegStr HKLM  "${PRODUCT_DIR_REGKEY}"  "" "$INSTDIR\${PRODUCT_NAME_EXE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayName"      "$(^Name)"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "UninstallString"  "$INSTDIR\uninst.exe"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayIcon"      "$INSTDIR\${PRODUCT_NAME_EXE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayVersion"   "${PRODUCT_VERSION}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "URLInfoAbout"     "${PRODUCT_WEB_SITE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "Publisher"        "${PRODUCT_PUBLISHER}"
SectionEnd





; --------------------------------- SECTION Uninstall -----------------------------------

Section Uninstall

     ; No more delete scheduled task -> done ONLY if removing users' data.
     ; Delete "$WINDIR\Tasks\${PRODUCT_NAME}.job"

     ; Unregister DLLs.
     UnRegDLL "$INSTDIR\Redemption.dll"
     UnRegDLL "$INSTDIR\FunambolAddin.dll"

     ; Copy Addin to WinDir and register it.
     ; (MUST keep it after uninstall, to be loaded at next Outlook startup
     ; and clean up buttons/bars - then it will unregister itself).
     CopyFiles /SILENT "$INSTDIR\FunambolAddin.dll" "$WINDIR\FunambolAddin.dll"
     ; MUST copy also ATL library to WINDIR! (could not be installed).
     CopyFiles /SILENT "$INSTDIR\Microsoft.VC80.ATL" "$WINDIR"
     RegDLL "$WINDIR\FunambolAddin.dll"


     ; Delete files from installDir.
     Delete "$INSTDIR\*.*"

     ; Delete startMenu shortcuts
     SetShellVarContext all
     !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
     Delete "$SMPROGRAMS\$ICONS_GROUP\*.*"


     ; Delete recursively empty folders created on install.
     RMDir /r "$INSTDIR"
     StrCpy $R4 "$INSTDIR"
  loop1:
     Push "$R4"
     Call un.GetLastStrPart
     Pop $R0
     StrCmp $R4 $R0 +5
     StrCmp "$PROGRAMFILES" $R0 +4
     StrCpy $R4 $R0
     RMDir "$R0"
     Goto loop1

     ; Delete recursively empty folders in start menu created on install.
     RMDir "$SMPROGRAMS\$ICONS_GROUP"
     StrCpy $R4 "$ICONS_GROUP"
  loop:
     Push "$R4"
     Call un.GetLastStrPart
     Pop $R0
     StrCmp $R4 $R0 +4
     StrCpy $R4 $R0
     RMDir "$SMPROGRAMS\$R0"
     Goto loop

     
     ; Delete registry keys
     Call un.deleteRegistry
     
     SetAutoClose true
     
SectionEnd



Function un.onInit

     Call un.CheckUserRights
     
     !insertmacro MUI_INSTALLOPTIONS_EXTRACT "removeData.ini"
     MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure to remove $(^Name) and all its components?" IDYES +2
     Abort
     Call un.CheckMicrosoftApp
     Call un.CheckFunClientApp
     
FunctionEnd



Function un.onUninstSuccess
     HideWindow
     MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been removed from the computer."
FunctionEnd



; Display a custom page during uninstaller, to ask the user if deleting files/settings.
Function un.RemoveUserData

     !insertmacro MUI_HEADER_TEXT "Delete local settings" "Delete all local synchronization files and settings for all users."
     !insertmacro MUI_INSTALLOPTIONS_DISPLAY_RETURN "removeData.ini"
     
     Pop $R0      ; This is the return value: "success", "cancel", "back" or "error"
     ;MessageBox MB_ICONINFORMATION|MB_OK "ret value = $R0"
     StrCmp $R0 "success"  0 done
     
     ; Read the 'State' value of checkbox: 0 = not selected.
     ReadINIStr $R2 "$PLUGINSDIR\removeData.ini" "Field 1" "State"
     IntCmp $R2 0 done
     
     Call un.deleteUsersRegistry
     Call un.deleteUsersFiles
     
     ; Delete scheduled task for all users.
     ; Tasks have the name: "Funambol Outlook Plug-in - <Username>.job"
     ; where 'Username' is the Windows current user that created it.
     Delete "$WINDIR\Tasks\${PRODUCT_NAME}*.job"
     
  done:
FunctionEnd



; Utility to get the last part of a string using '\' as the separator
Function un.GetLastStrPart
     Exch $R0
     Push $R1
     Push $R2
     StrLen $R1 $R0
     IntOp $R1 $R1 + 1
     StrCpy $R3 1
  loop:
     IntOp $R1 $R1 - 1
     StrCpy $R2 $R0 1 -$R3
     IntOp $R3 $R3 + 1
     StrCmp $R2 "" exit2
     StrCmp $R2 "\" exit1
     Goto loop
  exit1:
     IntOp $R3 $R3 - 1
     StrCpy $R0 $R0 -$R3
  exit2:
     Pop $R2
     Pop $R1
     Exch $R0
FunctionEnd


; Utility to transform a string: replace all '/' into '\'
Function StrSlash
     Exch $R3 ; $R3 = needle ("\" or "/")
     Exch
     Exch $R1 ; $R1 = String to replacement in (haystack)
     Push $R2 ; Replaced haystack
     Push $R4 ; $R4 = not $R3 ("/" or "\")
     Push $R6
     Push $R7 ; Scratch reg
     StrCpy $R2 ""
     StrLen $R6 $R1
     StrCpy $R4 "\"
     StrCmp $R3 "/" loop
     StrCpy $R4 "/"
  loop:
     StrCpy $R7 $R1 1
     StrCpy $R1 $R1 $R6 1
     StrCmp $R7 $R3 found
     StrCpy $R2 "$R2$R7"
     StrCmp $R1 "" done loop
found:
     StrCpy $R2 "$R2$R4"
     StrCmp $R1 "" done loop
done:
     StrCpy $R3 $R2
     Pop $R7
     Pop $R6
     Pop $R4
     Pop $R2
     Pop $R1
     Exch $R3
FunctionEnd


;
; Write registry keys.
; All client keys for synchronization are automatically created the first time
; by plugin at startup.
;
Function writeRegistry
     ; Register as program that uses Microsoft Outlook
     WriteRegStr   HKLM     "${MSMAPIAPPS_REGKEY_CONTEXT}"       "${PRODUCT_NAME_EXE}"           "${MICROSOFT_OUTLOOK}"

     ; Installation path and info used by Addin.
     ;(others are generated by DLL registration, see addin.rgs)
     WriteRegStr  HKLM      "${ADDIN_REGKEY_CONTEXT}"            "${PROPERTY_PATH}"              "$INSTDIR"
     WriteRegStr  HKLM      "${ADDIN_REGKEY_CONTEXT}"            "${PROPERTY_ADDIN_NAME}"        "FunambolAddin.dll"
     
     ; funambol_swv = Funambol Software version: it's important for the upgrade process!
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_SWV}"               "${PRODUCT_VERSION}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_PATH}"              "$INSTDIR"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_DESCRIPTION}"       "${PRODUCT_NAME} v.${PRODUCT_VERSION}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_CUSTOMER}"          "${CUSTOMER}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_FUNAMBOL_SWV}"      "${FUNAMBOL_SWV}"
     
     ; Standard/portal build (SP passed as parameter by build.xml).
  !ifdef SP
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_SP}"                "1"
  !else
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_SP}"                "0"
  !endif

FunctionEnd


;
; Delete registry keys.
;
Function un.deleteRegistry

     ; Delete HKLM keys (values set on install)
     DeleteRegKey HKLM "${PLUGIN_REGKEY_CONTEXT}"

     ; Delete application registered from System
     DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
     DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

     ; Delete HKLM keys created during install procedure (NOT the ones created by regDLL!)
     DeleteRegKey HKLM  "${ADDIN_REGKEY_CONTEXT}\${PROPERTY_PATH}"
     DeleteRegKey HKLM  "${ADDIN_REGKEY_CONTEXT}\${PROPERTY_ADDIN_NAME}"

FunctionEnd


;
; Delete all users registry keys.
; Need to cycle on all users trees, as each user stores its keys under HKCU.
; Keys are deleted ONLY if user selected it from uninstaller RemoveUsersData page.
;
Function un.deleteUsersRegistry

     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0
  loop:
     EnumRegKey  $R1  HKU  ""  $R0
     StrCmp $R1 "" done                                  ; empty string when finished

     DeleteRegKey  HKU  "$R1\${PLUGIN_REGKEY_CONTEXT}"   ; Delete users keys (concatenate strings) - plugin user's settings.
     DeleteRegKey  HKU  "$R1\${ADDIN_REGKEY_CONTEXT}"    ; addin user's settings.

     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd


;
; Delete all users files from 'Application Data' folder.
; Need to cycle on all users doc&settings folders.
; Files are deleted ONLY if user selected it from uninstaller RemoveUsersData page.
;
Function un.deleteUsersFiles

     ; Get 'Application Data' paths from each user's registry keys.
     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0                                                        ; $R0 = counter for users on this machine.
  loop:
     EnumRegKey  $R1  HKU  ""  $R0                                  ; $R1 = name of this user.
     StrCmp $R1 "" done                                             ; (empty string when finished -> done)

     ReadRegStr $R2  HKU "$R1\${SHELLFOLDERS_CONTEXT}" "AppData"    ; $R2 = path of 'Application Data' folder for this user.
     StrCmp $R2 "" next                                             ; (not found -> next user)
     StrCpy $R3 "$R2\${DATAFILES_CONTEXT}"                          ; $R3 = path of local files.
     Delete "$R3\*.*"
     RMDir /r "$R3"
     
     ; Delete recursively empty folders under 'Application Data'.
     loop1:
         Push "$R3"
         Call un.GetLastStrPart
         Pop $R4                                                    ; $R4 = path of parent folder.
         StrCmp $R3 $R4  next
         StrCmp $R2 $R4  next                                       ; (reached Application data folder -> next user)

         StrCpy $R3 $R4
         RMDir "$R4"
         Goto loop1

  next:
     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd

