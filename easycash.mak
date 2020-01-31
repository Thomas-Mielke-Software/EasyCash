# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=ECTIFace - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to ECTIFace - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "EasyCash - Win32 Release" && "$(CFG)" !=\
 "EasyCash - Win32 Debug" && "$(CFG)" != "ECTIFace - Win32 Release" && "$(CFG)"\
 != "ECTIFace - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "easycash.mak" CFG="ECTIFace - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EasyCash - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "EasyCash - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ECTIFace - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ECTIFace - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "EasyCash - Win32 Release"

!IF  "$(CFG)" == "EasyCash - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "ECTIFace - Win32 Release" "$(OUTDIR)\EasyCT.exe"\
 "$(OUTDIR)\easycash.bsc"

CLEAN : 
	-@erase "$(INTDIR)\BuchenDlg.obj"
	-@erase "$(INTDIR)\BuchenDlg.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\DauBuchAusfuehren.obj"
	-@erase "$(INTDIR)\DauBuchAusfuehren.sbr"
	-@erase "$(INTDIR)\DauerbuchungenDlg.obj"
	-@erase "$(INTDIR)\DauerbuchungenDlg.sbr"
	-@erase "$(INTDIR)\DruckauswahlDlg.obj"
	-@erase "$(INTDIR)\DruckauswahlDlg.sbr"
	-@erase "$(INTDIR)\EasyCash.obj"
	-@erase "$(INTDIR)\easycash.pch"
	-@erase "$(INTDIR)\EasyCash.res"
	-@erase "$(INTDIR)\EasyCash.sbr"
	-@erase "$(INTDIR)\EasyCashView.obj"
	-@erase "$(INTDIR)\EasyCashView.sbr"
	-@erase "$(INTDIR)\Einstellungen1.obj"
	-@erase "$(INTDIR)\Einstellungen1.sbr"
	-@erase "$(INTDIR)\Einstellungen2.obj"
	-@erase "$(INTDIR)\Einstellungen2.sbr"
	-@erase "$(INTDIR)\Einstellungen3.obj"
	-@erase "$(INTDIR)\Einstellungen3.sbr"
	-@erase "$(INTDIR)\Einstellungen4.obj"
	-@erase "$(INTDIR)\Einstellungen4.sbr"
	-@erase "$(INTDIR)\getprogramversion.obj"
	-@erase "$(INTDIR)\getprogramversion.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\NeuesKonto.obj"
	-@erase "$(INTDIR)\NeuesKonto.sbr"
	-@erase "$(INTDIR)\RegDlg.obj"
	-@erase "$(INTDIR)\RegDlg.sbr"
	-@erase "$(INTDIR)\RegistrierenDlg.obj"
	-@erase "$(INTDIR)\RegistrierenDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\VoranmeldezeitraumDlg.obj"
	-@erase "$(INTDIR)\VoranmeldezeitraumDlg.sbr"
	-@erase "$(OUTDIR)\easycash.bsc"
	-@erase "$(OUTDIR)\EasyCT.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
# SUBTRACT CPP /Ox /Ot /Og /Os
CPP_PROJ=/nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL"\
 /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/easycash.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/EasyCash.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/easycash.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BuchenDlg.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\DauBuchAusfuehren.sbr" \
	"$(INTDIR)\DauerbuchungenDlg.sbr" \
	"$(INTDIR)\DruckauswahlDlg.sbr" \
	"$(INTDIR)\EasyCash.sbr" \
	"$(INTDIR)\EasyCashView.sbr" \
	"$(INTDIR)\Einstellungen1.sbr" \
	"$(INTDIR)\Einstellungen2.sbr" \
	"$(INTDIR)\Einstellungen3.sbr" \
	"$(INTDIR)\Einstellungen4.sbr" \
	"$(INTDIR)\getprogramversion.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\NeuesKonto.sbr" \
	"$(INTDIR)\RegDlg.sbr" \
	"$(INTDIR)\RegistrierenDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\VoranmeldezeitraumDlg.sbr"

