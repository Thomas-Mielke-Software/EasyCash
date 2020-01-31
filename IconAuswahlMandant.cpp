// IconAuswahlMandant.cpp : implementation file
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
#include "XFolderDialog.h"
#include "easycash.h"
#include "IconAuswahlMandant.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlMandant dialog


CIconAuswahlMandant::CIconAuswahlMandant(CEasyCashView* pParent /*=NULL*/)
	: CIconAuswahl(IDD_ICONAUSWAHL_MANDANT, pParent)
{
	//{{AFX_DATA_INIT(CIconAuswahlMandant)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIconAuswahlMandant::DoDataExchange(CDataExchange* pDX)
{
	CIconAuswahl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIconAuswahlMandant)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIconAuswahlMandant, CIconAuswahl)
	//{{AFX_MSG_MAP(CIconAuswahlMandant)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlMandant message handlers



/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlMandant virtuals

static IconInfo Icons[] = {
	"Standard",
	"Einkaufstüte", 
	"Kiste",
	"Glas",
	"Zettel",
	"Füller",
	"Bleistift",
	"Tresor",
	"Schreibmaschine",
	"Notizblock",
	"CD",
	"Festplatte",
	"Maus",
	"Computer",
	"Notebook",
	"Platine",
	"Wassermann",
	"Schaf",
	"Krebs", 
	"Schütze", 
	"Skorpion",
	"Drache",
	"Möbel",
	"Tor",
	"Obst",
	"Brot",
	"Käse",
	"Kreuz",
	"Geschenk",
	"Kranz",
	"Kosmetik",
	"Schuh", 
	"Handy",
	"Uhr",
	"Kaktus",
	"Kerze",
	"Wecker",
	"Zauberei",
	"Bügeleisen",
	"Video",
	"Medikament",
	"Ente",
	"Spielzeug",
	"Mikrofon",
	"Keyboard",
	"Trommel",
	"Lautsprecher",
	"Hund",
	"Schädel",
	"Vogel",
	"Fisch",
	"Internet",
	"Ausrufungszeichen",
	"Hütchen",
	"Leuchtturm",
	"Pinsel",
	"Ziegel",
	"Teppichmesser",
	"Ölkanne",
	"Träger",
	"Zange",
	"Globus",
	"Qualle",
	"Schwamm",
	"Mond",
	"Rakete",
	"Trepanation",
	"Bär",
	"Papierkrieg",
	"Solar"
};

IconInfo* CIconAuswahlMandant::GetIcons()
{
	return Icons;
}

int CIconAuswahlMandant::GetIconCount()
{
	return sizeof(Icons)/sizeof(Icons[0]);
}

void CIconAuswahlMandant::Neu()
{
	
}

BOOL CIconAuswahlMandant::ChooseProperty(CString &csProperty)
{
	if (csProperty.IsEmpty())
		if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), csProperty.GetBuffer(MAX_PATH), CSIDL_PERSONAL, FALSE))
			csProperty = "C:\\";
	CString csTitle = (CString)GetProperty() + " auswählen";	

	// Mandantenverzeichnis wählen
	if (SelectFolder(csProperty.GetBuffer(MAX_PATH), (LPCTSTR)csTitle)) 
	{
		csProperty.ReleaseBuffer();
		if (csProperty.GetLength() && csProperty[csProperty.GetLength()-1] == '\\')
			csProperty = csProperty.Left(csProperty.GetLength()-1);
		return TRUE;
	}
	else
		return FALSE;
}

void CIconAuswahlMandant::Umbenennen(int nElement)
{
	
}

void CIconAuswahlMandant::Loeschen(int nElement)
{
	
}

int CIconAuswahlMandant::IconAuswahl()
{
	// Icon wählen
	CIconAuswahlMandant dlgIcon(m_pParent);
	dlgIcon.DoModal();		

	return dlgIcon.m_nSelected;
}