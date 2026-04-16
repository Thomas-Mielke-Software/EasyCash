// stdafx.h — Precompiled header für ECTBridge
//
// WICHTIG: Dieser Header wird von sowohl nativen als auch /clr-Dateien
// included. Er darf keine managed Typen enthalten, da er sonst in
// reinen nativen TU's nicht verwendbar wäre.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

// Windows / MFC
#include <afxwin.h>         // MFC-Kern und -Standardkomponenten
#include <afxext.h>         // MFC-Erweiterungen
#include <afxdisp.h>        // MFC-Automatisierungsklassen
#include <afxdtctl.h>       // MFC IE-Klassen
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC Common Controls
#endif

// Standard-Library
#include <string>
#include <vector>