"$(OUTDIR)\easycash.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ECTIFace\Release\ECTIFace.lib /nologo /subsystem:windows /machine:I386 /out:"Release\EasyCT.exe"
LINK32_FLAGS=ECTIFace\Release\ECTIFace.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/EasyCT.pdb" /machine:I386\
 /out:"$(OUTDIR)/EasyCT.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BuchenDlg.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DauBuchAusfuehren.obj" \
	"$(INTDIR)\DauerbuchungenDlg.obj" \
	"$(INTDIR)\DruckauswahlDlg.obj" \
	"$(INTDIR)\EasyCash.obj" \
	"$(INTDIR)\EasyCash.res" \
	"$(INTDIR)\EasyCashView.obj" \
	"$(INTDIR)\Einstellungen1.obj" \
	"$(INTDIR)\Einstellungen2.obj" \
	"$(INTDIR)\Einstellungen3.obj" \
	"$(INTDIR)\Einstellungen4.obj" \
	"$(INTDIR)\getprogramversion.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NeuesKonto.obj" \
	"$(INTDIR)\RegDlg.obj" \
	"$(INTDIR)\RegistrierenDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\VoranmeldezeitraumDlg.obj" \
	".\ECTIFace\Release\ECTIFace.lib"

"$(OUTDIR)\EasyCT.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "ECTIFace - Win32 Debug" "$(OUTDIR)\EasyCT.exe" "$(OUTDIR)\easycash.bsc"

CLEAN : 
	-@erase "$(INTDIR)\BuchenDlg.obj"
	-@erase "$(INTDIR)\BuchenDlg.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\DauBuchAusfuehren.obj"
	-@erase "$(INTDIR)\DauBuchAusfuehren.sbr"
	-@erase "$(INTDIR)\DauerbuchungenDlg.obj"
	-@erase "$(INTDIR)\DauerbuchungenDlg.sbr"
	-@erase "$(INTDIR)\DruckauswahlDlg.obj"
	-@erase "$(INTDIR)\DruckauswahlDlg.sbr"
	-@erase "$(INTDIR)\EasyCash.obj"
	-@erase "$(INTDIR)\easycash.pch"
	-@erase "$(INTDIR)\EasyCash.res"
	-@erase "$(INTDIR)\EasyCash.sbr"
	-@erase "$(INTDIR)\EasyCashView.obj"
	-@erase "$(INTDIR)\EasyCashView.sbr"
	-@erase "$(INTDIR)\Einstellungen1.obj"
	-@erase "$(INTDIR)\Einstellungen1.sbr"
	-@erase "$(INTDIR)\Einstellungen2.obj"
	-@erase "$(INTDIR)\Einstellungen2.sbr"
	-@erase "$(INTDIR)\Einstellungen3.obj"
	-@erase "$(INTDIR)\Einstellungen3.sbr"
	-@erase "$(INTDIR)\Einstellungen4.obj"
	-@erase "$(INTDIR)\Einstellungen4.sbr"
	-@erase "$(INTDIR)\getprogramversion.obj"
	-@erase "$(INTDIR)\getprogramversion.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\NeuesKonto.obj"
	-@erase "$(INTDIR)\NeuesKonto.sbr"
	-@erase "$(INTDIR)\RegDlg.obj"
	-@erase "$(INTDIR)\RegDlg.sbr"
	-@erase "$(INTDIR)\RegistrierenDlg.obj"
	-@erase "$(INTDIR)\RegistrierenDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\VoranmeldezeitraumDlg.obj"
	-@erase "$(INTDIR)\VoranmeldezeitraumDlg.sbr"
	-@erase "$(OUTDIR)\easycash.bsc"
	-@erase "$(OUTDIR)\EasyCT.exe"
	-@erase "$(OUTDIR)\EasyCT.ilk"
	-@erase "$(OUTDIR)\EasyCT.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/easycash.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/EasyCash.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/easycash.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BuchenDlg.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\DauBuchAusfuehren.sbr" \
	"$(INTDIR)\DauerbuchungenDlg.sbr" \
	"$(INTDIR)\DruckauswahlDlg.sbr" \
	"$(INTDIR)\EasyCash.sbr" \
	"$(INTDIR)\EasyCashView.sbr" \
	"$(INTDIR)\Einstellungen1.sbr" \
	"$(INTDIR)\Einstellungen2.sbr" \
	"$(INTDIR)\Einstellungen3.sbr" \
	"$(INTDIR)\Einstellungen4.sbr" \
	"$(INTDIR)\getprogramversion.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\NeuesKonto.sbr" \
	"$(INTDIR)\RegDlg.sbr" \
	"$(INTDIR)\RegistrierenDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\VoranmeldezeitraumDlg.sbr"

