// DruckauswahlDlg.cpp : implementation file
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
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "DruckauswahlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DruckauswahlDlg dialog


DruckauswahlDlg::DruckauswahlDlg(CEasyCashView* pParent /*=NULL*/)
	: CDialog(DruckauswahlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(DruckauswahlDlg)
	m_bm = 0;
	m_bt = 0;
	m_vm = 0;
	m_vt = 0;
	m_auswahl = -1;
	//}}AFX_DATA_INIT

	m_pParent = pParent;
}


void DruckauswahlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DruckauswahlDlg)
	DDX_Text(pDX, IDC_DATUM_BIS_MONAT, m_bm);
	DDV_MinMaxInt(pDX, m_bm, 1, 12);
	DDX_Text(pDX, IDC_DATUM_BIS_TAG, m_bt);
	DDV_MinMaxInt(pDX, m_bt, 1, 31);
	DDX_Text(pDX, IDC_DATUM_VON_MONAT, m_vm);
	DDV_MinMaxInt(pDX, m_vm, 1, 12);
	DDX_Text(pDX, IDC_DATUM_VON_TAG, m_vt);
	DDV_MinMaxInt(pDX, m_vt, 1, 31);
	DDX_Radio(pDX, IDC_BUCHUNGSLISTE, m_auswahl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DruckauswahlDlg, CDialog)
	//{{AFX_MSG_MAP(DruckauswahlDlg)
	ON_BN_CLICKED(IDC_BUCHUNGSLISTE, OnBuchungsliste)
	ON_BN_CLICKED(IDC_BUCHUNGSLISTE_KONTEN, OnBuchungslisteKonten)
	ON_BN_CLICKED(IDC_BUCHUNGSLISTE_BESTANDSKONTEN, OnBuchungslisteBestandskonten)
	ON_BN_CLICKED(IDC_UMST_ERKLAERUNG, OnUmstErklaerung)
	ON_BN_CLICKED(IDC_EURECHNUNG, OnEurechnung)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DruckauswahlDlg message handlers

