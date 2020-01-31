//	DSAMessageBox.cpp: implementation file
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
#include "nbMessageBox.h"
#include "DSAMessageBox.h"

// 'member' variables
static HINSTANCE m_hInst = NULL;	// module name to get the ressource strings from
static char m_hkcu[200]  = "";		// registry key to remember history for each ressource string ID

// --- Call these both to initialize DSAMessageBox

// set module to get ressource strings from (e.g. by using GetModuleHandle("MYAPP.EXE") )
void DSASetModule(HINSTANCE hInst)	
{
	m_hInst = hInst;
}

// set registry key to store checkbox history (e.g. "SOFTWARE\\MyCompany\\MyProduct")
void DSASetRegKey(char *s)			
{
	ASSERT(strlen(s) < sizeof(m_hkcu));
	strcpy(m_hkcu, s);
}

//--- Main function

int DSAMessageBox(int nStringId, UINT nType)
{
	int rc;					// return value
	HKEY hkcu;				// registry key handle
	char data[2];			// store data from registry here
	CString csValueName;	// value name in registry
	csValueName.Format("DSA%05d", nStringId);

	// look in user registry
	if (RegCreateKey(HKEY_CURRENT_USER, m_hkcu, &hkcu) == ERROR_SUCCESS)
	{
		long lDummy;
		long lType = REG_SZ;
		long lCb;
		lDummy = 0L;

		// read settings for current user
		*data = '\0';
		lCb = sizeof(data);
		if (RegQueryValueEx(hkcu, csValueName.GetBuffer(0), 
								   NULL, (ULONG *)&lType, 
								   (LPBYTE)data, (ULONG *)&lCb) != ERROR_SUCCESS)
		{
			*data = '0'; data[1] = '\0';
		}
	}
	else
	{
		*data = '0'; data[1] = '\0';
		hkcu = NULL;
	}

	// show message box
	if (*data != '1')
	{
		char buf[10000];
		CnbMessageBox box(AfxGetMainWnd());
		LoadString(m_hInst, nStringId, buf, sizeof(buf));
		rc = box.MessageBox(buf, nType);

		if (hkcu)
		{
			if (box.GetChecked())	// 'Don't show again' checked?
			{
				long lRetCode;

				*data = '1'; data[1] = '\0';
				lRetCode = RegSetValueEx(hkcu,	csValueName.GetBuffer(0), // remember in user registry
										(ULONG)0, (ULONG)REG_SZ, 
										(LPBYTE)data, (ULONG)strlen(data));
			}
		}
	}
	else
	{
		switch (nType & 0xf)
		{
		case MB_YESNO: 
		case MB_YESNOCANCEL:
			rc = IDYES; break;
		case MB_ABORTRETRYIGNORE: 
			rc = IDIGNORE; break;
		case MB_RETRYCANCEL:
			rc = IDCANCEL; break;
		default:
		case MB_OK: 
		case MB_OKCANCEL:
			rc = IDOK; break;
		}
	}

	if (hkcu) RegCloseKey(hkcu);

	return rc;
}
