// Einstellungen1.cpp : implementation file
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
#include "Einstellungen1.h"
#include "ECTIFace\AfAGenauigkeit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen1 property page

IMPLEMENT_DYNCREATE(CEinstellungen1, CPropertyPage)

CEinstellungen1::CEinstellungen1() : CPropertyPage(CEinstellungen1::IDD)
{
	//{{AFX_DATA_INIT(CEinstellungen1)
	m_monatliche_voranmeldung = -1;
	m_taeglich_buchen = FALSE;
	m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen = FALSE;
	m_bErzeugeLaufendeBuchungsnummernFuerAusgaben = FALSE;
	m_lfd_Buchungsnummer_Ausgaben = 0;
	m_lfd_Buchungsnummer_Einnahmen = 0;
	m_BuchungsdatumBelassen = FALSE;
	m_buchungsjahr = 0;
	m_lfd_Buchungsnummer_Bank = 0;
	m_lfd_Buchungsnummer_Kasse = 0;
	m_sondervorauszahlung = _T("");
	m_bErzeugeLaufendeBuchungsnummernFuerBank = FALSE;
	m_bErzeugeLaufendeBuchungsnummernFuerKasse = FALSE;
	//}}AFX_DATA_INIT
#if (_MSC_VER >= 1600)
	EnableDynamicLayout(TRUE);
#endif
}

CEinstellungen1::~CEinstellungen1()
{
}

void CEinstellungen1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEinstellungen1)
	DDX_Radio(pDX, IDC_MONAT, m_monatliche_voranmeldung);
	DDX_Check(pDX, IDC_TAEGLICH_BUCHEN, m_taeglich_buchen);
	DDX_Check(pDX, IDC_LAUFENDE_BUCHUNGSNUMMERN_FUER_EINNAHMEN, m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen);
	DDX_Check(pDX, IDC_LAUFENDE_BUCHUNGSNUMMERN_FUER_AUSGABEN, m_bErzeugeLaufendeBuchungsnummernFuerAusgaben);
	DDX_Text(pDX, IDC_LFD_BUCHUNGSNUMMER_AUSGABEN, m_lfd_Buchungsnummer_Ausgaben);
	DDX_Text(pDX, IDC_LFD_BUCHUNGSNUMMER_EINNAHMEN, m_lfd_Buchungsnummer_Einnahmen);
	DDX_Check(pDX, IDC_BUCHUNGSDATUM_BELASSEN, m_BuchungsdatumBelassen);
	DDX_Text(pDX, IDC_BUCHUNGSJAHR, m_buchungsjahr);
	DDV_MinMaxInt(pDX, m_buchungsjahr, 1980, 3000);
	DDX_Text(pDX, IDC_LFD_BUCHUNGSNUMMER_BANK, m_lfd_Buchungsnummer_Bank);
	DDX_Text(pDX, IDC_LFD_BUCHUNGSNUMMER_KASSE, m_lfd_Buchungsnummer_Kasse);
	DDX_Text(pDX, IDC_SONDERVORAUSZAHLUNG, m_sondervorauszahlung);
	DDX_Check(pDX, IDC_LAUFENDE_BUCHUNGSNUMMERN_FUER_BANK, m_bErzeugeLaufendeBuchungsnummernFuerBank);
	DDX_Check(pDX, IDC_LAUFENDE_BUCHUNGSNUMMERN_FUER_KASSE, m_bErzeugeLaufendeBuchungsnummernFuerKasse);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEinstellungen1, CPropertyPage)
	//{{AFX_MSG_MAP(CEinstellungen1)
	ON_LBN_SELCHANGE(IDC_LISTE, OnSelchangeListe)
	ON_EN_KILLFOCUS(IDC_EA, OnKillfocusEa)
	ON_EN_KILLFOCUS(IDC_BUCHUNGSPOSTENTEXT, OnKillfocusBuchungspostentext)
	ON_EN_KILLFOCUS(IDC_MWST, OnKillfocusMwst)
	ON_EN_KILLFOCUS(IDC_AFA, OnKillfocusAfa)
	ON_CBN_SELCHANGE(IDC_EURECHNUNGSPOSTEN, OnSelchangeEurechnungsposten)
	ON_BN_CLICKED(IDC_BILDSCHIRMSCHRIFT, OnBildschirmschrift)
	ON_BN_CLICKED(IDC_DRUCKERSCHRIFT, OnDruckerschrift)
	ON_BN_CLICKED(IDC_AFA_GENAUIGKEIT, OnAfaGenauigkeit)
	ON_EN_KILLFOCUS(IDC_SONDERVORAUSZAHLUNG, OnKillfocusSondervorauszahlung)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen1 message handlers