void DruckauswahlDlg::InitKontenfilter(int n)
{
	int i;

	((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->ResetContent();

	((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->AddString("<alle Konten>");
	if (n == 0) // normale Konten
	{
		for (i = 0; i < 100; i++)
		{
			if (!m_pParent->einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->AddString(m_pParent->einstellungen1->EinnahmenRechnungsposten[i]);
		}
		for (i = 0; i < 100; i++)
		{
			if (!m_pParent->einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->AddString(m_pParent->einstellungen1->AusgabenRechnungsposten[i]);
		}
	}
	else // if (n == 1) Bestandskonten
	{
		for (i = 0; i < m_pParent->m_csaBestandskontenNamen.GetSize(); i++)
			((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->AddString(m_pParent->m_csaBestandskontenNamen[i]);
	}
	((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->SetCurSel(0);
}

BOOL DruckauswahlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitKontenfilter(0);
	
	if (m_pParent->m_csaBetriebeNamen.GetSize())
	{
		int i;

		((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->AddString("<alle Betriebe>");
		for (i = 0; i < m_pParent->m_csaBetriebeNamen.GetSize(); i++)
			((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->AddString(m_pParent->m_csaBetriebeNamen[i]);
		((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->SetCurSel(0);	
	}
	else
	{
		GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow(FALSE);
	}

	// ((CButton *)GetDlgItem(IDC_BUCHUNGSLISTE))->SetCheck(TRUE);
	
	return TRUE;  
}

void DruckauswahlDlg::OnOK() 
{
	UpdateData();

	{
		//Datum checken
		CTime von(m_nJahr, m_vm, m_vt, 0, 0, 0);
		CTime bis(m_nJahr, m_bm, m_bt, 0, 0, 0);
		if (m_vm != von.GetMonth() || m_vt != von.GetDay()) 
		{
			AfxMessageBox("Das Anfangsdatum ist unrichtig.");
			GetDlgItem(IDC_DATUM_VON_TAG)->SetFocus();
			return;
		}
		if (m_bm != bis.GetMonth() || m_bt != bis.GetDay()) 
		{
			AfxMessageBox("Das Enddatum ist unrichtig.");
			GetDlgItem(IDC_DATUM_BIS_TAG)->SetFocus();
			return;
		}
		if (von > bis)
		{
			char buffer[1000];
			sprintf(buffer, "Das Enddatum liegt vor dem Anfangsdatum. Soll für das Enddatum das Jahr %04d angenommen werden?", m_nJahr+1);
			if (AfxMessageBox(buffer, MB_YESNO) != IDYES)
			{
				GetDlgItem(IDC_DATUM_BIS_TAG)->SetFocus();
				return;
			}
		}
	}

	int n = ((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->GetCurSel();
	if (n > 0 && n < ((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->GetCount()) 
		((CComboBox *)GetDlgItem(IDC_KONTENFILTER))->GetLBText(n, m_pParent->m_KontenFilterPrinter);
	else
		m_pParent->m_KontenFilterPrinter = "<alle Konten>";

	n = ((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->GetCurSel();
	if (n >= 0 && n < ((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->GetCount()) 
		((CComboBox *)GetDlgItem(IDC_BETRIEBFILTER))->GetLBText(n, m_pParent->m_BetriebFilterPrinter);
	else
		m_pParent->m_BetriebFilterPrinter = "<alle Betriebe>";

	if (((CButton *)GetDlgItem(IDC_BUCHUNGSLISTE))->GetCheck())
	{
		CDialog::EndDialog(m_pParent->m_KontenFilterPrinter == "<alle Konten>" ? IDC_BUCHUNGSLISTE : IDC_BUCHUNGSLISTE_KONTEN);
		return;
	}

	if (((CButton *)GetDlgItem(IDC_BUCHUNGSLISTE_KONTEN))->GetCheck())
	{
		CDialog::EndDialog(IDC_BUCHUNGSLISTE_KONTEN);
		return;
	}

	if (((CButton *)GetDlgItem(IDC_BUCHUNGSLISTE_BESTANDSKONTEN))->GetCheck())
	{
		CDialog::EndDialog(IDC_BUCHUNGSLISTE_BESTANDSKONTEN);
		return;
	}

	if (((CButton *)GetDlgItem(IDC_ANLAGENVERZEICHNIS))->GetCheck())
	{
		CDialog::EndDialog(IDC_ANLAGENVERZEICHNIS);
		return;
	}

	if (((CButton *)GetDlgItem(IDC_UMST_ERKLAERUNG))->GetCheck())
	{
		CDialog::EndDialog(IDC_UMST_ERKLAERUNG);
		return;
	}

	if (((CButton *)GetDlgItem(IDC_EURECHNUNG))->GetCheck())
	{
		CDialog::EndDialog(IDC_EURECHNUNG);
		return;
	}

	AfxMessageBox("Bitte eine Druckoption auswählen.");
}

void DruckauswahlDlg::OnCancel() 
{
	UpdateData();

	CDialog::EndDialog(0);
}

void DruckauswahlDlg::OnBuchungsliste() 
{
	InitKontenfilter(0);
	GetDlgItem(IDC_DATUM_VON_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow();
	GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow();
}

void DruckauswahlDlg::OnBuchungslisteKonten() 
{
	InitKontenfilter(0);
	GetDlgItem(IDC_DATUM_VON_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow();
	GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow();
}

void DruckauswahlDlg::OnBuchungslisteBestandskonten() 
{
	InitKontenfilter(1);
	GetDlgItem(IDC_DATUM_VON_TAG)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATUM_BIS_TAG)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow(FALSE);
}

void DruckauswahlDlg::OnUmstErklaerung() 
{
	InitKontenfilter(0);
	GetDlgItem(IDC_DATUM_VON_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow();
	GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow(FALSE);
}

void DruckauswahlDlg::OnEurechnung() 
{
	InitKontenfilter(0);
	GetDlgItem(IDC_DATUM_VON_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_TAG)->EnableWindow();
	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow();
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow();
	GetDlgItem(IDC_BETRIEBFILTER)->EnableWindow();
}