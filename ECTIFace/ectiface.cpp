// ECTIFace.cpp : Defines the initialization routines for the DLL.
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

#include "stdafx.h"
#include <afxdllx.h>
#include "ECTIFace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE ECTIFaceDLL = { NULL, NULL };

HINSTANCE hInst;
char LocaleDezimalkomma = ',';	// für Dezimalpunkt und Tausendertrenner aus Betriebssystem geholt
char LocaleTausendertrenner = '.';
char LocaleZifferngruppierung = '3';
char LocaleMinuszeichen = '-';

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("ECTIFACE.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		AfxInitExtensionModule(ECTIFaceDLL, hInstance);

		hInst = hInstance;
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, &LocaleDezimalkomma, 1);
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONTHOUSANDSEP, &LocaleTausendertrenner, 1);
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONGROUPING  , &LocaleZifferngruppierung, 1);
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, &LocaleMinuszeichen, 1);
		
		// Insert this DLL into the resource chain
		new CDynLinkLibrary(ECTIFaceDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("ECTIFACE.DLL Terminating!\n");
	}
	return 1;   // ok
}