"$(OUTDIR)\easycash.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ECTIFace\Debug\ECTIFace.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug\EasyCT.exe"
LINK32_FLAGS=ECTIFace\Debug\ECTIFace.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/EasyCT.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/EasyCT.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BuchenDlg.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DauBuchAusfuehren.obj" \
	"$(INTDIR)\DauerbuchungenDlg.obj" \
	"$(INTDIR)\DruckauswahlDlg.obj" \
	"$(INTDIR)\EasyCash.obj" \
	"$(INTDIR)\EasyCash.res" \
	"$(INTDIR)\EasyCashView.obj" \
	"$(INTDIR)\Einstellungen1.obj" \
	"$(INTDIR)\Einstellungen2.obj" \
	"$(INTDIR)\Einstellungen3.obj" \
	"$(INTDIR)\Einstellungen4.obj" \
	"$(INTDIR)\getprogramversion.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NeuesKonto.obj" \
	"$(INTDIR)\RegDlg.obj" \
	"$(INTDIR)\RegistrierenDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\VoranmeldezeitraumDlg.obj" \
	".\ECTIFace\Debug\ECTIFace.lib"

"$(OUTDIR)\EasyCT.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ECTIFace\Release"
# PROP BASE Intermediate_Dir "ECTIFace\Release"
# PROP BASE Target_Dir "ECTIFace"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ECTIFace\Release"
# PROP Intermediate_Dir "ECTIFace\Release"
# PROP Target_Dir "ECTIFace"
OUTDIR=.\ECTIFace\Release
INTDIR=.\ECTIFace\Release

ALL : ".\Release\ECTIFace.dll"

CLEAN : 
	-@erase "$(INTDIR)\buchungsjahrwaehlen.obj"
	-@erase "$(INTDIR)\EasyCashDoc.obj"
	-@erase "$(INTDIR)\ECTIFace.obj"
	-@erase "$(INTDIR)\ECTIFace.pch"
	-@erase "$(INTDIR)\ECTIFace.res"
	-@erase "$(INTDIR)\ECTIFaceMisc.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\ECTIFace.exp"
	-@erase "$(OUTDIR)\ECTIFace.lib"
	-@erase ".\Release\ECTIFace.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Fp"$(INTDIR)/ECTIFace.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ECTIFace\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/ECTIFace.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ECTIFace.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /out:"Release/ECTIFace.dll"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ECTIFace.pdb" /machine:I386 /def:".\ECTIFace\ECTIFace.def"\
 /out:"Release/ECTIFace.dll" /implib:"$(OUTDIR)/ECTIFace.lib" 
DEF_FILE= \
	".\ECTIFace\ECTIFace.def"
LINK32_OBJS= \
	"$(INTDIR)\buchungsjahrwaehlen.obj" \
	"$(INTDIR)\EasyCashDoc.obj" \
	"$(INTDIR)\ECTIFace.obj" \
	"$(INTDIR)\ECTIFace.res" \
	"$(INTDIR)\ECTIFaceMisc.obj" \
	"$(INTDIR)\StdAfx.obj"

".\Release\ECTIFace.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ECTIFace\Debug"
# PROP BASE Intermediate_Dir "ECTIFace\Debug"
# PROP BASE Target_Dir "ECTIFace"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ECTIFace\Debug"
# PROP Intermediate_Dir "ECTIFace\Debug"
# PROP Target_Dir "ECTIFace"
OUTDIR=.\ECTIFace\Debug
INTDIR=.\ECTIFace\Debug

ALL : ".\Debug\ECTIFace.dll"

