# Microsoft Developer Studio Project File - Name="EasyCash" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=EasyCash - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EasyCash.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EasyCash.mak" CFG="EasyCash - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EasyCash - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "EasyCash - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EasyCash - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fr /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Ox /Ot /Og /Os
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ECTIFace\Release\ECTIFace.lib ..\GrafLib\cimage\Release\cimage.lib ..\GrafLib\jpeg\Release\jpeg.lib ..\GrafLib\png\Release\png.lib ..\GrafLib\zlib\Release\zlib.lib rpcrt4.lib Wininet.lib /nologo /stack:0xa00000 /subsystem:windows /machine:I386 /out:".\Release\EasyCT.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "EasyCash - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ECTIFace\Debug\ECTIFace.lib ../GrafLib/png/Debug/png.lib ../GrafLib/jpeg/Debug/jpeg.lib ../GrafLib/zlib/Debug/zlib.lib ../GrafLib/cimage/Debug/cimage.lib rpcrt4.lib Wininet.lib /nologo /stack:0xa00000 /subsystem:windows /debug /machine:I386 /out:".\Debug\EasyCT.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "EasyCash - Win32 Release"
# Name "EasyCash - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\AnfangssaldoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BuchenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\DatenverzeichnisDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DauBuchAusfuehren.cpp
# End Source File
# Begin Source File

SOURCE=.\DauerbuchungenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DruckauswahlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DSAMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\EasyCash.cpp
# End Source File
# Begin Source File

SOURCE=.\EasyCash.rc
# End Source File
# Begin Source File

SOURCE=.\EasyCashView.cpp
# End Source File
# Begin Source File

SOURCE=.\Einstellungen1.cpp
# End Source File
# Begin Source File

SOURCE=.\Einstellungen2.cpp
# End Source File
# Begin Source File

SOURCE=.\Einstellungen3.cpp
# End Source File
# Begin Source File

SOURCE=.\Einstellungen4.cpp
# End Source File
# Begin Source File

SOURCE=.\einstellungen5.cpp
# End Source File
# Begin Source File

SOURCE=.\Formularfeld.cpp
# End Source File
# Begin Source File

SOURCE=.\getprogramversion.cpp
# End Source File
# Begin Source File

SOURCE=.\IconAuswahl.cpp
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlBestandskonto.cpp
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlBetrieb.cpp
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlMandant.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MandantName.cpp
# End Source File
# Begin Source File

SOURCE=.\nbMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\NeuesFormular.cpp
# End Source File
# Begin Source File

SOURCE=.\NeuesKonto.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginManager.cpp
# End Source File
# Begin Source File

SOURCE=.\QuickEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\QuickList.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\RechnDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RegDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistrierenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Separator.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Tip.cpp
# End Source File
# Begin Source File

SOURCE=.\TipDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# Begin Source File

SOURCE=.\ToolBarEx.cpp
# End Source File
# Begin Source File

SOURCE=.\UnternehmensartDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\UstVorauszahlungenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VCalcParser.cpp
# End Source File
# Begin Source File

SOURCE=.\VCalcParserException.cpp
# End Source File
# Begin Source File

SOURCE=.\VisualStylesXP.cpp
# End Source File
# Begin Source File

SOURCE=.\VoranmeldezeitraumDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VoranmeldungszeitraumDroplist.cpp
# End Source File
# Begin Source File

SOURCE=.\WebUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\WinVer.cpp
# End Source File
# Begin Source File

SOURCE=.\XFileOpenListView.cpp
# End Source File
# Begin Source File

SOURCE=.\XFolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\XFolderDialog.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\XHistoryCombo.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\AnfangssaldoDlg.h
# End Source File
# Begin Source File

SOURCE=.\buchendlg.h
# End Source File
# Begin Source File

SOURCE=.\childfrm.h
# End Source File
# Begin Source File

SOURCE=.\DatenverzeichnisDlg.h
# End Source File
# Begin Source File

SOURCE=.\daubuchausfuehren.h
# End Source File
# Begin Source File

SOURCE=.\dauerbuchungendlg.h
# End Source File
# Begin Source File

SOURCE=.\druckauswahldlg.h
# End Source File
# Begin Source File

SOURCE=.\DSAMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\easycash.h
# End Source File
# Begin Source File

SOURCE=.\easycashview.h
# End Source File
# Begin Source File

SOURCE=.\einstellungen1.h
# End Source File
# Begin Source File

SOURCE=.\einstellungen2.h
# End Source File
# Begin Source File

SOURCE=.\einstellungen3.h
# End Source File
# Begin Source File

SOURCE=.\einstellungen4.h
# End Source File
# Begin Source File

SOURCE=.\Einstellungen5.h
# End Source File
# Begin Source File

SOURCE=.\Formularfeld.h
# End Source File
# Begin Source File

SOURCE=.\getprogramversion.h
# End Source File
# Begin Source File

SOURCE=.\IconAuswahl.h
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlBestandskonto.h
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlBetrieb.h
# End Source File
# Begin Source File

SOURCE=.\IconAuswahlMandant.h
# End Source File
# Begin Source File

SOURCE=.\mainfrm.h
# End Source File
# Begin Source File

