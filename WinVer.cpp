// WinVer.cpp
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
#include "WinVer.h"


BOOL Is2000OrGreater()
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osinfo);

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;

	return ((dwPlatformId == 2) && 
			(dwMajorVersion >= 5));
}

BOOL Is2000()
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osinfo);

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	DWORD dwMinorVersion = osinfo.dwMinorVersion;

	return ((dwPlatformId == 2) && 
			(dwMajorVersion == 5) && 
			(dwMinorVersion == 0));
}

BOOL IsXP()
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osinfo);

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	DWORD dwMinorVersion = osinfo.dwMinorVersion;

	return ((dwPlatformId == 2) && 
			(dwMajorVersion == 5) && 
			(dwMinorVersion >= 1));
}

BOOL IsVista()
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osinfo);

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;

	return ((dwPlatformId == 2) && 
			(dwMajorVersion >= 6));
}