CLEAN : 
	-@erase "$(INTDIR)\buchungsjahrwaehlen.obj"
	-@erase "$(INTDIR)\EasyCashDoc.obj"
	-@erase "$(INTDIR)\ECTIFace.obj"
	-@erase "$(INTDIR)\ECTIFace.pch"
	-@erase "$(INTDIR)\ECTIFace.res"
	-@erase "$(INTDIR)\ECTIFaceMisc.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\ECTIFace.exp"
	-@erase "$(OUTDIR)\ECTIFace.lib"
	-@erase ".\Debug\ECTIFace.dll"
	-@erase ".\Debug\ECTIFace.ilk"
	-@erase ".\Debug\ECTIFace.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Fp"$(INTDIR)/ECTIFace.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ECTIFace\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x407 /fo"$(INTDIR)/ECTIFace.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ECTIFace.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:"Debug/ECTIFace.pdb" /debug /machine:I386 /out:"Debug/ECTIFace.dll"
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"Debug/ECTIFace.pdb" /debug /machine:I386 /def:".\ECTIFace\ECTIFace.def"\
 /out:"Debug/ECTIFace.dll" /implib:"$(OUTDIR)/ECTIFace.lib" 
DEF_FILE= \
	".\ECTIFace\ECTIFace.def"
LINK32_OBJS= \
	"$(INTDIR)\buchungsjahrwaehlen.obj" \
	"$(INTDIR)\EasyCashDoc.obj" \
	"$(INTDIR)\ECTIFace.obj" \
	"$(INTDIR)\ECTIFace.res" \
	"$(INTDIR)\ECTIFaceMisc.obj" \
	"$(INTDIR)\StdAfx.obj"

".\Debug\ECTIFace.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "EasyCash - Win32 Release"
# Name "EasyCash - Win32 Debug"

!IF  "$(CFG)" == "EasyCash - Win32 Release"

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "EasyCash - Win32 Release"

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EasyCash.cpp
DEP_CPP_EASYC=\
	".\buchendlg.h"\
	".\childfrm.h"\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	".\getprogramversion.h"\
	".\mainfrm.h"\
	".\regdlg.h"\
	".\registrierendlg.h"\
	

"$(INTDIR)\EasyCash.obj" : $(SOURCE) $(DEP_CPP_EASYC) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\EasyCash.sbr" : $(SOURCE) $(DEP_CPP_EASYC) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\ECTESaldo\..\StdAfx.h"\
	

!IF  "$(CFG)" == "EasyCash - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL"\
 /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/easycash.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\easycash.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/easycash.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\easycash.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\mainfrm.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChildFrm.cpp
DEP_CPP_CHILD=\
	".\childfrm.h"\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\EasyCashView.cpp
DEP_CPP_EASYCA=\
	".\buchendlg.h"\
	".\childfrm.h"\
	".\daubuchausfuehren.h"\
	".\dauerbuchungendlg.h"\
	".\druckauswahldlg.h"\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	".\voranmeldezeitraumdlg.h"\
	

"$(INTDIR)\EasyCashView.obj" : $(SOURCE) $(DEP_CPP_EASYCA) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\EasyCashView.sbr" : $(SOURCE) $(DEP_CPP_EASYCA) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\EasyCash.rc
DEP_RSC_EASYCAS=\
	".\Res\31ster.bmp"\
	".\Res\bmp00001.bmp"\
	".\Res\bmp31ste.bmp"\
	".\Res\button1.bmp"\
	".\Res\button2.bmp"\
	".\Res\button3.bmp"\
	".\Res\button4.bmp"\
	".\Res\button5.bmp"\
	".\Res\clock.bmp"\
	".\Res\easycash.ico"\
	".\Res\easycash.rc2"\
	".\Res\easycashdoc.ico"\
	".\Res\infosign.bmp"\
	".\Res\spanner.bmp"\
	".\Res\spike.bmp"\
	".\Res\spike1.bmp"\
	".\Res\spike2.bmp"\
	".\Res\stop.bmp"\
	".\Res\toolbar.bmp"\
	

"$(INTDIR)\EasyCash.res" : $(SOURCE) $(DEP_RSC_EASYCAS) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\BuchenDlg.cpp
DEP_CPP_BUCHE=\
	".\buchendlg.h"\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	