BOOL CEinstellungen1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	SetDlgItemText(IDC_BUCHUNGSPOSTENNUMMER, "");
	SetDlgItemText(IDC_EA, "");
	SetDlgItemText(IDC_MWST, "");
	SetDlgItemText(IDC_AFA, "");
	SetDlgItemText(IDC_BUCHUNGSPOSTENTEXT, "");
	
	// Liste löschen und neu aufbauen
	((CListBox *)GetDlgItem(IDC_LISTE))->ResetContent();
	int TabStopArray[4] = { 15, 30, 52, 72 };
	((CListBox *)GetDlgItem(IDC_LISTE))->SetTabStops(4, TabStopArray);
	int i;
	for (i = 0; i < 100; i++)
	{
		char buffer[1000];
		if (Buchungsposten[i].Beschreibung != "")
		{
			char mwstbuffer[30];
			CBetrag dummy_betrag;
			dummy_betrag.MWSt = Buchungsposten[i].MWSt;
			dummy_betrag.GetMWSt(mwstbuffer);			
			
			sprintf(buffer, "%02d\t%s\t%s\t%d\t%s -> %s",
				i,
				(Buchungsposten[i].Ausgaben ? "A" : "E"),
				mwstbuffer,
				Buchungsposten[i].Abschreibungsjahre,
				Buchungsposten[i].Beschreibung,
				Buchungsposten[i].Rechnungsposten);
		}
		else
		{
			sprintf(buffer, "%02d\t-\t-\t-\t-", i);
		}
		int n = ((CListBox *)GetDlgItem(IDC_LISTE))->AddString(buffer);
		((CListBox *)GetDlgItem(IDC_LISTE))->SetItemData(n, i);
	}
	
	((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->ResetContent();

	return TRUE;  
}

void CEinstellungen1::OnSelchangeListe() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	int m;

	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		sprintf(buffer, "%02d", i);
		SetDlgItemText(IDC_BUCHUNGSPOSTENNUMMER, buffer);
		SetDlgItemText(IDC_EA, (Buchungsposten[i].Ausgaben ? "A" : "E"));
		CBetrag dummy_betrag;
		dummy_betrag.MWSt = Buchungsposten[i].MWSt;
		dummy_betrag.GetMWSt(buffer);
		SetDlgItemText(IDC_MWST, buffer);
		sprintf(buffer, "%2d", Buchungsposten[i].Abschreibungsjahre);
		SetDlgItemText(IDC_AFA, buffer);
		SetDlgItemText(IDC_BUCHUNGSPOSTENTEXT, Buchungsposten[i].Beschreibung);
		
		if (Buchungsposten[i].Ausgaben == FALSE) 
			UpdateCombo("E");
		else
			UpdateCombo("A");
		if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, Buchungsposten[i].Rechnungsposten)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
		else
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
	}
	
}


void CEinstellungen1::OnKillfocusEa() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		GetDlgItemText(IDC_EA, buffer, sizeof(buffer));
		if (*buffer)
		{
			if (*buffer == 'E' || *buffer == 'e')
				Buchungsposten[i].Ausgaben = FALSE;
			else if (*buffer == 'A' || *buffer == 'a')
				Buchungsposten[i].Ausgaben = TRUE;
			else
			{
				AfxMessageBox("Bitte 'E' für Einnahmen-Buchungsposten oder 'A' für Ausgaben-Buchungsposten eingeben!", MB_ICONEXCLAMATION);
				GetDlgItem(IDC_EA)->SetFocus();
				return;
			}
			if (Buchungsposten[i].Ausgaben == FALSE) 
				UpdateCombo("E");
			else
				UpdateCombo("A");

			UpdateListe();
			OnSelchangeEurechnungsposten();
		}
	}	
}

