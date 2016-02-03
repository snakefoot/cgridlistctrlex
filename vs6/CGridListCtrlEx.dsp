# Microsoft Developer Studio Project File - Name="CGridListCtrlEx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CGridListCtrlEx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CGridListCtrlEx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CGridListCtrlEx.mak" CFG="CGridListCtrlEx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CGridListCtrlEx - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CGridListCtrlEx - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CGridListCtrlEx - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG"
# ADD RSC /l 0x406 /d "NDEBUG" /d "_VC60"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CGridListCtrlEx - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG"
# ADD RSC /l 0x406 /d "_DEBUG" /d "_VC60"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CGridListCtrlEx - Win32 Release"
# Name "CGridListCtrlEx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\DemoApplication\CGridListCtrlEx.rc
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\CGridListCtrlExApp.cpp
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\CGridListCtrlExDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\DemoApplication\CGridListCtrlExApp.h
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\CGridListCtrlExDlg.h
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\CListCtrl_DataModel.h
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\resource.h
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\DemoApplication\res\CGridListCtrlEx.exe.manifest
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\res\CGridListCtrlEx.ico
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\res\CGridListCtrlEx.rc2
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\res\FLGDEN.ICO
# End Source File
# Begin Source File

SOURCE=..\DemoApplication\res\FLGSWED.ICO
# End Source File
# End Group
# Begin Group "CGridListCtrlEx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTrait.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitCombo.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitCombo.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitDateTime.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitDateTime.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitEdit.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitHyperLink.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitHyperLink.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitImage.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitImage.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitMultilineEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitMultilineEdit.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitText.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitText.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridColumnTraitVisitor.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridListCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridListCtrlEx.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridListCtrlGroups.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridListCtrlGroups.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTrait.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTraitText.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTraitText.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTraitVisitor.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTraitXP.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\CGridRowTraitXP.h
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\ViewConfigSection.cpp
# End Source File
# Begin Source File

SOURCE=..\CGridListCtrlEx\ViewConfigSection.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
