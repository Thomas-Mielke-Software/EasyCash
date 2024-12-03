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
	CString strRest = "";
	if ((nPos = csProperty.Find("\t")) < 0)
		dlg.m_Unternehmensart1 = csProperty;
	else	// Unternehmensart1, Unternehmensart2 (Rechtsform), Steuernummer und Wirtschaftsidentifikationsnummer sind durch Tabs getrennt
	{
		int nTokenPos = 0;
		CString strToken;
		strToken = csProperty.Tokenize(_T("\t"), nTokenPos);
		for (int i = 0; !strToken.IsEmpty(); strToken = csProperty.Tokenize(_T("\t"), nTokenPos), i++)
		{
			switch (i)
			{
			case 0: dlg.m_Unternehmensart1 = strToken; break;
			case 1: dlg.m_Unternehmensart2 = strToken; break;
			case 2: dlg.m_Steuernummer = strToken;	   break;
			case 3: dlg.m_wirtschaftsIdNr = strToken;  break;
			default: strRest += _T("\t") + strToken;   break;  // zukünftige betriebsabhängige Einstellungedaten beibehalten
			}			
		}
	}

	if (dlg.DoModal() == IDOK)
	{
		csProperty = dlg.m_Unternehmensart1 + "\t" + dlg.m_Unternehmensart2 + "\t" + dlg.m_Steuernummer + "\t" + dlg.m_wirtschaftsIdNr + strRest;
		if (!dlg.m_wirtschaftsIdNr.IsEmpty())
		{
			if (dlg.m_wirtschaftsIdNr.GetLength() != 17)
				AfxMessageBox("Hinweis: Die Wirtschaftsidentifikationsnummer muss 17 Zeichen lang sein.");
			else if (dlg.m_wirtschaftsIdNr.Left(2) != _T("DE"))
				AfxMessageBox("Hinweis: Die Wirtschaftsidentifikationsnummer muss mit 'DE' beginnen.");
			else if (dlg.m_wirtschaftsIdNr[11] != _T('-'))
				AfxMessageBox("Hinweis: Die Wirtschaftsidentifikationsnummer muss an der 12. Position einen Bindestrich enthalten.");
			else 
				for (int i = 2; i < 17; i++)
				{
					if (i == 11) i++;  // '-' überspringen
					if (!isdigit(dlg.m_wirtschaftsIdNr[i]))
					{
						CString msg;
						msg.Format("Hinweis: Die Wirtschaftsidentifikationsnummer muss an der %d. Position eine Ziffer enthalten.", i + 1);
						AfxMessageBox(msg);
						break;
					}
				}
		}
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