"$(INTDIR)\BuchenDlg.obj" : $(SOURCE) $(DEP_CPP_BUCHE) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\BuchenDlg.sbr" : $(SOURCE) $(DEP_CPP_BUCHE) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DauerbuchungenDlg.cpp
DEP_CPP_DAUER=\
	".\dauerbuchungendlg.h"\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	

"$(INTDIR)\DauerbuchungenDlg.obj" : $(SOURCE) $(DEP_CPP_DAUER) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\DauerbuchungenDlg.sbr" : $(SOURCE) $(DEP_CPP_DAUER) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DauBuchAusfuehren.cpp
DEP_CPP_DAUBU=\
	".\buchendlg.h"\
	".\daubuchausfuehren.h"\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	

"$(INTDIR)\DauBuchAusfuehren.obj" : $(SOURCE) $(DEP_CPP_DAUBU) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\DauBuchAusfuehren.sbr" : $(SOURCE) $(DEP_CPP_DAUBU) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\RegistrierenDlg.cpp
DEP_CPP_REGIS=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\registrierendlg.h"\
	

"$(INTDIR)\RegistrierenDlg.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\RegistrierenDlg.sbr" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\RegDlg.cpp
DEP_CPP_REGDL=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\regdlg.h"\
	

"$(INTDIR)\RegDlg.obj" : $(SOURCE) $(DEP_CPP_REGDL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\RegDlg.sbr" : $(SOURCE) $(DEP_CPP_REGDL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DruckauswahlDlg.cpp
DEP_CPP_DRUCK=\
	".\druckauswahldlg.h"\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	

"$(INTDIR)\DruckauswahlDlg.obj" : $(SOURCE) $(DEP_CPP_DRUCK) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\DruckauswahlDlg.sbr" : $(SOURCE) $(DEP_CPP_DRUCK) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Einstellungen1.cpp
DEP_CPP_EINST=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\einstellungen1.h"\
	

"$(INTDIR)\Einstellungen1.obj" : $(SOURCE) $(DEP_CPP_EINST) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\Einstellungen1.sbr" : $(SOURCE) $(DEP_CPP_EINST) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Einstellungen2.cpp
DEP_CPP_EINSTE=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\einstellungen2.h"\
	

"$(INTDIR)\Einstellungen2.obj" : $(SOURCE) $(DEP_CPP_EINSTE) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\Einstellungen2.sbr" : $(SOURCE) $(DEP_CPP_EINSTE) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Einstellungen3.cpp
DEP_CPP_EINSTEL=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\einstellungen3.h"\
	

"$(INTDIR)\Einstellungen3.obj" : $(SOURCE) $(DEP_CPP_EINSTEL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\Einstellungen3.sbr" : $(SOURCE) $(DEP_CPP_EINSTEL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\VoranmeldezeitraumDlg.cpp
DEP_CPP_VORAN=\
	".\easycash.h"\
	".\easycashview.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\einstellungen1.h"\
	".\einstellungen2.h"\
	".\einstellungen3.h"\
	".\einstellungen4.h"\
	".\voranmeldezeitraumdlg.h"\
	

"$(INTDIR)\VoranmeldezeitraumDlg.obj" : $(SOURCE) $(DEP_CPP_VORAN) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\VoranmeldezeitraumDlg.sbr" : $(SOURCE) $(DEP_CPP_VORAN) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ToDo.txt

!IF  "$(CFG)" == "EasyCash - Win32 Release"

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\getprogramversion.cpp
DEP_CPP_GETPR=\
	".\ECTESaldo\..\StdAfx.h"\
	

"$(INTDIR)\getprogramversion.obj" : $(SOURCE) $(DEP_CPP_GETPR) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\getprogramversion.sbr" : $(SOURCE) $(DEP_CPP_GETPR) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Einstellungen4.cpp
DEP_CPP_EINSTELL=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\einstellungen1.h"\
	".\einstellungen4.h"\
	".\neueskonto.h"\
	

"$(INTDIR)\Einstellungen4.obj" : $(SOURCE) $(DEP_CPP_EINSTELL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\Einstellungen4.sbr" : $(SOURCE) $(DEP_CPP_EINSTELL) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\NeuesKonto.cpp
DEP_CPP_NEUES=\
	".\easycash.h"\
	".\ECTESaldo\..\StdAfx.h"\
	".\neueskonto.h"\
	

"$(INTDIR)\NeuesKonto.obj" : $(SOURCE) $(DEP_CPP_NEUES) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"

"$(INTDIR)\NeuesKonto.sbr" : $(SOURCE) $(DEP_CPP_NEUES) "$(INTDIR)"\
 "$(INTDIR)\easycash.pch"


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ECTIFace"

!IF  "$(CFG)" == "EasyCash - Win32 Release"

"ECTIFace - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\easycash.mak" CFG="ECTIFace - Win32 Release" 

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

"ECTIFace - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\easycash.mak" CFG="ECTIFace - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "ECTIFace - Win32 Release"
# Name "ECTIFace - Win32 Debug"

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ECTIFace\ReadMe.txt

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\ECTIFace.cpp

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_CPP_ECTIF=\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\ECTIFace.obj" : $(SOURCE) $(DEP_CPP_ECTIF) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_CPP_ECTIF=\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\ECTIFace.obj" : $(SOURCE) $(DEP_CPP_ECTIF) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\ECTIFace.def

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\StdAfx.cpp

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_CPP_STDAF=\
	".\ECTIFace\stdafx.h"\
	
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Fp"$(INTDIR)/ECTIFace.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ECTIFace.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_CPP_STDAF=\
	".\ECTIFace\stdafx.h"\
	
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Fp"$(INTDIR)/ECTIFace.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ECTIFace.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\ECTIFace.rc

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_RSC_ECTIFA=\
	".\ECTIFace\res\ectiface.rc2"\
	

"$(INTDIR)\ECTIFace.res" : $(SOURCE) $(DEP_RSC_ECTIFA) "$(INTDIR)"
   $(RSC) /l 0x407 /fo"$(INTDIR)/ECTIFace.res" /i "ECTIFace" /d "NDEBUG" /d\
 "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_RSC_ECTIFA=\
	".\ECTIFace\res\ectiface.rc2"\
	

"$(INTDIR)\ECTIFace.res" : $(SOURCE) $(DEP_RSC_ECTIFA) "$(INTDIR)"
   $(RSC) /l 0x407 /fo"$(INTDIR)/ECTIFace.res" /i "ECTIFace" /d "_DEBUG" /d\
 "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\buchungsjahrwaehlen.cpp

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_CPP_BUCHU=\
	".\ECTIFace\buchungsjahrwaehlen.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\buchungsjahrwaehlen.obj" : $(SOURCE) $(DEP_CPP_BUCHU) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_CPP_BUCHU=\
	".\ECTIFace\buchungsjahrwaehlen.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\buchungsjahrwaehlen.obj" : $(SOURCE) $(DEP_CPP_BUCHU) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\EasyCashDoc.cpp

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_CPP_EASYCASH=\
	".\ECTIFace\buchungsjahrwaehlen.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\EasyCashDoc.obj" : $(SOURCE) $(DEP_CPP_EASYCASH) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_CPP_EASYCASH=\
	".\ECTIFace\buchungsjahrwaehlen.h"\
	".\ECTIFace\EasyCashDoc.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\EasyCashDoc.obj" : $(SOURCE) $(DEP_CPP_EASYCASH) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ECTIFace\ECTIFaceMisc.cpp

!IF  "$(CFG)" == "ECTIFace - Win32 Release"

DEP_CPP_ECTIFAC=\
	".\ECTIFace\EasyCashDoc.h"\
	".\ECTIFace\ectiface.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\ECTIFaceMisc.obj" : $(SOURCE) $(DEP_CPP_ECTIFAC) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ECTIFace - Win32 Debug"

DEP_CPP_ECTIFAC=\
	".\ECTIFace\EasyCashDoc.h"\
	".\ECTIFace\ectiface.h"\
	".\ECTIFace\stdafx.h"\
	

"$(INTDIR)\ECTIFaceMisc.obj" : $(SOURCE) $(DEP_CPP_ECTIFAC) "$(INTDIR)"\
 "$(INTDIR)\ECTIFace.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