void CEinstellungen1::OnKillfocusMwst() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		GetDlgItemText(IDC_MWST, buffer, sizeof(buffer));
		if (*buffer)
		{
			CBetrag dummy_betrag;
			dummy_betrag.SetMWSt(buffer);
			Buchungsposten[i].MWSt = dummy_betrag.MWSt;
			UpdateListe();
		}
	}	
}

void CEinstellungen1::OnKillfocusAfa() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		GetDlgItemText(IDC_AFA, buffer, sizeof(buffer));
		if (*buffer)
		{
			Buchungsposten[i].Abschreibungsjahre = atoi(buffer);
			if (Buchungsposten[i].Abschreibungsjahre < 1) 
			{
				Buchungsposten[i].Abschreibungsjahre = 1;
				SetDlgItemText(IDC_AFA, "1");
			}
			UpdateListe();
		}
	}	
}

void CEinstellungen1::OnKillfocusBuchungspostentext() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		GetDlgItemText(IDC_BUCHUNGSPOSTENTEXT, buffer, sizeof(buffer));
		Buchungsposten[i].Beschreibung = buffer;
		if (Buchungsposten[i].Beschreibung == "")
		{
			Buchungsposten[i].Ausgaben = TRUE;
			Buchungsposten[i].MWSt = 0;
			Buchungsposten[i].Abschreibungsjahre = 1;
			Buchungsposten[i].Rechnungsposten = "";
		}
		UpdateListe();
	}	
}

void CEinstellungen1::OnSelchangeEurechnungsposten() 
{
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		char buffer[1000];
		GetDlgItemText(IDC_EA, buffer, sizeof(buffer));
		if (*buffer)
		{
			CString cs;

			int m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel();
			if (m == CB_ERR)
				Buchungsposten[i].Rechnungsposten = "";
			else
			{
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetLBText(m, Buchungsposten[i].Rechnungsposten);
			}
			UpdateListe();
		}
	}	
}

void CEinstellungen1::UpdateListe()
{
	char buffer[1000];
	int n = ((CListBox *)GetDlgItem(IDC_LISTE))->GetCurSel();
	if (n >= 0)
	{
		int i = ((CListBox *)GetDlgItem(IDC_LISTE))->GetItemData(n);
		if (Buchungsposten[i].Beschreibung != "")
		{
			char mwstbuffer[30];
			CBetrag dummy_betrag;
			dummy_betrag.MWSt = Buchungsposten[i].MWSt;
			dummy_betrag.GetMWSt(mwstbuffer);			
			
			sprintf(buffer, "%02d\t%s\t%s\t%d\t%s -> %s",
				i,
				(Buchungsposten[i].Ausgaben ? "A" : "E"),
				mwstbuffer,
				Buchungsposten[i].Abschreibungsjahre,
				Buchungsposten[i].Beschreibung,
				Buchungsposten[i].Rechnungsposten);
		}
		else
		{
			sprintf(buffer, "%02d\t-\t-\t-\t-", i);
		}

		((CListBox *)GetDlgItem(IDC_LISTE))->DeleteString(n);
		((CListBox *)GetDlgItem(IDC_LISTE))->InsertString(n, buffer);
		((CListBox *)GetDlgItem(IDC_LISTE))->SetItemData(n, i);
		((CListBox *)GetDlgItem(IDC_LISTE))->SetCurSel(n);
	}
}

