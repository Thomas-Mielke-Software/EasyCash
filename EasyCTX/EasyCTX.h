#if !defined(AFX_EASYCTX_H__B7733DD4_465C_426E_98AF_C24BE87AE80A__INCLUDED_)
#define AFX_EASYCTX_H__B7733DD4_465C_426E_98AF_C24BE87AE80A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// EasyCTX.h : main header file for EASYCTX.DLL
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

#include "..\ECTIFace\EasyCashDoc.h"

#include "comcat.h"
#include "objsafe.h"

/////////////////////////////////////////////////////////////////////////////
// CEasyCTXApp : See EasyCTX.cpp for implementation.

class CEasyCTXApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

void DrawAndCenterBitmap(CDC* pDC, int ResourceBitmapID, RECT *r);

HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYCTX_H__B7733DD4_465C_426E_98AF_C24BE87AE80A__INCLUDED)

