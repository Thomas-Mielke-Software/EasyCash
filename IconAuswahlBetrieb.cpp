// IconAuswahlBetrieb.cpp : implementation file
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
#include "easycash.h"
#include "IconAuswahlBetrieb.h"
#include "UnternehmensartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlBetrieb dialog


CIconAuswahlBetrieb::CIconAuswahlBetrieb(CEasyCashView* pParent /*=NULL*/)
	: CIconAuswahl(IDD_ICONAUSWAHL_BETRIEB, pParent)
{
	//{{AFX_DATA_INIT(CIconAuswahlBetrieb)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIconAuswahlBetrieb::DoDataExchange(CDataExchange* pDX)
{
	CIconAuswahl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIconAuswahlBetrieb)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIconAuswahlBetrieb, CIconAuswahl)
	//{{AFX_MSG_MAP(CIconAuswahlBetrieb)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CIconAuswahlBetrieb::InitDialog()
{
	if (m_nModus == 1) // Auswahl-Modus mit der Option zu ändern
	{
		SetDlgItemText(IDOK, "Sel. anzeigen");
		SetDlgItemText(IDCANCEL, "Alle anzeigen");
	}
	CIconAuswahl::InitDialog();
}

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlBetrieb message handlers



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

IconInfo* CIconAuswahlBetrieb::GetIcons()
{
	return Icons;
}

int CIconAuswahlBetrieb::GetIconCount()
{
	return sizeof(Icons)/sizeof(Icons[0]);
}

void CIconAuswahlBetrieb::Neu()
{
 
}

BOOL CIconAuswahlBetrieb::ChooseProperty(CString &csProperty)
{
	CUnternehmensartDlg dlg(this);
	int nPos;
	if ((nPos = csProperty.Find("\t")) < 0)
		dlg.m_Unternehmensart1 = csProperty;
	else	// Unternehmensart1, Unternehmensart2 (Rechtsform) und Steuernummer sind durch Tabs getrennt
	{
		int nPos2;
		dlg.m_Unternehmensart1 = csProperty.Left(nPos);
		if ((nPos2 = csProperty.Find("\t", nPos+1)) < 0)
			dlg.m_Unternehmensart2 = csProperty.Mid(nPos+1);
		else
		{
			dlg.m_Unternehmensart2 = csProperty.Mid(nPos+1, nPos2 - nPos - 1);
			dlg.m_Steuernummer = csProperty.Mid(nPos2+1);
		}
	}

	if (dlg.DoModal() == IDOK)
	{
		csProperty = dlg.m_Unternehmensart1 + "\t" + dlg.m_Unternehmensart2 + "\t" + dlg.m_Steuernummer;
		return TRUE;
	}
	else
		return FALSE;
}

void CIconAuswahlBetrieb::Umbenennen(int nElement)
{
	
}

void CIconAuswahlBetrieb::Loeschen(int nElement)
{
	
}

int CIconAuswahlBetrieb::IconAuswahl()
{
	// Icon wählen
	CIconAuswahlBetrieb dlgIcon(m_pParent);
	dlgIcon.DoModal();		

	return dlgIcon.m_nSelected;
}