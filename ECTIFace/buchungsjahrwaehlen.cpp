// BuchungsjahrWaehlen.cpp : implementation file
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
#include "resource.h"
#include "BuchungsjahrWaehlen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuchungsjahrWaehlen dialog


CBuchungsjahrWaehlen::CBuchungsjahrWaehlen(CWnd* pParent /*=NULL*/)
	: CDialog(CBuchungsjahrWaehlen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBuchungsjahrWaehlen)
	m_jahr = 0;
	m_waehrung = _T("");
	//}}AFX_DATA_INIT
	m_bBuchungsdateienVorhanden = FALSE;
}


void CBuchungsjahrWaehlen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuchungsjahrWaehlen)
	DDX_Text(pDX, IDC_JAHR, m_jahr);
	DDV_MinMaxInt(pDX, m_jahr, 1900, 2035);
	DDX_Text(pDX, IDC_WAEHRUNG, m_waehrung);
	DDV_MaxChars(pDX, m_waehrung, 3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuchungsjahrWaehlen, CDialog)
	//{{AFX_MSG_MAP(CBuchungsjahrWaehlen)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_LBN_DBLCLK(IDC_LISTE_BUCHUNGSDATEIEN, &CBuchungsjahrWaehlen::OnDblclkListeBuchungsdateien)
	ON_BN_CLICKED(IDRETRY, &CBuchungsjahrWaehlen::OnOeffnen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuchungsjahrWaehlen message handlers

BOOL CBuchungsjahrWaehlen::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListBox* pListe = (CListBox*)GetDlgItem(IDC_LISTE_BUCHUNGSDATEIEN);

	// Bestehende Jahres-Buchungsdateien (*.eca) im Datenverzeichnis auflisten
	m_bBuchungsdateienVorhanden = FALSE;
	if (!m_csDatenverzeichnis.IsEmpty())
	{
		CFileFind ff;
		BOOL bGoOn = ff.FindFile(m_csDatenverzeichnis + "\\*.eca");
		while (bGoOn)
		{
			bGoOn = ff.FindNextFile();
			if (!ff.IsDirectory())
			{
				pListe->AddString(ff.GetFileName());
				m_bBuchungsdateienVorhanden = TRUE;
			}
		}
	}

	if (m_bBuchungsdateienVorhanden)
	{
		// juengste Datei (durch LBS_SORT zuletzt) vorauswaehlen
		pListe->SetCurSel(pListe->GetCount() - 1);
	}
	else
	{
		pListe->InsertString(0, "keine bestehenden Jahres-Buchungsdateien im Datenverzeichnis gefunden,");
		pListe->InsertString(1, "bitte Option 2 wählen oder ggf. ein Datenverzeichnis mit JahrXXXX.eca-Dateien auswählen");
		GetDlgItem(IDRETRY)->EnableWindow(FALSE);   // "Oeffnen" deaktivieren
	}

	return TRUE;  // TRUE = Standardfokus beibehalten
}

void CBuchungsjahrWaehlen::OnDblclkListeBuchungsdateien()
{
	UebernehmeAuswahlUndOeffne();
}

void CBuchungsjahrWaehlen::OnOeffnen()
{
	UebernehmeAuswahlUndOeffne();
}

void CBuchungsjahrWaehlen::UebernehmeAuswahlUndOeffne()
{
	if (!m_bBuchungsdateienVorhanden)
		return;

	CListBox* pListe = (CListBox*)GetDlgItem(IDC_LISTE_BUCHUNGSDATEIEN);
	int nSel = pListe->GetCurSel();
	if (nSel == LB_ERR)
		return;   // nichts ausgewaehlt

	CString csName;
	pListe->GetText(nSel, csName);
	m_csAusgewaehlteDatei = m_csDatenverzeichnis + "\\" + csName;
	EndDialog(IDRETRY);
}
