// ECTUpdater.cpp: Definiert das Klassenverhalten für die Anwendung.
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
#include "ECTUpdater.h"
#include "ECTUpdaterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CECTUpdaterApp

BEGIN_MESSAGE_MAP(CECTUpdaterApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CECTUpdaterApp-Erstellung

CECTUpdaterApp::CECTUpdaterApp()
{
	// TODO: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}


// Das einzige CECTUpdaterApp-Objekt

CECTUpdaterApp theApp;


// CECTUpdaterApp-Initialisierung

BOOL CECTUpdaterApp::InitInstance()
{
	CWinAppEx::InitInstance();

	// Standardinitialisierung
	SetRegistryKey(_T("EasyCash&Tax-Updater"));

	if (!strlen(m_lpCmdLine))
	{
		AfxMessageBox(_T("Der Updater benötigt einen Parameter beim Aufruf! Update nicht möglich."));
		return FALSE;
	}

	CECTUpdaterDlg dlg(m_lpCmdLine);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	// Da das Dialogfeld geschlossen wurde, FALSE zurückliefern, sodass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}
