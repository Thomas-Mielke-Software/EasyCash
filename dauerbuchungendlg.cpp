// DauerbuchungenDlg.cpp : implementation file
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
#include "DauerbuchungenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DauerbuchungenDlg dialog


DauerbuchungenDlg::DauerbuchungenDlg(CEasyCashDoc *pDoc, BOOL ausgaben, CEasyCashView* pParent /*=NULL*/)
	: CDialog(DauerbuchungenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(DauerbuchungenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDoc = pDoc;
	m_pParent = pParent;
	
#if (_MSC_VER >= 1600)
	EnableDynamicLayout(TRUE);
#endif
}


void DauerbuchungenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DauerbuchungenDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DauerbuchungenDlg, CDialog)
	//{{AFX_MSG_MAP(DauerbuchungenDlg)
	ON_BN_CLICKED(IDC_AENDERN, OnAendern)
	ON_BN_CLICKED(IDC_Neu, OnNeu)
	ON_BN_CLICKED(IDC_LOESCHEN, OnLoeschen)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_VERWERFEN, OnVerwerfen)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	ON_EN_KILLFOCUS(IDC_DATUM_BIS_JAHR, OnKillfocusDatumBisJahr)
	ON_EN_KILLFOCUS(IDC_DATUM_VON_JAHR, OnKillfocusDatumVonJahr)
	ON_BN_CLICKED(IDC_EINNAHMEN, OnEinnahmen)
	ON_BN_CLICKED(IDC_AUSGABEN, OnAusgaben)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_BESCHREIBUNG, OnSelchangeBeschreibung)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DauerbuchungenDlg message handlers

//--- Initialisierung --------------