SOURCE=.\MandantName.h
# End Source File
# Begin Source File

SOURCE=.\nbMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\NeuesFormular.h
# End Source File
# Begin Source File

SOURCE=.\neueskonto.h
# End Source File
# Begin Source File

SOURCE=.\PluginManager.h
# End Source File
# Begin Source File

SOURCE=.\QuickEdit.h
# End Source File
# Begin Source File

SOURCE=.\QuickList.h
# End Source File
# Begin Source File

SOURCE=.\RechnDlg.h
# End Source File
# Begin Source File

SOURCE=.\regdlg.h
# End Source File
# Begin Source File

SOURCE=.\registrierendlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Schemadef.h
# End Source File
# Begin Source File

SOURCE=.\Separator.h
# End Source File
# Begin Source File

SOURCE=.\sha1.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Tip.h
# End Source File
# Begin Source File

SOURCE=.\TipDlg.h
# End Source File
# Begin Source File

SOURCE=.\Tmschema.h
# End Source File
# Begin Source File

SOURCE=.\UnternehmensartDlg.h
# End Source File
# Begin Source File

SOURCE=.\UstVorauszahlungenDlg.h
# End Source File
# Begin Source File

SOURCE=.\Uxtheme.h
# End Source File
# Begin Source File

SOURCE=.\VCalcParser.h
# End Source File
# Begin Source File

SOURCE=.\VCalcParserException.h
# End Source File
# Begin Source File

SOURCE=.\VCalcParserTypes.h
# End Source File
# Begin Source File

SOURCE=.\VisualStylesXP.h
# End Source File
# Begin Source File

SOURCE=.\voranmeldezeitraumdlg.h
# End Source File
# Begin Source File

SOURCE=.\VoranmeldungszeitraumDroplist.h
# End Source File
# Begin Source File

SOURCE=.\WebUpdate.h
# End Source File
# Begin Source File

SOURCE=.\XFileOpenListView.h
# End Source File
# Begin Source File

SOURCE=.\XFolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\XHistoryCombo.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\01.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\02.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\03.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\04.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\05.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\06.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\07.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\08.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\09.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\31ster.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\bmp31ste.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\button1.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\button2.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\button3.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\button4.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\button5.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\button6.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\clock.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\datenverzeichnis.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\easycash.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\easycash.ico
# End Source File
# Begin Source File

SOURCE=.\Res\easycash.rc2
# End Source File
# Begin Source File

SOURCE=.\Res\easycashdoc.ico
# End Source File
# Begin Source File

SOURCE=.\RES\FACE01.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\FACE02.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\FACE03.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\find.bmp
# End Source File
# Begin Source File

SOURCE=.\gray.ico
# End Source File
# Begin Source File

SOURCE=.\green.ico
# End Source File
# Begin Source File

SOURCE=.\RES\icon01.ico
# End Source File
# Begin Source File

SOURCE=.\RES\icons.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\icons_bestandskonten.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\imglist16.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\imglist32.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\infosign.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\litebulb.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\pie.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\plugin.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\plugin_platzhalter.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\Rechner.bmp
# End Source File
# Begin Source File

SOURCE=.\red.ico
# End Source File
# Begin Source File

SOURCE=.\Res\spanner.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\spike.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\spike1.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\spike2.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\stop.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\toolbar_hicolor.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\toolbar_hicolor_disabled.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\toolbar_hicolor_hot.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\tipps.txt
# End Source File
# End Target
# End Project
# Section EasyCash : {72ADFD54-2C39-11D0-9903-00A0C91BC942}
# 	1:17:CG_IDS_DIDYOUKNOW:106
# 	1:22:CG_IDS_TIPOFTHEDAYMENU:105
# 	1:18:CG_IDS_TIPOFTHEDAY:104
# 	1:22:CG_IDS_TIPOFTHEDAYHELP:109
# 	1:19:CG_IDP_FILE_CORRUPT:108
# 	1:7:IDD_TIP:103
# 	1:13:IDB_LIGHTBULB:102
# 	1:18:CG_IDS_FILE_ABSENT:107
# 	2:17:CG_IDS_DIDYOUKNOW:CG_IDS_DIDYOUKNOW
# 	2:7:CTipDlg:CTipDlg
# 	2:22:CG_IDS_TIPOFTHEDAYMENU:CG_IDS_TIPOFTHEDAYMENU
# 	2:18:CG_IDS_TIPOFTHEDAY:CG_IDS_TIPOFTHEDAY
# 	2:12:CTIP_Written:OK
# 	2:22:CG_IDS_TIPOFTHEDAYHELP:CG_IDS_TIPOFTHEDAYHELP
# 	2:2:BH:
# 	2:19:CG_IDP_FILE_CORRUPT:CG_IDP_FILE_CORRUPT
# 	2:7:IDD_TIP:IDD_TIP
# 	2:8:TipDlg.h:TipDlg.h
# 	2:13:IDB_LIGHTBULB:IDB_LIGHTBULB
# 	2:18:CG_IDS_FILE_ABSENT:CG_IDS_FILE_ABSENT
# 	2:10:TipDlg.cpp:TipDlg.cpp
# End Section