// ea: "E" == Einnahmen, "A" == Ausgaben für Einnahmen-Überschußrechnung
void CEinstellungen1::UpdateCombo(CString ea)
{
	int i;
	CString cs;

	int n = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel();
	if (n != CB_ERR)
		((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetLBText(n, cs);

	((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->ResetContent();

	for (i = 0; i < 100; i++)
	{
		if (ea == "E")
		{
			if (!EinnahmenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(EinnahmenRechnungsposten[i]);
		}
		else
		{
			if (!AusgabenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(AusgabenRechnungsposten[i]);
		}
	}
	if (n != CB_ERR)
		((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, cs));
	else
		((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);

	UpdateListe();
}

void CEinstellungen1::OnBildschirmschrift() 
{
	LOGFONT		lf;
	CHOOSEFONT	cf;
	memset(&lf, 0x00, sizeof(lf));
	memset(&cf, 0x00, sizeof(lf));
	cf.lStructSize = sizeof(cf);
	strcpy(lf.lfFaceName, m_Bildschirmschrift.GetBuffer(0));
	if (m_Bildschirmschriftgroesse)
		lf.lfHeight = m_Bildschirmschriftgroesse;
	else
		lf.lfHeight = -17;	
	lf.lfWeight = m_BildschirmschriftFett;
	lf.lfItalic = m_BildschirmschriftKursiv;
	cf.lpLogFont = &lf;
	cf.nSizeMin = 1;
	cf.nSizeMax = 100;
	cf.Flags = CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_NOSCRIPTSEL|CF_NOVERTFONTS|CF_SCALABLEONLY;
	
	if (ChooseFont(&cf))
	{
		m_Bildschirmschrift = lf.lfFaceName;
		m_Bildschirmschriftgroesse = abs(lf.lfHeight);
		m_BildschirmschriftFett = lf.lfWeight;
		m_BildschirmschriftKursiv = lf.lfItalic;
		m_pView->RedrawWindow();
		m_pDoc->UpdateAllViews(NULL);
	}
	else
	{
		DWORD dw = CommDlgExtendedError();
		if (dw)
		{
			char buffer[1000];
			sprintf	(buffer, "Common Dialog Box Fehler #%ld", dw);
			AfxMessageBox(buffer);
		}
	}
}

void CEinstellungen1::OnDruckerschrift() 
{
	LOGFONT		lf;
	CHOOSEFONT	cf;
	memset(&lf, 0x00, sizeof(lf));
	memset(&cf, 0x00, sizeof(lf));
	cf.lStructSize = sizeof(cf);
	strcpy(lf.lfFaceName, m_Druckerschrift.GetBuffer(0));
	if (m_Bildschirmschriftgroesse)
		lf.lfHeight = m_Druckerschriftgroesse;
	else
		lf.lfHeight = -17;
	lf.lfWeight = m_DruckerschriftFett;
	lf.lfItalic = m_DruckerschriftKursiv;
	cf.lpLogFont = &lf;
	cf.nSizeMin = 1;
	cf.nSizeMax = 100;
	cf.Flags = CF_BOTH|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT|CF_NOSCRIPTSEL|CF_NOVERTFONTS|CF_SCALABLEONLY;
	
	if (ChooseFont(&cf))
	{
		m_Druckerschrift = lf.lfFaceName;
		m_Druckerschriftgroesse = abs(lf.lfHeight);
		m_DruckerschriftFett = lf.lfWeight;
		m_DruckerschriftKursiv = lf.lfItalic;
	}
	else
	{
		DWORD dw = CommDlgExtendedError();
		if (dw)
		{
			char buffer[1000];
			sprintf	(buffer, "Common Dialog Box Fehler #%ld", dw);
			AfxMessageBox(buffer);
		}
	}
}

void CEinstellungen1::OnAfaGenauigkeit() 
{
	CAfAGenauigkeit dlg;
	dlg.m_afa_genauigkeit = m_AbschreibungGenauigkeit;
	if (dlg.DoModal() == IDOK)
		m_AbschreibungGenauigkeit = dlg.m_afa_genauigkeit;	
}

void CEinstellungen1::OnKillfocusSondervorauszahlung() 
{
	UpdateData();
	int n = currency_to_int(m_sondervorauszahlung.GetBuffer(100));
	int_to_currency(n, 4, m_sondervorauszahlung.GetBuffer(100));
	UpdateData(FALSE);	
}