BOOL DauerbuchungenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	EnableToolTips();
	
	((CEdit *)GetDlgItem(IDC_BESCHREIBUNG))->LimitText(27);

	// Combo-Box für E/Ü-Konten
	((CComboBox *)GetDlgItem(IDC_MWST))->ResetContent();
	int i;
	for (i = 0; *m_pParent->GetVATs(i); i++)
		((CComboBox *)GetDlgItem(IDC_MWST))->AddString(m_pParent->GetVATs(i));		

	// Combo-Boxen für Betriebe und Bestandskonten aufbauen
	for (i = 0; i < 100; i++)
	{
		char inifile[1000], buffer[1000]; 
		GetIniFileName(inifile, sizeof(inifile));
		CString csKey;
		csKey.Format("Betrieb%02dName", i);
		GetPrivateProfileString("Betriebe", csKey, "", buffer, sizeof(buffer), inifile);
		if (!*buffer) break;
		((CComboBox *)GetDlgItem(IDC_BETRIEB))->AddString(buffer);
	}
	for (i = 0; i < 100; i++)
	{
		char inifile[1000], buffer[1000]; 
		GetIniFileName(inifile, sizeof(inifile));
		CString csKey;
		csKey.Format("Bestandskonto%02dName", i);
		GetPrivateProfileString("Bestandskonten", csKey, "", buffer, sizeof(buffer), inifile);
		if (!*buffer) break;
		((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->AddString(buffer);
	}

	InitCtrls();
	EnableCtrls(FALSE);	

	newFlag = FALSE;
	aktDauerbuchung = NULL;
	
	SetTimer(101, 1, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//--- Listbox-Knöpfe ----------------

void DauerbuchungenDlg::OnNeu() 
{
	InitCtrls();
	EnableCtrls(TRUE);	
	GetDlgItem(IDC_DATUM_AKT_MONAT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATUM_AKT_JAHR)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATUM_VON_MONAT)->SetFocus();
	newFlag = TRUE;
}

void DauerbuchungenDlg::OnDblclkList() 
{
	OnAendern();
}

void DauerbuchungenDlg::OnAendern() 
{
	int n;
	CDauerbuchung *dbp;	
	char buf[300];

	n = ((CListBox *)GetDlgItem(IDC_LIST))->GetCurSel();
	if (n >= 0)
	{
		dbp = (CDauerbuchung *)((CListBox *)GetDlgItem(IDC_LIST))->GetItemData(n);
		aktDauerbuchung = dbp;

		InitCtrls();
		EnableCtrls(TRUE);	
		GetDlgItem(IDC_DATUM_VON_MONAT)->SetFocus();
		newFlag = FALSE;
		
		if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
		{
			((CButton *)GetDlgItem(IDC_EINNAHMEN))->SetCheck(TRUE);
			((CButton *)GetDlgItem(IDC_AUSGABEN))->SetCheck(FALSE);
			UpdateCombo("E");
			UpdateBeschreibungCombo("E");
		}
		else
		{
			((CButton *)GetDlgItem(IDC_EINNAHMEN))->SetCheck(FALSE);
			((CButton *)GetDlgItem(IDC_AUSGABEN))->SetCheck(TRUE);
			UpdateCombo("A");
			UpdateBeschreibungCombo("A");
		}

		((CButton *)GetDlgItem(IDC_MONATLICH))->SetCheck(dbp->Intervall == INTERVALL_MONAT);
		((CButton *)GetDlgItem(IDC_QUARTALSMAESSIG))->SetCheck(dbp->Intervall == INTERVALL_QUARTAL);
		((CButton *)GetDlgItem(IDC_HALBJAEHRLICH))->SetCheck(dbp->Intervall == INTERVALL_HALBJAHR);
		((CButton *)GetDlgItem(IDC_JAEHRLICH))->SetCheck(dbp->Intervall == INTERVALL_JAHR);

		sprintf(buf, "%d", dbp->Buchungstag);
		SetDlgItemText(IDC_DATUM_TAG, buf);
		sprintf(buf, "%d", (int)dbp->VonDatum.GetMonth());
		SetDlgItemText(IDC_DATUM_VON_MONAT, buf);
		sprintf(buf, "%d", (int)dbp->BisDatum.GetMonth());
		SetDlgItemText(IDC_DATUM_BIS_MONAT, buf);
		sprintf(buf, "%d", (int)dbp->VonDatum.GetYear());
		SetDlgItemText(IDC_DATUM_VON_JAHR, buf);
		sprintf(buf, "%d", (int)dbp->BisDatum.GetYear());
		SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
		sprintf(buf, "%d", (int)dbp->AktualisiertBisDatum.GetMonth());
		SetDlgItemText(IDC_DATUM_AKT_MONAT, buf);
		sprintf(buf, "%d", (int)dbp->AktualisiertBisDatum.GetYear());
		SetDlgItemText(IDC_DATUM_AKT_JAHR, buf);
		SetDlgItemText(IDC_BESCHREIBUNG, (LPCSTR)dbp->Beschreibung);
		SetDlgItemText(IDC_BELEGNUMMER2, (LPCSTR)dbp->Belegnummer);
		int_to_currency(dbp->Betrag, 4, buf);
		SetDlgItemText(IDC_BETRAG, buf);
		dbp->GetMWSt(buf);
		SetDlgItemText(IDC_MWST, buf);
		int m;
		if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, dbp->Konto)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
		else
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
		if ((m = ((CComboBox *)GetDlgItem(IDC_BETRIEB))->FindStringExact(0, dbp->Betrieb)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_BETRIEB))->SetCurSel(m);
		else
			((CComboBox *)GetDlgItem(IDC_BETRIEB))->SetCurSel(-1);
		if ((m = ((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->FindStringExact(0, dbp->Bestandskonto)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->SetCurSel(m);
		else
			((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->SetCurSel(-1);
	}
}

void DauerbuchungenDlg::OnLoeschen() 
{
	int n;
	CDauerbuchung *dbp;	

	n = ((CListBox *)GetDlgItem(IDC_LIST))->GetCurSel();
	if (n >= 0)
	{
		dbp = (CDauerbuchung *)((CListBox *)GetDlgItem(IDC_LIST))->GetItemData(n);
		n = ((CListBox *)GetDlgItem(IDC_LIST))->DeleteString(n);

		if (dbp)
		{
			CDauerbuchung **dbpp;	

			if (dbp == m_pDoc->Dauerbuchungen)
			{
				m_pDoc->Dauerbuchungen = dbp->next;
				dbp->next = NULL;
				delete dbp;
				m_pDoc->SetModifiedFlag("Dauerbuchung wurde gelöscht");
				return;
			}

			for (dbpp = &(m_pDoc->Dauerbuchungen); *dbpp; dbpp = &((*dbpp)->next))
			{
				if ((*dbpp)->next == dbp)
				{
					CDauerbuchung *dbp_next;
					
					dbp_next = dbp->next;
					dbp->next = NULL;
					delete dbp;
					(*dbpp)->next = dbp_next;
					m_pDoc->SetModifiedFlag("Dauerbuchung wurde gelöscht");
					break;
				}
			}
		}
	}
}


//--- Eingabe beenden -----------

void DauerbuchungenDlg::OnVerwerfen() 
{
	newFlag = FALSE;
	//aktDauerbuchung = NULL;
	
	InitCtrls();
	EnableCtrls(FALSE);	
}

void DauerbuchungenDlg::OnOK() 
{
	CDauerbuchung **p;

	if (newFlag)
	{
		p = &(m_pDoc->Dauerbuchungen);	// bei Neu

		while (*p)
			p = &((*p)->next);

		*p = new CDauerbuchung;
		(*p)->next = NULL;
	}
	else
		p = &aktDauerbuchung;	// bei Ändern

	if (((CButton *)GetDlgItem(IDC_EINNAHMEN))->GetCheck())
		(*p)->Buchungstyp = BUCHUNGSTYP_EINNAHMEN;
	else
		(*p)->Buchungstyp = BUCHUNGSTYP_AUSGABEN;

	if (((CButton *)GetDlgItem(IDC_MONATLICH))->GetCheck())
		(*p)->Intervall = INTERVALL_MONAT;
	else if (((CButton *)GetDlgItem(IDC_QUARTALSMAESSIG))->GetCheck())
		(*p)->Intervall = INTERVALL_QUARTAL;
	else if (((CButton *)GetDlgItem(IDC_HALBJAEHRLICH))->GetCheck())
		(*p)->Intervall = INTERVALL_HALBJAHR;
	else if (((CButton *)GetDlgItem(IDC_JAEHRLICH))->GetCheck())
		(*p)->Intervall = INTERVALL_JAHR;
	else
		(*p)->Intervall = INTERVALL_2MONATE;

	char buf[100];
	int mv, jv, mb, jb;
	GetDlgItemText(IDC_DATUM_TAG, buf, sizeof(buf)); (*p)->Buchungstag = atoi(buf);
	if ((*p)->Buchungstag > 28) (*p)->Buchungstag = 28;
	GetDlgItemText(IDC_DATUM_VON_MONAT, buf, sizeof(buf)); mv = atoi(buf);
	if (mv < 1 || mv > 12)
	{
		MessageBox("Eingabefehler: Wert für Monat von 1 bis 12!", NULL, MB_ICONSTOP);
		GetDlgItem(IDC_DATUM_VON_MONAT)->SetFocus();
		goto error_delete_buchung;
	}
	GetDlgItemText(IDC_DATUM_VON_JAHR, buf, sizeof(buf)); jv = atoi(buf);
	if (jv < 100 && jv > 37)
	{
		jv += 1900;
	}
	else if (jv <= 37 && jv >= 0)
	{
		jv += 2000;
	}
	GetDlgItemText(IDC_DATUM_BIS_MONAT, buf, sizeof(buf)); mb = atoi(buf);
	if (mb < 1 || mb > 12)
	{
		MessageBox("Eingabefehler: Wert für Monat von 1 bis 12!", NULL, MB_ICONSTOP);
		GetDlgItem(IDC_DATUM_BIS_MONAT)->SetFocus();
		goto error_delete_buchung;
	}
	GetDlgItemText(IDC_DATUM_BIS_JAHR, buf, sizeof(buf)); jb = atoi(buf);
	if (jb < 100 && jb > 37)
	{
		jb += 1900;
	}
	else if (jb <= 37 && jb >= 0)
	{
		jb += 2000;
	}
	if (jv > 3000 || jb > 3000)
	{
		MessageBox("Betriebssystem-Beschränkung: Die Jahreszahl darf nicht größer als 3000 sein - sorry!", NULL, MB_ICONSTOP);
		if (jv > 3000) goto error_delete_buchung;
		if (jb > 3000) 
		{
			jb = 3000;
			SetDlgItemText(IDC_DATUM_BIS_JAHR, "3000");
		}
	}

	{
		CTime datv(jv, mv, 1, 0, 0, 0);
		(*p)->VonDatum = datv;	
		CTime *pdatb = new CTime(jb, mb, 31, 0, 0, 0);
		if (pdatb->GetDay() != 31) 
		{ 
			delete pdatb; 
			pdatb = new CTime(jb, mb, 30, 0, 0, 0); 

			if (pdatb->GetDay() != 30) 
			{ 
				delete pdatb; 
				pdatb = new CTime(jb, mb, 29, 0, 0, 0); 

				if (pdatb->GetDay() != 29) 
				{ 
					delete pdatb; 
					pdatb = new CTime(jb, mb, 28, 0, 0, 0); 
				}
			}
		}
		(*p)->BisDatum = *pdatb;	delete pdatb; 
		if ((*p)->VonDatum > (*p)->BisDatum)
		{
			MessageBox("Von-Datum muß kleiner sein als Bis-Datum!", NULL, MB_ICONSTOP);
			GetDlgItem(IDC_DATUM_VON_MONAT)->SetFocus();
			goto error_delete_buchung;
		}

		// bei Ändern kann man auch das AktualisiertBis Datum zurücksetzen
		if (!newFlag)
		{
			int ma, ja;
			GetDlgItemText(IDC_DATUM_AKT_MONAT, buf, sizeof(buf)); ma = atoi(buf);
			if (ma < 1 || ma > 12)
			{
				MessageBox("Eingabefehler: Wert für Monat von 1 bis 12!", NULL, MB_ICONSTOP);
				GetDlgItem(IDC_DATUM_AKT_MONAT)->SetFocus();
				goto error_delete_buchung;
			}

			GetDlgItemText(IDC_DATUM_AKT_JAHR, buf, sizeof(buf)); ja = atoi(buf);
			if (ja < 100 && ja > 37)
			{
				ja += 1900;
			}
			else if (ja <= 37 && ja >= 0)
			{
				ja += 2000;
			}
			if (ja > 3000) ja = 3000;

			(*p)->AktualisiertBisDatum = CTime(ja, ma, 1, 0, 0, 0);

			if ((*p)->AktualisiertBisDatum > (*p)->BisDatum)
			{
				MessageBox("Hinweis: Das Aktualisiert-Bis-Datum liegt nach dem Bis-Datum. Diese Dauerbuchung wird deshalb nicht dazu führen, dass reale Buchungen erzeugt werden.", NULL, MB_ICONSTOP);
				GetDlgItem(IDC_DATUM_VON_MONAT)->SetFocus();
				goto error_delete_buchung;
			}
		}
		else
		{
			CTime data(2000, 1, 1, 0, 0, 0);
			if (newFlag) (*p)->AktualisiertBisDatum = data;	
		}
		
		GetDlgItemText(IDC_BETRAG, buf, sizeof(buf));
		(*p)->Betrag = currency_to_int(buf);
		if (!strlen(buf))
		{
			MessageBox("Keinen Betrag angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}

		GetDlgItemText(IDC_BESCHREIBUNG, buf, sizeof(buf));
		(*p)->Beschreibung = buf;
		if (!strlen(buf))
		{
			MessageBox("Keine Beschreibung angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}
		GetDlgItemText(IDC_BELEGNUMMER2, buf, sizeof(buf));
		(*p)->Belegnummer = buf;

		GetDlgItemText(IDC_MWST, buf, sizeof(buf)); 
		if (!strlen(buf) || !(*p)->SetMWSt(buf))
		{
			MessageBox("Keinen gültigen MWSt-Satz angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}

		int m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel();
		if (m == CB_ERR)
			(*p)->Konto = "";
		else
		{
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetLBText(m, (*p)->Konto);
		}
		
		m = ((CComboBox *)GetDlgItem(IDC_BETRIEB))->GetCurSel();
		if (m == CB_ERR)
			(*p)->Betrieb = "";
		else
		{
			((CComboBox *)GetDlgItem(IDC_BETRIEB))->GetLBText(m, (*p)->Betrieb);
		}
		
		m = ((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->GetCurSel();
		if (m == CB_ERR)
			(*p)->Bestandskonto = "";
		else
		{
			((CComboBox *)GetDlgItem(IDC_BESTANDSKONTO))->GetLBText(m, (*p)->Bestandskonto);
		}
		
	//normal_exit:
		m_pDoc->SetModifiedFlag(newFlag ? "Dauerbuchung wurde angelegt" : "Dauerbuchung wurde geändert");
		
		aktDauerbuchung = (*p);

		InitCtrls();
		EnableCtrls(FALSE);	
	}

common_exit:		
	//newFlag = FALSE;

	return;

error_delete_buchung:
	if (newFlag)
	{
		delete *p;
		*p = NULL;
	}
	goto common_exit;
}


//--- Schließen -------------------------
void DauerbuchungenDlg::OnCancel() 
{
	//m_pParent->dauerbuchungenDlg = NULL;
	
	CDialog::OnCancel();
}

void DauerbuchungenDlg::OnClose() 
{
	//m_pParent->dauerbuchungenDlg = NULL;
		
	CDialog::OnClose();
}


//--- Hilfsfunktionen -----

void DauerbuchungenDlg::EnableCtrls(BOOL b)
{
	GetDlgItem(IDC_LIST)->EnableWindow(!b);
	GetDlgItem(IDC_Neu)->EnableWindow(!b);
	GetDlgItem(IDC_AENDERN)->EnableWindow(!b);
	GetDlgItem(IDC_LOESCHEN)->EnableWindow(!b);

	GetDlgItem(IDC_DATUM_VON_MONAT)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_VON_JAHR)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_BIS_MONAT)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_BIS_JAHR)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_AKT_MONAT)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_AKT_JAHR)->EnableWindow(b);
	GetDlgItem(IDC_DATUM_TAG)->EnableWindow(b);
	GetDlgItem(IDC_BETRAG)->EnableWindow(b);
	GetDlgItem(IDC_BESCHREIBUNG)->EnableWindow(b);
	GetDlgItem(IDC_BELEGNUMMER2)->EnableWindow(b);
	GetDlgItem(IDC_EURECHNUNGSPOSTEN)->EnableWindow(b);
	GetDlgItem(IDC_MWST)->EnableWindow(b);
	GetDlgItem(IDC_EINNAHMEN)->EnableWindow(b);
	GetDlgItem(IDC_AUSGABEN)->EnableWindow(b);
	GetDlgItem(IDC_MONATLICH)->EnableWindow(b);
	GetDlgItem(IDC_QUARTALSMAESSIG)->EnableWindow(b);
	GetDlgItem(IDC_HALBJAEHRLICH)->EnableWindow(b);
	GetDlgItem(IDC_JAEHRLICH)->EnableWindow(b);
	GetDlgItem(IDC_2MONATLICH)->EnableWindow(b);
	GetDlgItem(IDC_VERWERFEN)->EnableWindow(b);
	GetDlgItem(IDOK)->EnableWindow(b);
	GetDlgItem(IDC_BETRIEB)->EnableWindow(b);
	GetDlgItem(IDC_BESTANDSKONTO)->EnableWindow(b);
}

void DauerbuchungenDlg::InitCtrls()
{
	// Listbox
	char buf[300];
	int TabStopArray[3] = { 150, 190, 230 };
	((CListBox *)GetDlgItem(IDC_LIST))->SetTabStops(3, TabStopArray);
	((CListBox *)GetDlgItem(IDC_LIST))->ResetContent();
	CDauerbuchung *dbp;	
	int i;
	for (i = 0, dbp = m_pDoc->Dauerbuchungen; dbp; i++, dbp = dbp->next)
	{
		char buf2[100];
		int_to_currency(dbp->Betrag, 4, buf2);
		sprintf(buf ,"%s\t%s\t%s\tbis %02d.%02d.%04d", (LPCSTR)dbp->Beschreibung, buf2, 
			dbp->Intervall == INTERVALL_MONAT ? "monatlich" : 
			dbp->Intervall == INTERVALL_2MONATE ? "2-monatlich" : 
			dbp->Intervall == INTERVALL_QUARTAL ? "pro quartal" : 
			dbp->Intervall == INTERVALL_HALBJAHR ? "halbjährlich" : "jährlich",
			dbp->BisDatum.GetDay(), dbp->BisDatum.GetMonth(), dbp->BisDatum.GetYear());
		((CListBox *)GetDlgItem(IDC_LIST))->AddString(buf);
		((CListBox *)GetDlgItem(IDC_LIST))->SetItemData(
			((CListBox *)GetDlgItem(IDC_LIST))->GetCount()-1, (DWORD)dbp);
		if (dbp == aktDauerbuchung) 
			((CListBox *)GetDlgItem(IDC_LIST))->SetCurSel(
				((CListBox *)GetDlgItem(IDC_LIST))->GetCount()-1);
	}
	
	// Rest
	char buf_window_text[300];

	((CButton *)GetDlgItem(IDC_EINNAHMEN))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_AUSGABEN))->SetCheck(TRUE);
	UpdateCombo("A");
	UpdateBeschreibungCombo("A");
	
	((CButton *)GetDlgItem(IDC_MONATLICH))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_QUARTALSMAESSIG))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_HALBJAEHRLICH))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_JAEHRLICH))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_2MONATLICH))->SetCheck(FALSE);

	strcpy(buf_window_text, "Dauerbuchungen für ");
	strcat(buf_window_text, (LPCTSTR)m_pDoc->GetPathName());
	SetWindowText(buf_window_text);

	CTime now = CTime::GetCurrentTime();
	SetDlgItemText(IDC_DATUM_TAG, "1");
	sprintf(buf, "%d", (int)now.GetMonth());
	SetDlgItemText(IDC_DATUM_VON_MONAT, buf);
	SetDlgItemText(IDC_DATUM_BIS_MONAT, "12");
	sprintf(buf, "%d", (int)now.GetYear());
	SetDlgItemText(IDC_DATUM_VON_JAHR, buf);
	sprintf(buf, "%d", (int)now.GetYear() + 50);
	SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
	SetDlgItemText(IDC_DATUM_AKT_MONAT, "1");
	SetDlgItemText(IDC_DATUM_AKT_JAHR, "2000");
	SetDlgItemText(IDC_BESCHREIBUNG, "");
	SetDlgItemText(IDC_BELEGNUMMER2, "");
	SetDlgItemText(IDC_BETRAG, "");
	((CComboBox *)GetDlgItem(IDC_MWST))->ResetContent();
	for (i = 0; *m_pParent->GetVATs(i); i++)
		((CComboBox *)GetDlgItem(IDC_MWST))->AddString(m_pParent->GetVATs(i));		
	SetDlgItemText(IDC_MWST, m_pParent->GetDefaultVAT());
	((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
}

void DauerbuchungenDlg::OnKillfocusDatumBisJahr() 
{
	int j;
	char buf[100];

	GetDlgItemText(IDC_DATUM_BIS_JAHR, buf, sizeof(buf)); j = atoi(buf);
	if (j < 100 && j > 37)
	{
		j += 1900;
	}
	else if (j <= 37 && j >= 0)
	{
		j += 2000;
	}
	sprintf(buf, "%d", j);
	SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
}

void DauerbuchungenDlg::OnKillfocusDatumVonJahr() 
{
	int j;
	char buf[100];

	GetDlgItemText(IDC_DATUM_VON_JAHR, buf, sizeof(buf)); j = atoi(buf);
	if (j < 100 && j > 37)
	{
		j += 1900;
	}
	else if (j <= 37 && j >= 0)
	{
		j += 2000;
	}
	sprintf(buf, "%d", j);
	SetDlgItemText(IDC_DATUM_VON_JAHR, buf);
}

// ea: "E" == Einnahmen, "A" == Ausgaben für Einnahmen-Überschußrechnung
void DauerbuchungenDlg::UpdateCombo(CString ea)
{
	int i;

	((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->ResetContent();

	for (i = 0; i < 100; i++)
	{
		if (ea == "E")
		{
			if (!m_pParent->einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(m_pParent->einstellungen1->EinnahmenRechnungsposten[i]);
		}
		else
		{
			if (!m_pParent->einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(m_pParent->einstellungen1->AusgabenRechnungsposten[i]);
		}
	}
}

void DauerbuchungenDlg::UpdateBeschreibungCombo(CString ea)
{
	BOOL bAusgaben = (ea != "E");

	// Combo-Box löschen und neu aufbauen
	((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->ResetContent();
	int i;
	for (i = 0; i < 100; i++)
	{
		char buffer[1000];
		if (m_pParent->einstellungen1->Buchungsposten[i].Beschreibung != "" 
			&& bAusgaben == m_pParent->einstellungen1->Buchungsposten[i].Ausgaben)
		{
			sprintf(buffer, "%02d %s",
				i, m_pParent->einstellungen1->Buchungsposten[i].Beschreibung);
			int n = ((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->AddString(buffer);
			((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->SetItemData(n, i);
		}
	}
}

void DauerbuchungenDlg::OnEinnahmen() 
{
	UpdateCombo("E");
	UpdateBeschreibungCombo("E");
}

void DauerbuchungenDlg::OnAusgaben() 
{
	UpdateCombo("A");
	UpdateBeschreibungCombo("A");
}

void DauerbuchungenDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == 101)
	{
		int x = 0, y = 0, cx = -1, cy = -1;
		RECT rWnd;
		RECT rScreen;

		GetWindowRect(&rWnd);
		GetDesktopWindow()->GetWindowRect(&rScreen);
		
		x = theApp.GetProfileInt("Fenster", "DauerbuchungenPosX", 100);
		y = theApp.GetProfileInt("Fenster", "DauerbuchungenPosY", 50);
		cx = theApp.GetProfileInt("Fenster", "DauerbuchungenSizeX", -1);
		cy = theApp.GetProfileInt("Fenster", "DauerbuchungenSizeY", -1);

		if (x > rScreen.right-(rWnd.right-rWnd.left)) x = rScreen.right-(rWnd.right-rWnd.left);
		if (x < 0) x = 0;
		if (y > rScreen.bottom-(rWnd.bottom-rWnd.top)) y = rScreen.bottom-(rWnd.bottom-rWnd.top);
		if (y < 0) y = 0;

		if (cx > 0 && cy > 0)  // wenn beide aktiv: dynamisches layout aktivieren
			SetWindowPos(NULL, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		else
			SetWindowPos(NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
		
		KillTimer(nIDEvent);
	}
	else if (nIDEvent < 100 && nIDEvent >= 0)
	{
		SetDlgItemText(IDC_BESCHREIBUNG, (LPCSTR)m_pParent->einstellungen1->Buchungsposten[nIDEvent].Beschreibung);
		KillTimer(nIDEvent);
	}
	
	CDialog::OnTimer(nIDEvent);
}

void DauerbuchungenDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	RECT r;
	GetWindowRect(&r);

	theApp.WriteProfileInt("Fenster", "DauerbuchungenPosX", r.left);
	theApp.WriteProfileInt("Fenster", "DauerbuchungenPosY", r.top);
	theApp.WriteProfileInt("Fenster", "DauerbuchungenSizeX", r.right - r.left);
	theApp.WriteProfileInt("Fenster", "DauerbuchungenSizeY", r.bottom - r.top);
}

void DauerbuchungenDlg::OnSelchangeBeschreibung() 
{
	int n = ((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->GetCurSel();
	if (n >= 0)
	{
		int i = (int)((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->GetItemData(n);

		SetTimer(i, 1, NULL);
		char mwstbuffer[30];
		CBetrag dummy_betrag;
		dummy_betrag.MWSt = m_pParent->einstellungen1->Buchungsposten[i].MWSt;
		dummy_betrag.GetMWSt(mwstbuffer);			
		SetDlgItemText(IDC_MWST, mwstbuffer);
	
		int m;
		if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, m_pParent->einstellungen1->Buchungsposten[i].Rechnungsposten)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
		else
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
	}
}

BOOL DauerbuchungenDlg::OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(id);

	NMTTDISPINFO* pTTT = (NMTTDISPINFO*)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	BOOL bRet = FALSE;

	if (pTTT->uFlags & TTF_IDISHWND)
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
		switch (nID)
		{
		case IDC_Neu: pTTT->lpszText = _T("neue Dauerbuchung anlegen"); goto finish;
		case IDC_AENDERN: pTTT->lpszText = _T("bestehende Dauerbuchung ändern -- bitte zuvor oben in der Liste die gewünschte Dauerbuchung auswählen"); goto finish;
		case IDC_LOESCHEN: pTTT->lpszText = _T("bestehende Dauerbuchung löschen -- bitte zuvor oben in der Liste die gewünschte Dauerbuchung auswählen"); goto finish;
		case IDC_EINNAHMEN: pTTT->lpszText = _T("zu erzeugende Buchungen sollen vom Typ 'Einnahmen' sein"); goto finish;
		case IDC_AUSGABEN: pTTT->lpszText = _T("zu erzeugende Buchungen sollen vom Typ 'Ausgaben' sein"); goto finish;
		case IDC_MONATLICH: pTTT->lpszText = _T("die Buchung wird monatlich erzeugt"); goto finish;
		case IDC_QUARTALSMAESSIG: pTTT->lpszText = _T("die Buchung wird vierteljährlich erzeugt"); goto finish;
		case IDC_HALBJAEHRLICH: pTTT->lpszText = _T("die Buchung wird halbjährlich erzeugt"); goto finish;
		case IDC_JAEHRLICH: pTTT->lpszText = _T("die Buchung wird halbjährlich erzeugt"); goto finish;
		case IDC_2MONATLICH: pTTT->lpszText = _T("die Buchung wird alle zwei Monate erzeugt"); goto finish;
		case IDC_DATUM_VON_STATIC:
		case IDC_DATUM_VON_MONAT: pTTT->lpszText = _T("Monat der erste Ausführung"); goto finish;
		case IDC_DATUM_VON_JAHR: pTTT->lpszText = _T("Jahr der erste Ausführung"); goto finish;
		case IDC_DATUM_BIS_STATIC: 
		case IDC_DATUM_BIS_MONAT: pTTT->lpszText = _T("Monat der letzten Ausführung"); goto finish;
		case IDC_DATUM_BIS_JAHR: pTTT->lpszText = _T("Jahr der letzten Ausführung"); goto finish;
		case IDC_DATUM_TAG_STATIC:
		case IDC_DATUM_TAG: pTTT->lpszText = _T("Tag im Monat, die in das Buchungsdatum übernommen wird"); goto finish;
		case IDC_DATUM_AKT_STATIC:
		case IDC_DATUM_AKT_MONAT: 
		case IDC_DATUM_AKT_JAHR: pTTT->lpszText = _T("hiermit merkt sich das Programm, bis wann die Buchungen schon ausgeführt wurden; sollte anfangs vor dem Von-Datum liegen; durch zurücksetzen lassen sich Dauerbuchungen noch einmal ausführen"); goto finish;
		case IDC_BETRAG_STATIC:
		case IDC_BETRAG: pTTT->lpszText = _T("Geldbetrag, der wiederholt werden soll"); goto finish;
		case IDC_BESCHREIBUNG_STATIC:
		case IDC_BESCHREIBUNG: pTTT->lpszText = _T("Buchungs-Beschreibungstext, der wiederholt werden soll"); goto finish;
		case IDC_BELEGNUMMER2_STATIC:
		case IDC_BELEGNUMMER2: pTTT->lpszText = _T("Buchungs-Belegnummer, der wiederholt werden soll"); goto finish;
		case IDC_PLATZHALTER_STATIC: pTTT->lpszText = _T("$J = Jahr vierstellig, $j = Jahr zweistellig, $q = Quartal, $h = Halbjahr, $2 = 2-Monats-Zeitraum, $m = Monat (1-12), $M = Monat (01-12), $+m = nächster Monat, $--M = vorletzter Monat mit führender Null"); goto finish;
		case IDC_EURECHNUNGSPOSTEN_STATIC:
		case IDC_EURECHNUNGSPOSTEN: pTTT->lpszText = _T("E/Ü-Konto, auf das der Betrag verbucht werden soll"); goto finish;
		case IDC_MWST_STATIC:
		case IDC_MWST: pTTT->lpszText = _T("Mehrwertsteuersatz"); goto finish;
		case IDC_BETRIEB_STATIC:
		case IDC_BETRIEB: pTTT->lpszText = _T("Betrieb, für den die Buchung angelegt werden soll"); goto finish;
		case IDC_BESTANDSKONTO_STATIC:
		case IDC_BESTANDSKONTO: pTTT->lpszText = _T("Bestandskonto (z.B. 'Bank' oder 'Kasse'), auf das der Betrag verbucht werden soll"); goto finish;
		case IDC_VERWERFEN: pTTT->lpszText = _T("eingegebene Werte nicht in einer neuen Dauerbuchung speichern"); goto finish;
		case IDOK: pTTT->lpszText = _T("eingegebene Werte in einer neuen Dauerbuchung speichern"); goto finish;
		case IDCANCEL: pTTT->lpszText = _T("Dialogfenster schließen"); goto finish;	
		finish:
			pTTT->hinst = AfxGetResourceHandle();
			bRet = TRUE;
			break;
		default:
			bRet = FALSE;
		}
	}

	*pResult = 0;

	return bRet;
}
