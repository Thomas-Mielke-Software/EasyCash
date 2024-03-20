// EasyCash.h : main header file for the EASYCASH application
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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "DSAMessageBox.h"	// Don't Show again Messagebox-Unterstützung

/////////////////////////////////////////////////////////////////////////////
// CEasyCashApp:
// See EasyCash.cpp for the implementation of this class
//

class CEasyCashApp : public CWinAppEx
{
public:
	CStringArray m_MandantenverzeichnisMRUDateinamen;
	BOOL OnOpenRecentFile(UINT nID);
	BOOL Check4EasyCTX();
	CEasyCashApp();

// Overrides

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyCashApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL	

// Implementation

	void RegistrierungsinformationenSichern(char *DateinameParam);

	//{{AFX_MSG(CEasyCashApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void ShowTipAtStartup(void);
	void ShowTipOfTheDay(void);
	void CaptionBox(LPCTSTR str);
	void CaptionBox(LPCTSTR str, int id, LPCTSTR buttontext, LPCTSTR tooltiptext);
	void CaptionBoxHide();
	BOOL IsCaptionBoxShown();
	CString m_csStartupPlugin;
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);
	CDocument* GenericGetActiveDocument(CRuntimeClass* pClass);

	BOOL m_bCrashReportingAktiv;
};


/////////////////////////////////////////////////////////////////////////////

extern CEasyCashApp theApp;

extern BOOL SelectFolder(LPSTR sFolder, LPCTSTR sTitle);
extern HICON HICONFromCBitmap(CBitmap& bitmap);

#define GLOBAL_BANK "DAB"
#define GLOBAL_BLZ "70120400"
#define GLOBAL_KONTO "7406319017"
#define GLOBAL_BIC "DABBDEMMXXX"
#define GLOBAL_IBAN "DE54701204007406319017"