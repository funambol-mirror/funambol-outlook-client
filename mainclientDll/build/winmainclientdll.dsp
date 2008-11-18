# Microsoft Developer Studio Project File - Name="winmainclientdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=winmainclientdll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "winmainclientdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "winmainclientdll.mak" CFG="winmainclientdll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winmainclientdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "winmainclientdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winmainclientdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINMAINCLIENTDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../src/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINMAINCLIENTDLL_EXPORTS" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x410 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib scapiwin32.lib wininet.lib /nologo /dll /incremental:yes /machine:I386 /nodefaultlib:"LIBCD" /libpath:"../../lib/win32lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "winmainclientdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINMAINCLIENTDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../src/include" /I "..\..\..\..\..\client-api\native\src\include\win32" /I "..\..\..\..\..\client-api\native\src\include\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINMAINCLIENTDLL_EXPORTS" /D "_UNICODE" /D "UNICODE" /D "DEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wininet.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "winmainclientdll - Win32 Release"
# Name "winmainclientdll - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\cpp\AppointmentBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\ContactBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\Container.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\OutlookConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\OutlookScheduler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\TaskBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\WindowsSyncSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\winmaincpp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cpp\winmaincpp.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\include\outlook\AppointmentBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\ContactBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\Container.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\OutlookConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\ptypes\ptypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\TaskBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\WindowsSyncSource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\include\outlook\winmaincpp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "ptypes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\lib\ptypes\patomic.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\pfatal.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\pmem.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\pobjlist.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\pstring.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\pstrmanip.cxx
# End Source File
# Begin Source File

SOURCE=..\..\lib\ptypes\punknown.cxx
# End Source File
# End Group
# End Target
# End Project
