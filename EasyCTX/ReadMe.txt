========================================================================
		ActiveX Control DLL : EASYCTX
========================================================================

ControlWizard has created this project for your EASYCTX ActiveX Control
DLL, which contains 3 controls.

This skeleton project not only demonstrates the basics of writing an
ActiveX Control, but is also a starting point for writing the specific
features of your control.

This file contains a summary of what you will find in each of the files
that make up your EASYCTX ActiveX Control DLL.

EasyCTX.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.

EasyCTX.h
	This is the main include file for the ActiveX Control DLL.  It
	includes other project-specific includes such as resource.h.

EasyCTX.cpp
	This is the main source file that contains code for DLL initialization,
	termination and other bookkeeping.

EasyCTX.rc
	This is a listing of the Microsoft Windows resources that the project
	uses.  This file can be directly edited with the Visual C++ resource
	editor.

EasyCTX.def
	This file contains information about the ActiveX Control DLL that
	must be provided to run with Microsoft Windows.

EasyCTX.clw
	This file contains information used by ClassWizard to edit existing
	classes or add new classes.  ClassWizard also uses this file to store
	information needed to generate and edit message maps and dialog data
	maps and to generate prototype member functions.

EasyCTX.odl
	This file contains the Object Description Language source code for the
	type library of your controls.

/////////////////////////////////////////////////////////////////////////////
Dokument control:

DokumentCtl.h
	This file contains the declaration of the CDokumentCtrl C++ class.

DokumentCtl.cpp
	This file contains the implementation of the CDokumentCtrl C++ class.

DokumentPpg.h
	This file contains the declaration of the CDokumentPropPage C++ class.

DokumentPpg.cpp
	This file contains the implementation of the CDokumentPropPage C++ class.

DokumentCtl.bmp
	This file contains a bitmap that a container will use to represent the
	CDokumentCtrl control when it appears on a tool palette.  This bitmap
	is included by the main resource file EasyCTX.rc.

/////////////////////////////////////////////////////////////////////////////
Einstellung control:

EinstellungCtl.h
	This file contains the declaration of the CEinstellungCtrl C++ class.

EinstellungCtl.cpp
	This file contains the implementation of the CEinstellungCtrl C++ class.

EinstellungPpg.h
	This file contains the declaration of the CEinstellungPropPage C++ class.

EinstellungPpg.cpp
	This file contains the implementation of the CEinstellungPropPage C++ class.

EinstellungCtl.bmp
	This file contains a bitmap that a container will use to represent the
	CEinstellungCtrl control when it appears on a tool palette.  This bitmap
	is included by the main resource file EasyCTX.rc.

/////////////////////////////////////////////////////////////////////////////
Buchung control:

BuchungCtl.h
	This file contains the declaration of the CBuchungCtrl C++ class.

BuchungCtl.cpp
	This file contains the implementation of the CBuchungCtrl C++ class.

BuchungPpg.h
	This file contains the declaration of the CBuchungPropPage C++ class.

BuchungPpg.cpp
	This file contains the implementation of the CBuchungPropPage C++ class.

BuchungCtl.bmp
	This file contains a bitmap that a container will use to represent the
	CBuchungCtrl control when it appears on a tool palette.  This bitmap
	is included by the main resource file EasyCTX.rc.

/////////////////////////////////////////////////////////////////////////////
Help Support:

EasyCTX.hpj
	This file is the Help Project file used by the Help compiler to create
	your ActiveX Control's Help file.

*.bmp
	These are bitmap files required by the standard Help file topics for
	Microsoft Foundation Class Library standard commands.  These files are
	located in the HLP subdirectory.

EasyCTX.rtf
	This file contains the standard help topics for the common properties,
	events and methods supported by many ActiveX Controls.  You can edit
	this to add or remove additional control specific topics.  This file is
	located in the HLP subdirectory.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

stdafx.h, stdafx.cpp
	These files are used to build a precompiled header (PCH) file
	named stdafx.pch and a precompiled types (PCT) file named stdafx.obj.

resource.h
	This is the standard header file, which defines new resource IDs.
	The Visual C++ resource editor reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

ControlWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
