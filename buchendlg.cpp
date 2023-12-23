// BuchenDlg.cpp : implementation file
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
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "BuchenDlg.h"
#include "RechnDlg.h"
#include <ctype.h>
#include <rpcdce.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// kleine Hilfsfunktion zur Erzeugung von UUIDs:
extern CString MakeUuidString(UUID* pUUID/*=NULL*/)
{
   CString sUUID = "";
   unsigned char* sTemp;
   BOOL bAllocated = FALSE;

   if (pUUID == NULL)
   {
      pUUID      = new UUID;
      bAllocated = TRUE;
   }
   if (pUUID != NULL)
   {
      HRESULT hr;
      hr = UuidCreate(pUUID);
      if (hr == RPC_S_OK)
      {
         hr = UuidToString(pUUID, &sTemp);
         if (hr == RPC_S_OK)
         {
            sUUID = sTemp;
            sUUID.MakeUpper();
            RpcStringFree(&sTemp);
         }
      }
      if (bAllocated)
      {
         delete pUUID;
         pUUID = NULL;
      }
   }
   return sUUID;
}


/////////////////////////////////////////////////////////////////////////////
// BuchenDlg dialog


BuchenDlg::BuchenDlg(CEasyCashDoc *pDoc, BOOL ausgaben, 
					 CEasyCashView* pParent, CBuchung **ppb, BOOL bKopieren, BOOL bNeueBelegnummer)
	: CDialog(BuchenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(BuchenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_ausgaben = ausgaben;
	m_pDoc = pDoc;
	m_pParent = pParent;
	m_ppb = ppb;	// wenn !=0 wird nur geändert
	m_bKopieren = bKopieren;
	m_bNeueBelegnummer = bNeueBelegnummer;
	m_UpdateBeschreibung = TRUE;
	m_nGewaehlterSplit = -1;

	char inifile[1000], betriebe[2], bestandskonten[2];
	GetIniFileName(inifile, sizeof(inifile));
	GetPrivateProfileString("Betriebe", "Betrieb00Name", "", betriebe, sizeof(betriebe), inifile);
	GetPrivateProfileString("Bestandskonten", "Bestandskonto00Name", "", bestandskonten, sizeof(bestandskonten), inifile);
#if (_MSC_VER >= 1600)
	if (*betriebe && *bestandskonten)
		EnableDynamicLayout(TRUE);
#endif
}


void BuchenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BuchenDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BuchenDlg, CDialog)
	//{{AFX_MSG_MAP(BuchenDlg)
	ON_BN_CLICKED(IDC_AUSGABEN, OnAusgaben)
	ON_BN_CLICKED(IDC_EINNAHMEN, OnEinnahmen)
	ON_WM_CLOSE()
	ON_EN_KILLFOCUS(IDC_DATUM_JAHR, OnKillfocusDatumJahr)
	ON_CBN_SETFOCUS(IDC_BESCHREIBUNG, OnSetfocusBeschreibung)
	ON_CBN_SELCHANGE(IDC_BESCHREIBUNG, OnSelchangeBeschreibung)
	ON_WM_TIMER()
	ON_CBN_EDITCHANGE(IDC_BESCHREIBUNG, OnEditchangeBeschreibung)
	ON_CBN_SELCHANGE(IDC_ABSCHREIBUNGJAHRE, OnSelchangeAbschreibungjahre)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_JAHR_ENABLED, OnJahrEnabled)
	ON_EN_CHANGE(IDC_DATUM_TAG, OnChangeDatumTag)
	ON_EN_CHANGE(IDC_DATUM_JAHR, OnChangeDatumJahr)
	ON_EN_CHANGE(IDC_DATUM_MONAT, OnChangeDatumMonat)
	ON_EN_SETFOCUS(IDC_DATUM_TAG, OnSetfocusDatumTag)
	ON_EN_SETFOCUS(IDC_DATUM_JAHR, OnSetfocusDatumJahr)
	ON_EN_SETFOCUS(IDC_DATUM_MONAT, OnSetfocusDatumMonat)
	ON_EN_CHANGE(IDC_BETRAG, OnChangeBetrag)
	ON_BN_CLICKED(IDC_KONVERTIEREN, OnKonvertieren)
	ON_BN_CLICKED(IDC_SPLIT, OnSplit)
	ON_EN_CHANGE(IDC_BELEGNUMMER, OnChangeBelegnummer)
	ON_BN_CLICKED(IDC_RECHNER, OnRechner)
	ON_BN_CLICKED(IDC_ALT1, OnAlt1)
	ON_BN_CLICKED(IDC_ALT2, OnAlt2)
	ON_BN_CLICKED(IDC_ALT3, OnAlt3)
	ON_BN_CLICKED(IDC_ALT4, OnAlt4)
	ON_BN_CLICKED(IDC_ALT5, OnAlt5)
	ON_BN_CLICKED(IDC_ALT6, OnAlt6)
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_ABSCHREIBUNGNUMMER, &BuchenDlg::OnCbnSelchangeAbschreibungnummer)
	ON_CBN_SELCHANGE(IDC_MWST, &BuchenDlg::OnCbnSelchangeMwst)
	ON_CBN_EDITCHANGE(IDC_MWST, &BuchenDlg::OnCbnEditchangeMwst)
	ON_CBN_EDITCHANGE(IDC_ABSCHREIBUNGJAHRE, &BuchenDlg::OnCbnEditchangeAbschreibungjahre)
	ON_CBN_EDITCHANGE(IDC_ABSCHREIBUNGNUMMER, &BuchenDlg::OnCbnEditchangeAbschreibungnummer)
	ON_BN_CLICKED(IDC_MWST_ENABLED, &BuchenDlg::OnBnClickedMwstEnabled)
	ON_CBN_SELCHANGE(IDC_EURECHNUNGSPOSTEN, &BuchenDlg::OnCbnSelchangeEurechnungsposten)
	ON_BN_CLICKED(IDC_ABGANG_BUCHEN, &BuchenDlg::OnBnClickedAbgangBuchen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BuchenDlg message handlers

BOOL BuchenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

//char *cp = NULL; // provoziere crash um CrashRpt zu testen
//*cp = 'X';	
	Button1Bmp.LoadBitmap(IDB_BUTTON1);
	((CButton *)GetDlgItem(IDOK))->SetBitmap(Button1Bmp);

	StopBmp.LoadBitmap(IDB_STOP);
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(StopBmp);
	SplitBmp.LoadBitmap(IDB_PIE);
	((CButton *)GetDlgItem(IDC_SPLIT))->SetBitmap(SplitBmp);
	RechnerBmp.LoadBitmap(IDB_RECHNER);
	((CButton *)GetDlgItem(IDC_RECHNER))->SetBitmap(RechnerBmp);
	
	PopUp.CreatePopupMenu();

	((CEdit *)GetDlgItem(IDC_BESCHREIBUNG))->LimitText(27);
	((CEdit *)GetDlgItem(IDC_DATUM_TAG))->LimitText(2);
	((CEdit *)GetDlgItem(IDC_DATUM_MONAT))->LimitText(2);
	((CEdit *)GetDlgItem(IDC_DATUM_JAHR))->LimitText(4);
	((CButton *)GetDlgItem(IDC_JAHR_ENABLED))->SetCheck(m_pParent->einstellungen1->m_bJahresfeldAktiviert);
	GetDlgItem(IDC_DATUM_JAHR)->EnableWindow(m_pParent->einstellungen1->m_bJahresfeldAktiviert);
	((CButton *)GetDlgItem(IDC_MWST_ENABLED))->SetCheck(m_pParent->einstellungen1->m_bMwstFeldAktiviert);
	GetDlgItem(IDC_MWST)->EnableWindow(m_pParent->einstellungen1->m_bMwstFeldAktiviert);

	((CComboBox *)GetDlgItem(IDC_MWST))->ResetContent();
	int i;
	char *mwst;
	for (i = 0; *(mwst = m_pParent->GetVATs(i)); i++)
	{
		if (((CComboBox *)GetDlgItem(IDC_MWST))->FindStringExact(0, mwst) < 0)
			((CComboBox *)GetDlgItem(IDC_MWST))->AddString(mwst);
	}

	// Imagelists für Betriebe und Bestandskonten füllen
	{
		CBitmap generische_bitmap;
		m_imgListIcons.Create(32, 32, ILC_COLOR24, 0, 100);
		generische_bitmap.LoadBitmap(IDB_ICONS);
		m_imgListIcons.Add(&generische_bitmap, RGB(255,0,255));
		generische_bitmap.Detach();	
	}
	((CListCtrl *)GetDlgItem(IDC_BETRIEB))->SetImageList(&m_imgListIcons, LVSIL_NORMAL);
	{
		CBitmap generische_bitmap;
		m_imgListIconsBestandskonten.Create(32, 32, ILC_COLOR24, 0, 100);
		generische_bitmap.LoadBitmap(IDB_ICONS_BESTANDSKONTEN);
		m_imgListIconsBestandskonten.Add(&generische_bitmap, RGB(255,0,255));
		generische_bitmap.Detach();	
	}
	((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->SetImageList(&m_imgListIconsBestandskonten, LVSIL_NORMAL);

	// Listen für Betriebe und Bestandskonten aufbauen
	for (i = 0; i < 100; i++)
	{
		char inifile[1000], buffer[1000]; 
		GetIniFileName(inifile, sizeof(inifile));
		CString csKey;
		csKey.Format("Betrieb%02dName", i);
		GetPrivateProfileString("Betriebe", csKey, "", buffer, sizeof(buffer), inifile);
		csKey.Format("Betrieb%02dIcon", i);
		int nIcon = GetPrivateProfileInt("Betriebe", csKey, 0, inifile);
		if (!*buffer) break;
		((CListCtrl *)GetDlgItem(IDC_BETRIEB))->InsertItem(i, buffer, nIcon);
	}
	if (i > 2)	// bei mehr als zwei Einträgen: kleinere Icons benutzen
		((CListCtrl *)GetDlgItem(IDC_BETRIEB))->SetView(LV_VIEW_TILE);
	for (i = 0; i < 100; i++)
	{
		char inifile[1000], buffer[1000]; 
		GetIniFileName(inifile, sizeof(inifile));
		CString csKey;
		csKey.Format("Bestandskonto%02dName", i);
		GetPrivateProfileString("Bestandskonten", csKey, "", buffer, sizeof(buffer), inifile);
		csKey.Format("Bestandskonto%02dIcon", i);
		int nIcon = GetPrivateProfileInt("Bestandskonten", csKey, 0, inifile);
		if (!*buffer) break;
		((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->InsertItem(i, buffer, nIcon);
	}
	if (i > 2)	// bei mehr als zwei Einträgen: kleinere Icons benutzen
		((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->SetView(LV_VIEW_TILE);

/*	KONVERTIERUNGSCODE ERSTMAL AUSSCHALTEN!!!
	if (m_pDoc->csWaehrung == "EUR")
	{// BUTTON: Nach EURO konvertieren
		if (m_pDoc->csUrspruenglicheWaehrung == "" || m_pDoc->csUrspruenglicheWaehrung == "EUR")
		{
			switch (m_pParent->einstellungen2->m_land)
			{
			case 0: m_csZielwaehrung = "DEM"; 
			case 1: m_csZielwaehrung = "ATS"; 
				GetDlgItem(IDC_KONVERTIEREN)->ShowWindow(SW_SHOW);
			case 2: m_csZielwaehrung = ""; 
				break;
			}
		}
		else
		{
			for (i = 1; i < 11; i++)
			{
				if (m_pDoc->csUrspruenglicheWaehrung ==
					CEasyCashDoc::GetWaehrungskuerzel(i))
				{
					GetDlgItem(IDC_KONVERTIEREN)->ShowWindow(SW_SHOW);
					m_csZielwaehrung = m_pDoc->csUrspruenglicheWaehrung;
					break;
				}
			}
		}

		CString cs;
		cs.Format("&Konvertieren: %s -> EUR", m_csZielwaehrung);
		SetDlgItemText(IDC_KONVERTIEREN, cs);
	}
	else
	{
		for (i = 1; i < 11; i++)
		{
			if (m_pDoc->csWaehrung == CEasyCashDoc::GetWaehrungskuerzel(i))
			{
				GetDlgItem(IDC_KONVERTIEREN)->ShowWindow(SW_SHOW);
				m_csZielwaehrung = m_pDoc->csWaehrung;

				CString cs;
				cs.Format("&Konvertieren: EUR -> %s", m_csZielwaehrung);
				SetDlgItemText(IDC_KONVERTIEREN, cs);
				break;
			}
		}
	}
*/
	InitDlg();

	SetTimer(101, 1, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BuchenDlg::OnOK() 
{
	CBuchung **p;

//	GetDlgItem(IDC_KONVERTIEREN)->EnableWindow(TRUE);

	if (m_ppb)	
	{	// Buchung ändern ...
		p = m_ppb;	
	}
	else
	{	// ... oder neue Buchung
		if (m_ausgaben)
			p = &(m_pDoc->Ausgaben);
		else
			p = &(m_pDoc->Einnahmen);

		while (*p)
			p = &((*p)->next);

		*p = new CBuchung;
		(*p)->next = NULL;
	}
	CBuchung *pBuchungsmerker = *p; // p ist nicht mehr gültig nach dem Sortieren!
	
	char buf[10000];
	int	t, m, j;
	GetDlgItemText(IDC_DATUM_TAG, buf, sizeof(buf)); t = atoi(buf);
	if (t < 1 || t > 31)
	{
		MessageBox("Eingabefehler: Wert für Tag von 1 bis 31!", NULL, MB_ICONSTOP);
		GetDlgItem(IDC_DATUM_TAG)->SetFocus();
		goto error_delete_buchung;
	}
	GetDlgItemText(IDC_DATUM_MONAT, buf, sizeof(buf)); m = atoi(buf);
	if (m < 1 || m > 12)
	{
		MessageBox("Eingabefehler: Wert für Monat von 1 bis 12!", NULL, MB_ICONSTOP);
		GetDlgItem(IDC_DATUM_MONAT)->SetFocus();
		goto error_delete_buchung;
	}
	GetDlgItemText(IDC_DATUM_JAHR, buf, sizeof(buf)); j = atoi(buf);
	if (j < 100 && j > 37)
	{
		j += 1900;
	}
	else if (j <= 37 && j >= 0)
	{
		j += 2000;
	}
	if (j < 1990 || j > 3000)
	{
		MessageBox("Eingabefehler: Wert für Jahr von 1990 bis 3000!", NULL, MB_ICONSTOP);
		GetDlgItem(IDC_DATUM_JAHR)->SetFocus();
		goto error_delete_buchung;
	}

	{
		CTime dat(j, m, t, 12, 0, 0);
		(*p)->Datum = dat;	
		
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

		GetDlgItemText(IDC_BELEGNUMMER, buf, sizeof(buf));
		(*p)->Belegnummer = buf;

		GetDlgItemText(IDC_MWST, buf, sizeof(buf)); 
		if (!strlen(buf) || !(*p)->SetMWSt(buf))
		{
			MessageBox("Keinen gültigen MWSt-Satz angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}

		GetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buf, sizeof(buf)); 
		(*p)->AbschreibungJahre = atoi(buf);
		if (!strlen(buf))
		{
			MessageBox("Keinen Abschreibungszeitzaum angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}		

		GetDlgItemText(IDC_ABSCHREIBUNGNUMMER, buf, sizeof(buf)); 
		(*p)->AbschreibungNr = atoi(buf);
		if (!strlen(buf))
		{
			MessageBox("Kein Abschreibungsjahr angegeben!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}	
		if ((*p)->AbschreibungNr > (*p)->AbschreibungJahre + 1)
		{
			MessageBox("Das aktuelle Abschreibungsjahr übersteigt den Abschreibungszeitzaum um mehr als 1! (Hinweis: Für den Fall, dass im ersten Jahr keine ganze Jahresrate abgeschrieben wurde, kann das Abschreibungsjahr die Abschreibungsdauer um eins übersteigen.)", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}	

		if ((*p)->AbschreibungNr > 1)
		{
			GetDlgItemText(IDC_ABSCHREIBUNGRESTWERT, buf, sizeof(buf));
			(*p)->AbschreibungRestwert = currency_to_int(buf);
		}
		else		
			(*p)->AbschreibungRestwert = (*p)->GetNetto();

		(*p)->AbschreibungGenauigkeit = ((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT))->GetCurSel();

		if ((*p)->AbschreibungNr-1 > (*p)->AbschreibungJahre)
		{
			MessageBox("Laufende Abschreibungsnummer ist größer als die Gesamt-Abschreibungsdauer!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}

		if ((*p)->AbschreibungNr < 1)
		{
			MessageBox("Laufende Abschreibungsnummer muss mindestens 1 sein!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}

		if ((*p)->AbschreibungJahre < 1)
		{
			MessageBox("Gesamt-Abschreibungsdauer muss mindestens 1 sein!", NULL, MB_ICONSTOP);
			goto error_delete_buchung;
		}
		
		int m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel();
		if (m == CB_ERR)
			/*(*p)->Konto = ""*/;
		else
		{
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetLBText(m, (*p)->Konto);
			if (m_bUnterkategorien && (*p)->Konto.Left(3) == "   ")
				(*p)->Konto = (*p)->Konto.Mid(3);
		}
		
		if (m_ausgaben && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben && !m_ppb)
		{
			char buffer[100];
			sprintf(buffer, "A%04d", m_pDoc->nLaufendeBuchungsnummerFuerAusgaben);
			if (!strcmp((*p)->Belegnummer.GetBuffer(0), buffer))
				m_pDoc->nLaufendeBuchungsnummerFuerAusgaben++;	// erhöhen nur wenn Belegnummer unverändert geblieben
		}

		if (!m_ausgaben && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen && !m_ppb)
		{
			char buffer[100];
			sprintf(buffer, "E%04d", m_pDoc->nLaufendeBuchungsnummerFuerEinnahmen);
			if (!strcmp((*p)->Belegnummer.GetBuffer(0), buffer))
				m_pDoc->nLaufendeBuchungsnummerFuerEinnahmen++;	// erhöhen nur wenn Belegnummer unverändert geblieben
		}

		// Belegnummer erhöhen für Bank
		{
			char buffer[100];
			sprintf(buffer, "B%04d", m_pDoc->nLaufendeBuchungsnummerFuerBank);
			if (!strcmp((*p)->Belegnummer.GetBuffer(0), buffer))
				m_pDoc->nLaufendeBuchungsnummerFuerBank++;	// erhöhen nur wenn Belegnummer unverändert geblieben
		}

		// Belegnummer erhöhen für Kasse
		{
			char buffer[100];
			sprintf(buffer, "K%04d", m_pDoc->nLaufendeBuchungsnummerFuerKasse);
			if (!strcmp((*p)->Belegnummer.GetBuffer(0), buffer))
				m_pDoc->nLaufendeBuchungsnummerFuerKasse++;	// erhöhen nur wenn Belegnummer unverändert geblieben
		}

		// Betrieb und Bestandskonto speichern
		POSITION pos = ((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetFirstSelectedItemPosition();	
		if (pos)
		{
			int nBetriebSelected = ((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetNextSelectedItem(pos);
			(*p)->Betrieb = ((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetItemText(nBetriebSelected, 0);
		}
		pos = ((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetFirstSelectedItemPosition();	
		if (pos)
		{
			int nBestandskontoSelected = ((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetNextSelectedItem(pos);
			(*p)->Bestandskonto = ((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetItemText(nBestandskontoSelected, 0);
		}

		// Gegenbuchung auf Betriebeinnahmen-Konto, wenn Privat-Split aktiv:
		{
			CString csSplitText;
			GetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, csSplitText);
		
			if (m_nGewaehlterSplit >= 0)
			{
				CString csGewaehlterSplit;
				csGewaehlterSplit.Format("%d", m_nGewaehlterSplit);
				SetErweiterungKey((*p)->Erweiterung, "EasyCash", "GewaehlterSplit", csGewaehlterSplit.GetBuffer(100));
				
				// UUID erzeugen für Identifikation von korrespondierenden Split-Buchungen in Einnahmen
				CString csUUID;
				char *pUUID = GetErweiterungKey((*p)->Erweiterung, "EasyCash", "SplitBasisbuchung");
				if (!*pUUID)
				{
					csUUID = MakeUuidString(NULL);
					SetErweiterungKey((*p)->Erweiterung, "EasyCash", "SplitBasisbuchung", csUUID.GetBuffer(100));
				}
				else
				{
					char *cpUUIDEnde = strchr(pUUID, '|');
					if (cpUUIDEnde)
					{
						char *cpcsUUID = csUUID.GetBuffer(cpUUIDEnde-pUUID+1);
						strncpy(cpcsUUID, pUUID, cpUUIDEnde-pUUID);
						cpcsUUID[cpUUIDEnde-pUUID] = '\0';						
					}
				}
				csUUID.ReleaseBuffer();

				// Textkomponenten aufbereiten
				CString csSplitSatzText = *m_pParent->einstellungen5->m_psatz[m_nGewaehlterSplit];
				int		nSplitSatzWert  = currency_to_int(csSplitSatzText.GetBuffer(0));		// in % * 100	(Bsp.: 100% = 10000)
				CString csSplitUSTSatzText = *m_pParent->einstellungen5->m_pustsatz[m_nGewaehlterSplit];
				int		nSplitUSTSatzWert  = currency_to_int(csSplitUSTSatzText.GetBuffer(0));	// in % * 100
				CString csBeschreibungSplit = (CString)"Privatanteil " + (*p)->Beschreibung + " (" + csSplitSatzText + "%)";
				CString csSplitSatzKonto = *(m_pParent->einstellungen5->m_part[m_nGewaehlterSplit]);
				
				// existiert schon eine Buchung mit Vorsteuerabzug und Verdacht auf Doppelgänger?
				CBuchung *pGegenbuchung = m_pDoc->Einnahmen;
				while (pGegenbuchung)
				{
					if (!strncmp(GetErweiterungKey(pGegenbuchung->Erweiterung, "EasyCash", "SplitGegenbuchungMitVorsteuerabzug"), csUUID.GetBuffer(0), csUUID.GetLength()))
					{
						if (nSplitUSTSatzWert > 0)
						{
							if (AfxMessageBox((CString)"Es existiert für diese Buchung unter Betriebseinahmen bereits eine 'Gegenbuchung' mit Vorsteuerabzug. Ersetzen?", MB_YESNO) == IDYES)
								break;
							else
							{
								pGegenbuchung = NULL;
								break;
							}
						}
						else
						{
							if (AfxMessageBox((CString)"Es existiert für diese Buchung unter Betriebseinahmen bereits eine 'Gegenbuchung' mit Vorsteuerabzug. Löschen?", MB_YESNO) == IDYES)
							{
								CBuchung **ppb = &m_pDoc->Einnahmen;
								CBuchung *pb = pGegenbuchung;

								// Einnahmen nach zu löschender Buchung durchsuchen
								BOOL bFound = FALSE;
								while (*ppb)
								{
									if (*ppb == pb)
									{
										pb = (*ppb)->next;
										(*ppb)->next = NULL;	// ganz wichtig wegen Kettenlöschung
										delete *ppb;
										*ppb = pb;

										bFound = TRUE;
										break;
									}

									ppb = &((*ppb)->next);
								}
							}
							else
								SetErweiterungKey(pGegenbuchung->Beschreibung, "EasyCash", "SplitGegenbuchungMitVorsteuerabzug", "");	// Gegenbuchung-Status entziehen

							pGegenbuchung = NULL;
							break;
						}
					}
					pGegenbuchung = pGegenbuchung->next;
				}

				// Split Gegenbuchung mit Vorsteuerabzug
				if (nSplitUSTSatzWert > 0)
				{
					if (!pGegenbuchung)
					{
						pGegenbuchung = new CBuchung;
						*pGegenbuchung = **p;
						pGegenbuchung->next = m_pDoc->Einnahmen;	// einfügen
						m_pDoc->Einnahmen = pGegenbuchung;		
					}
					pGegenbuchung->Beschreibung = csBeschreibungSplit;
					if (nSplitUSTSatzWert > 0 && nSplitUSTSatzWert < 10000) 
						pGegenbuchung->Beschreibung += " (Anteil mit VST-Abzug)";
					pGegenbuchung->Konto = csSplitSatzKonto;
					LONGLONG ll = (LONGLONG)(*p)->Betrag * (LONGLONG)nSplitSatzWert / 10000;	// erst Privat-Split
					ll *= (LONGLONG)nSplitUSTSatzWert;
					ll /= (LONGLONG)10000;							// dann entspr. VST-Anteilen reduzieren	
					pGegenbuchung->Betrag = (int)ll;
					pGegenbuchung->AbschreibungNr = 1;
					pGegenbuchung->AbschreibungJahre = 1;
					pGegenbuchung->AbschreibungRestwert = 0;
					pGegenbuchung->AbschreibungDegressiv = 0;
					pGegenbuchung->AbschreibungSatz = 0;
					SetErweiterungKey(pGegenbuchung->Erweiterung, "EasyCash", "SplitGegenbuchungMitVorsteuerabzug", csUUID);
				}

				// existiert schon eine Buchung ohne Vorsteuerabzug und Verdacht auf Doppelgänger?
				pGegenbuchung = m_pDoc->Einnahmen;
				while (pGegenbuchung)
				{
					if (!strncmp(GetErweiterungKey(pGegenbuchung->Erweiterung, "EasyCash", "SplitGegenbuchungOhneVorsteuerabzug"), csUUID.GetBuffer(0), csUUID.GetLength()))
					{
						if (nSplitUSTSatzWert < 10000)
						{
							if (AfxMessageBox((CString)"Es existiert für diese Buchung unter Betriebseinahmen bereits eine 'Gegenbuchung' ohne Vorsteuerabzug. Ersetzen?", MB_YESNO) == IDYES)
								break;
							else
							{
								pGegenbuchung = NULL;
								break;
							}
						}
						else
						{
							if (AfxMessageBox((CString)"Es existiert für diese Buchung unter Betriebseinahmen bereits eine 'Gegenbuchung' ohne Vorsteuerabzug. Löschen?", MB_YESNO) == IDYES)
							{
								CBuchung **ppb = &m_pDoc->Einnahmen;
								CBuchung *pb = pGegenbuchung;

								// Einnahmen nach zu löschender Buchung durchsuchen
								BOOL bFound = FALSE;
								while (*ppb)
								{
									if (*ppb == pb)
									{
										pb = (*ppb)->next;
										(*ppb)->next = NULL;	// ganz wichtig wegen Kettenlöschung
										delete *ppb;
										*ppb = pb;

										bFound = TRUE;
										break;
									}

									ppb = &((*ppb)->next);
								}
							}
							else
								SetErweiterungKey(pGegenbuchung->Beschreibung, "EasyCash", "SplitGegenbuchungOhneVorsteuerabzug", "");	// Gegenbuchung-Status entziehen

							pGegenbuchung = NULL;
							break;
						}
					}
					pGegenbuchung = pGegenbuchung->next;
				}

				// Split Gegenbuchung ohne Vorsteuerabzug
				if (nSplitUSTSatzWert < 10000)	// bei vollen VST ist kein Anteil ohne VST nötig
				{
					if (!pGegenbuchung)
					{
						pGegenbuchung = new CBuchung;
						*pGegenbuchung = **p;
						pGegenbuchung->next = m_pDoc->Einnahmen;	// einfügen
						m_pDoc->Einnahmen = pGegenbuchung;		
					}
					pGegenbuchung->Beschreibung = csBeschreibungSplit;
					if (nSplitUSTSatzWert > 0 && nSplitUSTSatzWert < 10000) 
						pGegenbuchung->Beschreibung += " (Anteil ohne VST-Abzug)";
					pGegenbuchung->Konto = csSplitSatzKonto;
//CString csTemp;
//csTemp.Format("a: %d", (int)(*p)->Betrag);
//AfxMessageBox(csTemp);
//csTemp.Format("b: %d", (int)nSplitSatzWert);
//AfxMessageBox(csTemp);
					LONGLONG ll = (LONGLONG)(*p)->Betrag * (LONGLONG)nSplitSatzWert / 10000;	// erst Privat-Split
//csTemp.Format("1: %lld", ll);
//AfxMessageBox(csTemp);
					ll *= 100000;
//csTemp.Format("2: %lld", ll);
//AfxMessageBox(csTemp);
					ll /= (LONGLONG)100000 + (LONGLONG)(*p)->MWSt;						// dann auf Netto runterrechnen
//csTemp.Format("3: %lld", ll);
//AfxMessageBox(csTemp);
					ll *= (LONGLONG)(10000 - nSplitUSTSatzWert);
//csTemp.Format("4: %lld", ll);
//AfxMessageBox(csTemp);
					ll /= (LONGLONG)10000;				// schließlich entspr. VST-Anteilen reduzieren	
//csTemp.Format("5: %lld", ll);
//AfxMessageBox(csTemp);
					pGegenbuchung->Betrag = (int)ll;				
					pGegenbuchung->MWSt = 0;
					pGegenbuchung->AbschreibungNr = 1;
					pGegenbuchung->AbschreibungJahre = 1;
					pGegenbuchung->AbschreibungRestwert = 0;
					pGegenbuchung->AbschreibungDegressiv = 0;
					pGegenbuchung->AbschreibungSatz = 0;
					SetErweiterungKey(pGegenbuchung->Erweiterung, "EasyCash", "SplitGegenbuchungOhneVorsteuerabzug", csUUID);
				}
			}
			else
				SetErweiterungKey((*p)->Erweiterung, "EasyCash", "GewaehlterSplit", "-1");

		}

		m_pDoc->SetModifiedFlag(m_ppb ? "Buchung wurde geändert" : "Neue Buchung wurde hinzugefügt");
		m_pDoc->InkrementBuchungszaehler();

		// dauerte zu lange bei sehr vielen Buchungen, deshalb nur noch bei Split-Buchungen:
		if (m_nGewaehlterSplit >= 0)
			m_pDoc->Sort();		
		else // Neu jetzt:
		{
			if (m_ausgaben)
				m_pDoc->SortSingle(&m_pDoc->Ausgaben, p);
			else
				m_pDoc->SortSingle(&m_pDoc->Einnahmen, p);
		}


		if (m_ppb || m_bKopieren)
			CDialog::OnOK();	// bei Ändern: Dialog schließen
		else
			InitDlg();			// sonst nächste Buchung

		if (m_pParent->einstellungen1->m_taeglich_buchen)
			GetDlgItem(IDC_BETRAG)->SetFocus();
		else
		{
			GetDlgItem(IDC_DATUM_TAG)->SetFocus();
			((CEdit *)GetDlgItem(IDC_DATUM_TAG))->SetSel(0, -1, FALSE);
		}
	}

	if (m_pParent) 
	{
		m_pParent->RedrawWindow();
		m_pDoc->UpdateAllViews(NULL);
	}

	if (!m_ppb) // neue Buchung?
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i ++)
			if (m_pParent->ppPosBuchungsliste[i])
				if (*(m_pParent->ppPosBuchungsliste[i]) == pBuchungsmerker)
				{
					CRect r;
					m_pParent->GetClientRect(&r);
					m_pParent->nSelected = i;	// selektieren
					m_pParent->ScrollToPosition(CPoint(0, max(0, i * m_pParent->charheight - r.bottom / 2)));

					if (m_pParent) 
					{
						//m_pParent->RedrawWindow();
						m_pDoc->UpdateAllViews(NULL);
					}
					break;
				}
	}

	return;

error_delete_buchung:
	if (!m_ppb)
	{
		delete *p;
		*p = NULL;
	}
}

void BuchenDlg::InitDlg(BOOL bBelasseEinigeFelder)
{
	char buf_window_text[300];
	CString csWiederherstellen;

/*	char [500], IniFileName[500];
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*IniFileName) GetPrivateProfileString("Betriebe", "Betrieb00", "", szPathName, sizeof(szPathName), IniFileName);
*/

	// Combo-Box löschen und neu aufbauen
	if (bBelasseEinigeFelder) GetDlgItemText(IDC_BESCHREIBUNG, csWiederherstellen);
	((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->ResetContent();
	int i;
	for (i = 0; i < 100; i++)
	{
		char buffer[1000];
		if (m_pParent->einstellungen1->Buchungsposten[i].Beschreibung != "" 
			&& m_ausgaben == m_pParent->einstellungen1->Buchungsposten[i].Ausgaben)
		{
			sprintf(buffer, "%02d %s",
				i, (LPCTSTR)m_pParent->einstellungen1->Buchungsposten[i].Beschreibung);
			int n = ((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->AddString(buffer);
			((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->SetItemData(n, i);
		}
	}
	if (bBelasseEinigeFelder) SetDlgItemText(IDC_BESCHREIBUNG, csWiederherstellen);

	if (m_ausgaben)
	{
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE_STATIC2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ABSCHREIBUNGNUMMER)->ShowWindow(SW_SHOW);
		InitRestwert();
		GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT)->ShowWindow(SW_SHOW);
		if (m_ppb && (*m_ppb)->AbschreibungNr > 1)
			GetDlgItem(IDC_ABGANG_BUCHEN)->ShowWindow(SW_SHOW);
		else
			GetDlgItem(IDC_ABGANG_BUCHEN)->ShowWindow(SW_HIDE);

		((CButton *)GetDlgItem(IDC_EINNAHMEN))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_AUSGABEN))->SetCheck(TRUE);

		if (m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben && !m_ppb)
		{
			char buffer[100];
			sprintf(buffer, "A%04d", m_pDoc->nLaufendeBuchungsnummerFuerAusgaben);
			SetDlgItemText(IDC_BELEGNUMMER, buffer);
		}
		else
			SetDlgItemText(IDC_BELEGNUMMER, "");

		if (!bBelasseEinigeFelder) 
			SetDlgItemText(IDC_BESCHREIBUNG, "");

		UpdateCombo("A");

		GetDlgItem(IDC_SPLIT)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGNUMMER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGJAHRE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABGANG_BUCHEN)->ShowWindow(SW_HIDE);

		((CButton *)GetDlgItem(IDC_AUSGABEN))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_EINNAHMEN))->SetCheck(TRUE);

		if (m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen && !m_ppb)
		{
			char buffer[100];
			sprintf(buffer, "E%04d", m_pDoc->nLaufendeBuchungsnummerFuerEinnahmen);
			SetDlgItemText(IDC_BELEGNUMMER, buffer);
		}
		else
			SetDlgItemText(IDC_BELEGNUMMER, "");

		if (!bBelasseEinigeFelder) 
			SetDlgItemText(IDC_BESCHREIBUNG, "");

		UpdateCombo("E");

		GetDlgItem(IDC_SPLIT)->EnableWindow(FALSE);
	}

	m_nGewaehlterSplit = -1;
	SetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, "");	
	if (m_ppb && *m_ppb)
	{
		char *cp = GetErweiterungKey((*m_ppb)->Erweiterung, "EasyCash", "GewaehlterSplit");
		if (*cp)
		{
			m_nGewaehlterSplit = atoi(cp);
			if (m_nGewaehlterSplit >= 0)
			{
				CString csSplitText;
				csSplitText.Format("%s%% auf %s (%s%% UST)", (LPCSTR)*m_pParent->einstellungen5->m_psatz[m_nGewaehlterSplit], (LPCSTR)*(m_pParent->einstellungen5->m_part[m_nGewaehlterSplit]), (LPCSTR)*m_pParent->einstellungen5->m_pustsatz[m_nGewaehlterSplit]);
				SetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, csSplitText);	
			}
			else
				SetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, "");	
		}
	}	

	if (m_ppb && *m_ppb)
	{
		if (m_bKopieren)
			strcpy(buf_window_text, "Buchungskopie für ");
		else
			strcpy(buf_window_text, "Buchungsänderung für ");
	}
	else
		strcpy(buf_window_text, "Buchen für ");
	strcat(buf_window_text, (LPCTSTR)m_pDoc->GetPathName());
	SetWindowText(buf_window_text);

	if (bBelasseEinigeFelder) GetDlgItemText(IDC_MWST, csWiederherstellen);
	((CComboBox *)GetDlgItem(IDC_MWST))->ResetContent();
	char *mwst;
	for (i = 0; *(mwst = m_pParent->GetVATs(i)); i++)
	{
		if (((CComboBox *)GetDlgItem(IDC_MWST))->FindStringExact(0, mwst) < 0)
			((CComboBox *)GetDlgItem(IDC_MWST))->AddString(mwst);
	}
	if (bBelasseEinigeFelder) SetDlgItemText(IDC_MWST, csWiederherstellen);

	if (m_ppb && *m_ppb)	// ändern
	{
		GetDlgItem(IDC_AUSGABEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EINNAHMEN)->EnableWindow(FALSE);

		CTime now = CTime::GetCurrentTime();
		char buf[100];
		sprintf(buf, "%d", (int)(*m_ppb)->Datum.GetDay());
		SetDlgItemText(IDC_DATUM_TAG, buf);
		sprintf(buf, "%d", (int)(*m_ppb)->Datum.GetMonth());
		SetDlgItemText(IDC_DATUM_MONAT, buf);
		sprintf(buf, "%d", (int)(*m_ppb)->Datum.GetYear());
		SetDlgItemText(IDC_DATUM_JAHR, buf);
		SetDlgItemText(IDC_BESCHREIBUNG, (*m_ppb)->Beschreibung);
		if (!m_bKopieren || !m_bNeueBelegnummer)
			SetDlgItemText(IDC_BELEGNUMMER, (*m_ppb)->Belegnummer);
		else
		{
			if (m_ausgaben)
			{
				if (m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben)
				{
					char buffer[100];
					sprintf(buffer, "A%04d", m_pDoc->nLaufendeBuchungsnummerFuerAusgaben);
					SetDlgItemText(IDC_BELEGNUMMER, buffer);
				}
			}
			else
			{
				if (m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen)
				{
					char buffer[100];
					sprintf(buffer, "E%04d", m_pDoc->nLaufendeBuchungsnummerFuerEinnahmen);
					SetDlgItemText(IDC_BELEGNUMMER, buffer);
				}
			}
		}
		int_to_currency((*m_ppb)->Betrag, 4, buf);
		SetDlgItemText(IDC_BETRAG, buf);
		(*m_ppb)->GetMWSt(buf);
		SetDlgItemText(IDC_MWST, buf);
		if ((*m_ppb)->AbschreibungNr > (*m_ppb)->AbschreibungJahre + 1) 
			(*m_ppb)->AbschreibungNr = (*m_ppb)->AbschreibungJahre + 1;
		sprintf(buf, "%d", (int)(*m_ppb)->AbschreibungNr);
		SetDlgItemText(IDC_ABSCHREIBUNGNUMMER, buf);
		//((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNGNUMMER))->ResetContent();
		//for (i = 1; i <= (*m_ppb)->AbschreibungJahre + 1; i++)
		//{
		//	sprintf(buf, "%d", i);
		//	((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNGNUMMER))->AddString(buf);
		//}
		sprintf(buf, "%d", (int)(*m_ppb)->AbschreibungJahre);
		SetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buf);	
		((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT))->SetCurSel((*m_ppb)->AbschreibungGenauigkeit);
		if (m_ppb && (*m_ppb)->AbschreibungNr > 1)
			int_to_currency((*m_ppb)->AbschreibungRestwert, 4, buf);
		else
			int_to_currency((*m_ppb)->GetNetto(), 4, buf);
		SetDlgItemText(IDC_ABSCHREIBUNGRESTWERT, buf);		
		int m;
		if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, (*m_ppb)->Konto)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
		else
		{
			if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, "   " + (*m_ppb)->Konto)) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
		}

		for (i = 0; i < ((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetItemCount(); i++)
		{
			if (((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetItemText(i, 0) == (*m_ppb)->Betrieb)
			{
				((CListCtrl *)GetDlgItem(IDC_BETRIEB))->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				((CListCtrl *)GetDlgItem(IDC_BETRIEB))->EnsureVisible(((CListCtrl *)GetDlgItem(IDC_BETRIEB))->GetItemCount() - 1, TRUE); // ganz nach unten scrollen
				((CListCtrl *)GetDlgItem(IDC_BETRIEB))->EnsureVisible(i, TRUE); // gerade genug nach oben scrollen, um das Element anzuzeigen
			}
			else
				((CListCtrl *)GetDlgItem(IDC_BETRIEB))->SetItemState(i, 0, LVIS_SELECTED);
		}
		for (i = 0; i < ((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetItemCount(); i++)
		{
			if (((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetItemText(i, 0) == (*m_ppb)->Bestandskonto)
			{
				((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->EnsureVisible(((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->GetItemCount() - 1, TRUE); // ganz nach unten scrollen
				((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->EnsureVisible(i, TRUE); // gerade genug nach oben scrollen, um das Element anzuzeigen
			}
			else
				((CListCtrl *)GetDlgItem(IDC_BESTANDSKONTO))->SetItemState(i, 0, LVIS_SELECTED);
		}

		if (m_bKopieren) m_ppb = NULL;	 
	}
	else	// neu
	{
		GetDlgItem(IDC_AUSGABEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_EINNAHMEN)->EnableWindow(TRUE);

		CString dd, mm, yyyy;
		GetDlgItemText(IDC_DATUM_TAG, mm);
		GetDlgItemText(IDC_DATUM_MONAT, dd);
		GetDlgItemText(IDC_DATUM_JAHR, yyyy);
		if ((!m_pParent->einstellungen1->m_BuchungsdatumBelassen | !dd.GetLength() | !mm.GetLength() | !yyyy.GetLength()) && !bBelasseEinigeFelder)
		{
			CTime now = CTime::GetCurrentTime();
			char buf[100];
			if (now.GetYear() == m_pDoc->nJahr)
			{
				sprintf(buf, "%d", (int)now.GetDay());
				SetDlgItemText(IDC_DATUM_TAG, buf);
				sprintf(buf, "%d", (int)now.GetMonth());
				SetDlgItemText(IDC_DATUM_MONAT, buf);
				sprintf(buf, "%d", (int)now.GetYear());
				SetDlgItemText(IDC_DATUM_JAHR, buf);
			}
			else
			{
				SetDlgItemText(IDC_DATUM_TAG, "31");
				SetDlgItemText(IDC_DATUM_MONAT, "12");
				sprintf(buf, "%d", (int)m_pDoc->nJahr);
				SetDlgItemText(IDC_DATUM_JAHR, buf);
			}
		}

		if (!bBelasseEinigeFelder) 
		{
			SetDlgItemText(IDC_BETRAG, "");
			if (m_pParent->einstellungen1->m_bMwstFeldAktiviert)
				SetDlgItemText(IDC_MWST, m_pParent->GetDefaultVAT());
			else
				SetDlgItemText(IDC_MWST, "0");
		}
		SetDlgItemText(IDC_ABSCHREIBUNGNUMMER, "1");
		SetDlgItemText(IDC_ABSCHREIBUNGJAHRE, "1");
		((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT))->SetCurSel(m_pParent->einstellungen1->m_AbschreibungGenauigkeit);
		((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
	}
}

void BuchenDlg::InitRestwert()
{
	char buf[1000];

	GetDlgItemText(IDC_ABSCHREIBUNGNUMMER, buf, sizeof(buf)); 
	int n = atoi(buf);
	if (n > 1)
	{
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ABSCHREIBUNGRESTWERT)->ShowWindow(SW_HIDE);
	}
}

void BuchenDlg::OnSelchangeBeschreibung() 
{
	int n = ((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->GetCurSel();
	if (n >= 0)
	{
		char buffer[100];
		int i = (int)((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->GetItemData(n);

		SetTimer(i, 1, NULL);
		char mwstbuffer[30];
		CBetrag dummy_betrag;
		dummy_betrag.MWSt = m_pParent->einstellungen1->Buchungsposten[i].MWSt;
		dummy_betrag.GetMWSt(mwstbuffer);			
		SetDlgItemText(IDC_MWST, mwstbuffer);
		sprintf(buffer, "%d", m_pParent->einstellungen1->Buchungsposten[i].Abschreibungsjahre);
		SetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buffer);
	
		int m;
		if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, m_pParent->einstellungen1->Buchungsposten[i].Rechnungsposten)) != CB_ERR)
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
		else
		{
			if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, "   " + m_pParent->einstellungen1->Buchungsposten[i].Rechnungsposten)) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
		}
	}
}

void BuchenDlg::OnCbnSelchangeEurechnungsposten()
{
	if (m_bUnterkategorien)  // ggf. Auswahl einer Unterkategorie als Konto verhindern
	{
		int n = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel();
		if (n >= 0)
		{
			CString itemText;
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetLBText(n, itemText);
			if (itemText[0] != ' ')  // nur die Auswahl mit Tab-Zeichen eingerückter Einträge als Konto akzeptieren
			{
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
				AfxMessageBox("Die Auswahl einer Unterkategorie als Konto ist nicht möglich. Es können nur die eingerückten Elemente der Liste ausgewählt werden.");
			}
		}
	}
}

void BuchenDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer(nIDEvent);

	if (nIDEvent >= 0 && nIDEvent < 100)
	{
		CString cs;
		
		/*
		if (  (m_ausgaben && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben
			|| !m_ausgaben && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen) 
			&& !m_ppb)
			cs.Format("%s #%2.2d/%d", 
				(LPCSTR)m_pParent->einstellungen1->Buchungsposten[nIDEvent].Beschreibung,
				(int)m_pDoc->nJahr%100, 
				(m_ausgaben? m_pDoc->nLaufendeBuchungsnummerFuerAusgaben : m_pDoc->nLaufendeBuchungsnummerFuerEinnahmen));
		else
			*/
			cs = m_pParent->einstellungen1->Buchungsposten[nIDEvent].Beschreibung;
		
		m_UpdateBeschreibung = FALSE;
		SetDlgItemText(IDC_BESCHREIBUNG, (LPCSTR)cs);		
		int n = m_pParent->einstellungen1->Buchungsposten[nIDEvent].Beschreibung.GetLength();
		((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->SetEditSel(n, n);	
		m_UpdateBeschreibung = TRUE;
	}
	else if (nIDEvent == 101)
	{
		int x = 0, y = 0
#if (_MSC_VER >= 1600)
			, cx = -1, cy = -1
#endif
			;
		RECT rWnd;
		RECT rScreen;

		GetWindowRect(&rWnd);
		GetDesktopWindow()->GetWindowRect(&rScreen);

		char inifile[1000], betriebe[2], bestandskonten[2];
		GetIniFileName(inifile, sizeof(inifile));
		GetPrivateProfileString("Betriebe", "Betrieb00Name", "", betriebe, sizeof(betriebe), inifile);
		GetPrivateProfileString("Bestandskonten", "Bestandskonto00Name", "", bestandskonten, sizeof(bestandskonten), inifile);
		
		x = theApp.GetProfileInt("Fenster", "BuchenPosX", 150);
		y = theApp.GetProfileInt("Fenster", "BuchenPosY", 100);
#if (_MSC_VER >= 1600)
		if (*betriebe && *bestandskonten)  // wenn beide aktiv: dynamisches layout aktivieren (TODO: ListCtrls nach unten und horizontal anordenen, damit dynamisches layout auch ermöglicht wird, wenn ListCtrls ausgeblendet sein sollen)
		{
			cx = theApp.GetProfileInt("Fenster", "BuchenSizeX", -1);
			cy = theApp.GetProfileInt("Fenster", "BuchenSizeY", -1);
		}
#endif
		if (x > rScreen.right-(rWnd.right-rWnd.left)) x = rScreen.right-(rWnd.right-rWnd.left);
		if (x < 0) x = 0;
		if (y > rScreen.bottom-(rWnd.bottom-rWnd.top)) y = rScreen.bottom-(rWnd.bottom-rWnd.top);
		if (y < 0) y = 0;

		if (!*betriebe || !*bestandskonten)
		{
			CRect r, r2;
			GetDlgItem(IDC_BETRIEB_STATIC)->GetWindowRect(&r);
			GetDlgItem(IDC_BESTANDSKONTO_STATIC)->GetWindowRect(&r2);
			int width_betriebe_bestandskonten = r2.left-r.left;

			if (!*betriebe)
			{
				GetDlgItem(IDC_BETRIEB_STATIC)->GetWindowRect(&r);		// Bestandskonto Static verschieben
				GetDlgItem(IDC_BETRIEB_STATIC)->ShowWindow(SW_HIDE);
				ScreenToClient(r);
				GetDlgItem(IDC_BESTANDSKONTO_STATIC)->MoveWindow(r);
				
				GetDlgItem(IDC_BETRIEB)->GetWindowRect(&r);		// Bestandskonto ListCtrl verschieben
				GetDlgItem(IDC_BETRIEB)->ShowWindow(SW_HIDE);
				ScreenToClient(r);
				GetDlgItem(IDC_BESTANDSKONTO)->MoveWindow(r);
				
				GetDlgItem(IDC_ALT1)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ALT1)->GetWindowRect(&r);		// Knopf verschieben
				ScreenToClient(r);
				GetDlgItem(IDC_ALT4)->MoveWindow(r);

				GetDlgItem(IDC_ALT2)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ALT2)->GetWindowRect(&r);		// Knopf verschieben
				ScreenToClient(r);
				GetDlgItem(IDC_ALT5)->MoveWindow(r);

				GetDlgItem(IDC_ALT3)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ALT3)->GetWindowRect(&r);		// Knopf verschieben
				ScreenToClient(r);
				GetDlgItem(IDC_ALT6)->MoveWindow(r);

			}
			SetWindowPos(NULL, x, y, rWnd.right-rWnd.left-(*betriebe ? 0 : width_betriebe_bestandskonten)-(*bestandskonten ? 0 : width_betriebe_bestandskonten), rWnd.bottom-rWnd.top, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
		}
#if (_MSC_VER >= 1600)
		else if (*betriebe && *bestandskonten && cx > 0 && cy > 0)  // wenn beide aktiv: dynamisches layout aktivieren
			SetWindowPos(NULL, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
#endif
		else
			SetWindowPos(NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
		
		KillTimer(nIDEvent);
	}
	else if (nIDEvent == 102)
	{
		char buf[1000];

		GetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buf, sizeof(buf));
		int n = atoi(buf);

		// wenn Konto noch nicht gewählt, ein gängiges AfA-Konto voreinstellen
		if (n > 1 && ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->GetCurSel() == -1)
		{
			int m;
			if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindString(0, "Abschreibungen")) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindString(0, "AfA")) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindString(0, "AfA auf bewegliche Wirtschaftsgüter")) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindString(0, "Abschreibung auf das Anlagevermögen (Afa, GWG)")) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);			
		}			

		InitRestwert();

		// Restwert neu ermitteln
		CBuchung b;
		// Betrag
		GetDlgItemText(IDC_BETRAG, buf, sizeof(buf));
		b.Betrag = currency_to_int(buf);	
		// Datum
		GetDlgItemText(IDC_DATUM_MONAT, buf, sizeof(buf)); 
		int m = atoi(buf);
		if (m < 1 || m > 12) m = 1;
		b.Datum = CTime(2000, m, 1, 0, 0, 0);
		// MWSt
		GetDlgItemText(IDC_MWST, buf, sizeof(buf)); 
		if (!strlen(buf) || !b.SetMWSt(buf)) b.SetMWSt("0");
		// AbschreibungNr
		b.AbschreibungNr = 1;
		// AbschreibungJahre
		GetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buf, sizeof(buf)); 
		b.AbschreibungJahre = atoi(buf);
		// AbschreibungRestwert 
		GetDlgItemText(IDC_ABSCHREIBUNGRESTWERT, buf, sizeof(buf));
		b.AbschreibungRestwert = b.GetNetto();
		// AbschreibungGenauigkeit
		b.AbschreibungGenauigkeit = ((CComboBox *)GetDlgItem(IDC_ABSCHREIBUNG_GENAUIGKEIT))->GetCurSel();

		GetDlgItemText(IDC_ABSCHREIBUNGNUMMER, buf, sizeof(buf)); 
		n = atoi(buf);
		
		int i;
		for (i = 1; i < n; i++)	// bereits abgeschriebene Jahre durchlaufen
		{
			b.AbschreibungNr = i;
			b.AbschreibungRestwert -= b.GetBuchungsjahrNetto(m_pDoc);
		}

		if (b.AbschreibungRestwert < 0)
			b.AbschreibungRestwert = 0;
		int_to_currency(b.AbschreibungRestwert, 4, buf);
		SetDlgItemText(IDC_ABSCHREIBUNGRESTWERT, buf);

		KillTimer(nIDEvent);
	}

	CDialog::OnTimer(nIDEvent);
}

void BuchenDlg::OnEditchangeBeschreibung() 
{
	char buffer[1000];
	GetDlgItemText(IDC_BESCHREIBUNG, buffer, sizeof(buffer));
	if (isdigit(*buffer) && isdigit(buffer[1]))
	{
		int i = (*buffer - '0') * 10 + (buffer[1] - '0');

		if (m_pParent->einstellungen1->Buchungsposten[i].Ausgaben == m_ausgaben
			&& m_pParent->einstellungen1->Buchungsposten[i].Beschreibung != "")
		{
			SetTimer(i, 1, NULL);
			char mwstbuffer[30];
			CBetrag dummy_betrag;
			dummy_betrag.MWSt = m_pParent->einstellungen1->Buchungsposten[i].MWSt;
			dummy_betrag.GetMWSt(mwstbuffer);			
			SetDlgItemText(IDC_MWST, mwstbuffer);
			sprintf(buffer, "%d", m_pParent->einstellungen1->Buchungsposten[i].Abschreibungsjahre);
			SetDlgItemText(IDC_ABSCHREIBUNGJAHRE, buffer);
	
			int m;
			if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, m_pParent->einstellungen1->Buchungsposten[i].Rechnungsposten)) != CB_ERR)
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
			else
			{
				if ((m = ((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->FindStringExact(0, "   " + m_pParent->einstellungen1->Buchungsposten[i].Rechnungsposten)) != CB_ERR)
					((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(m);
				else
					((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->SetCurSel(-1);
			}
		}
		else
			MessageBeep(0xFFFFFFFF);
	}
}

void BuchenDlg::OnEinnahmen() 
{
	m_ausgaben = FALSE;
	UpdateCombo("E");
	InitDlg(TRUE);
	if (m_pParent->einstellungen1->m_taeglich_buchen)
		GetDlgItem(IDC_BETRAG)->SetFocus();
	else
	{
		GetDlgItem(IDC_DATUM_TAG)->SetFocus();
		((CEdit *)GetDlgItem(IDC_DATUM_TAG))->SetSel(0, -1, FALSE);
	}
}

void BuchenDlg::OnAusgaben() 
{
	m_ausgaben = TRUE;
	UpdateCombo("A");
	InitDlg(TRUE);
	if (m_pParent->einstellungen1->m_taeglich_buchen)
		GetDlgItem(IDC_BETRAG)->SetFocus();
	else
	{
		GetDlgItem(IDC_DATUM_TAG)->SetFocus();
		((CEdit *)GetDlgItem(IDC_DATUM_TAG))->SetSel(0, -1, FALSE);
	}}

void BuchenDlg::OnClose() 
{
//	m_pParent->buchenDlg = NULL;
	
	CDialog::OnClose();
}

void BuchenDlg::OnCancel() 
{
//	m_pParent->buchenDlg = NULL;
	
//	GetDlgItem(IDC_KONVERTIEREN)->EnableWindow(TRUE);
	CDialog::OnCancel();
}

void BuchenDlg::OnKillfocusDatumJahr() 
{
	int j;
	char buf[100];

	GetDlgItemText(IDC_DATUM_JAHR, buf, sizeof(buf)); j = atoi(buf);
	if (j < 100 && j > 37)
	{
		j += 1900;
	}
	else if (j <= 37 && j >= 0)
	{
		j += 2000;
	}
	sprintf(buf, "%d", j);
	SetDlgItemText(IDC_DATUM_JAHR, buf);
}

void BuchenDlg::OnJahrEnabled() 
{
	GetDlgItem(IDC_DATUM_JAHR)->EnableWindow(((CButton *)GetDlgItem(IDC_JAHR_ENABLED))->GetCheck());
	m_pParent->einstellungen1->m_bJahresfeldAktiviert = ((CButton *)GetDlgItem(IDC_JAHR_ENABLED))->GetCheck();
	m_pParent->SaveProfile();
}

void BuchenDlg::OnBnClickedMwstEnabled()
{
	GetDlgItem(IDC_MWST)->EnableWindow(((CButton *)GetDlgItem(IDC_MWST_ENABLED))->GetCheck());
	m_pParent->einstellungen1->m_bMwstFeldAktiviert = ((CButton *)GetDlgItem(IDC_MWST_ENABLED))->GetCheck();
	m_pParent->SaveProfile();
}

void BuchenDlg::OnSetfocusBeschreibung() 
{
/*	char buffer[1000];
	GetDlgItemText(IDC_BESCHREIBUNG, buffer, sizeof(buffer));
	if (*buffer == ' ' && buffer[1] == '#')
		((CComboBox *)GetDlgItem(IDC_BESCHREIBUNG))->SetEditSel(0, 0);	*/
}

// ea: "E" == Einnahmen, "A" == Ausgaben für Einnahmen-Überschußrechnung
void BuchenDlg::UpdateCombo(CString ea)
{
	int i;
	m_bUnterkategorien = FALSE;

	((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->ResetContent();

	if (ea == "E")
	{
		for (i = 0; i < 100; i++)
			if (!m_pParent->einstellungen1->EinnahmenUnterkategorien[i].IsEmpty())  // gibt es Unterkategorien?
			{
				m_bUnterkategorien = TRUE;
				break;
			}
		if (m_bUnterkategorien && m_pParent->einstellungen1->EinnahmenUnterkategorien[0].IsEmpty())  // erstes konto hat keine Unterkategorie? dann eine Kategorie erdichten...
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString("Einnahmenkonten ohne Unterkategorie");
		for (i = 0; i < 100; i++)
		{
			if (!m_pParent->einstellungen1->EinnahmenUnterkategorien[i].IsEmpty())  // Beginn einer neuen Unterkategorie?
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(m_pParent->einstellungen1->EinnahmenUnterkategorien[i]);
			if (!m_pParent->einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString((m_bUnterkategorien ? "   " : "") + m_pParent->einstellungen1->EinnahmenRechnungsposten[i]);
		}
	}
	else
	{
		for (i = 0; i < 100; i++)
			if (!m_pParent->einstellungen1->AusgabenUnterkategorien[i].IsEmpty())  // gibt es Unterkategorien?
			{
				m_bUnterkategorien = TRUE;
				break;
			}
		if (m_bUnterkategorien && m_pParent->einstellungen1->AusgabenUnterkategorien[0].IsEmpty())  // erstes konto hat keine Unterkategorie? dann eine Kategorie erdichten...
			((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString("Ausgabenkonten ohne Unterkategorie");
		for (i = 0; i < 100; i++)
		{
			if (!m_pParent->einstellungen1->AusgabenUnterkategorien[i].IsEmpty())  // Beginn einer neuen Unterkategorie?
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString(m_pParent->einstellungen1->AusgabenUnterkategorien[i]);
			if (!m_pParent->einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
				((CComboBox *)GetDlgItem(IDC_EURECHNUNGSPOSTEN))->AddString((m_bUnterkategorien ? "   " : "") + m_pParent->einstellungen1->AusgabenRechnungsposten[i]);
		}
	}
}

void BuchenDlg::OnSelchangeAbschreibungjahre() 
{
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnCbnEditchangeAbschreibungjahre()
{	
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnCbnSelchangeAbschreibungnummer()
{
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnCbnEditchangeAbschreibungnummer()
{
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnCbnSelchangeMwst()
{
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnCbnEditchangeMwst()
{		
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	RECT r;
	GetWindowRect(&r);
		
	theApp.WriteProfileInt("Fenster", "BuchenPosX", r.left);
	theApp.WriteProfileInt("Fenster", "BuchenPosY", r.top);
#if (_MSC_VER >= 1600)
	theApp.WriteProfileInt("Fenster", "BuchenSizeX", r.right - r.left);
	theApp.WriteProfileInt("Fenster", "BuchenSizeY", r.bottom - r.top);
#endif
}

void BuchenDlg::OnChangeDatumTag() 
{
	int m, n;
	((CEdit *)GetDlgItem(IDC_DATUM_TAG))->GetSel(m, n);
	if (m == 2 && n == 2) GetDlgItem(IDC_DATUM_MONAT)->SetFocus();
}

void BuchenDlg::OnChangeDatumMonat() 
{
	int m, n;
	((CEdit *)GetDlgItem(IDC_DATUM_MONAT))->GetSel(m, n);
	if (m == 2 && n == 2) 
	{
		if (GetDlgItem(IDC_DATUM_JAHR)->IsWindowEnabled())
			GetDlgItem(IDC_DATUM_JAHR)->SetFocus();
		else
			GetDlgItem(IDC_BETRAG)->SetFocus();
	}	
	
	SetTimer(102, 1, NULL);
}

void BuchenDlg::OnChangeDatumJahr() 
{
	int m, n;
	((CEdit *)GetDlgItem(IDC_DATUM_JAHR))->GetSel(m, n);
	if (m == 4 && n == 4) GetDlgItem(IDC_BETRAG)->SetFocus();
}

void BuchenDlg::OnSetfocusDatumTag() 
{
	((CEdit *)GetDlgItem(IDC_DATUM_TAG))->SetSel(0, -1);
}

void BuchenDlg::OnSetfocusDatumMonat() 
{
	((CEdit *)GetDlgItem(IDC_DATUM_MONAT))->SetSel(0, -1);
}

void BuchenDlg::OnSetfocusDatumJahr() 
{
	((CEdit *)GetDlgItem(IDC_DATUM_JAHR))->SetSel(0, -1);
}

void BuchenDlg::OnChangeBetrag() 
{
	char buffer[50];
	int m, n;

	SetTimer(102, 1, NULL);

	GetDlgItemText(IDC_BETRAG, buffer, sizeof(buffer));
//	GetDlgItem(IDC_KONVERTIEREN)->EnableWindow(TRUE);
	if (m_ausgaben) GetDlgItem(IDC_SPLIT)->EnableWindow(TRUE);
	if (strlen(buffer) < 3) return;
	((CEdit *)GetDlgItem(IDC_BETRAG))->GetSel(m, n);
	if (m != n || n != (int)strlen(buffer)) return;	
	if (isdigit(buffer[strlen(buffer)-1]) 
		&& isdigit(buffer[strlen(buffer)-2])
		&& buffer[strlen(buffer)-3] == ',')
			GetDlgItem(IDC_BESCHREIBUNG)->SetFocus();	
}

void BuchenDlg::OnChangeBelegnummer() 
{
	char buffer[50];
	GetDlgItemText(IDC_BELEGNUMMER, buffer, sizeof(buffer));

	if (strlen(buffer) == 1)
	{
		if (toupper(*buffer) == 'B' && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerBank)
		{
			sprintf(buffer, "B%04d", m_pDoc->nLaufendeBuchungsnummerFuerBank);
			SetDlgItemText(IDC_BELEGNUMMER, buffer);
			GetDlgItem(IDC_EURECHNUNGSPOSTEN)->SetFocus();
		}
		else if (toupper(*buffer) == 'K' && m_pParent->einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerKasse)
		{
			sprintf(buffer, "K%04d", m_pDoc->nLaufendeBuchungsnummerFuerKasse);
			SetDlgItemText(IDC_BELEGNUMMER, buffer);
			GetDlgItem(IDC_EURECHNUNGSPOSTEN)->SetFocus();
		}
	}

}

void BuchenDlg::OnKonvertieren() 
{
	CBetrag b;
	char buf[1000];

	GetDlgItemText(IDC_BETRAG, buf, sizeof(buf));
	b.Betrag = currency_to_int(buf);

	if (!strlen(buf)) return;

	if (m_pDoc->csWaehrung == "EUR")
		b.ConvertToEuro(m_csZielwaehrung);
	else
		b.ConvertFromEuro(m_csZielwaehrung);

	int_to_currency(b.Betrag, 12, buf);
	CString cs;
	cs = buf;
	cs.TrimLeft();
	SetDlgItemText(IDC_BETRAG, cs);

//	GetDlgItem(IDC_KONVERTIEREN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BESCHREIBUNG)->SetFocus();
}

void BuchenDlg::OnSplit() 
{
	CRect r;

	// erstmal Menü löschen
	while (PopUp.GetMenuItemCount() > 0)
		PopUp.DeleteMenu(0, MF_BYPOSITION);

	int i;
	for (i = 0; i < m_pParent->einstellungen5->m_privat_split_size; i++)
	{
		if (!(m_pParent->einstellungen5->m_part[i])->IsEmpty()
			|| *(m_pParent->einstellungen5->m_psatz[i]) != 100)
		{
			char buffer[1000];
			sprintf(buffer, "%s%%/%s%% %s", (LPCSTR)*m_pParent->einstellungen5->m_psatz[i], (LPCSTR)*m_pParent->einstellungen5->m_pustsatz[i], (LPCSTR)*m_pParent->einstellungen5->m_part[i]);
			char buffer_ampercent[2000], *cp1, *cp2;
			cp1 = buffer; cp2 = buffer_ampercent;
			do {
				if (*cp1 == '&') *cp2++ = '&';
			}
			while (*cp2++ = *cp1++);
			PopUp.AppendMenu(MF_STRING, POPUP_SPLIT+i, buffer_ampercent);
		}
	}
	PopUp.AppendMenu(MF_STRING, POPUP_SPLIT+i, "<kein Privat-Split>");
	
	GetDlgItem(IDC_SPLIT)->GetWindowRect(&r);
	PopUp.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, r.left, r.bottom, m_pParent);
}

void BuchenDlg::PrivatSplit(int n)
{
	if (n >= m_pParent->einstellungen5->m_privat_split_size) 
	{
		m_nGewaehlterSplit = -1;
		SetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, "");
	}
	else
	{
		CString csSplitText;
		csSplitText.Format("%s%% auf %s (%s%% UST)", (LPCSTR)*m_pParent->einstellungen5->m_psatz[n], (LPCSTR)*(m_pParent->einstellungen5->m_part[n]), (LPCSTR)*m_pParent->einstellungen5->m_pustsatz[n]);
		SetDlgItemText(IDC_EINNAHMEN_GEGENKONTO, csSplitText);
		m_nGewaehlterSplit = n;
	}
/*	alter Split-Code:
	CBetrag b;
	char buf[1000];

	GetDlgItemText(IDC_BETRAG, buf, sizeof(buf));
	b.Betrag = currency_to_int(buf);
	if (!strlen(buf)) return;

	b.Betrag *= *m_pParent->einstellungen5->m_psatz[n];
	b.Betrag /= 100;

	int_to_currency(b.Betrag, 12, buf);
	CString cs;
	cs = buf;
	cs.TrimLeft();
	SetDlgItemText(IDC_BETRAG, cs);

	GetDlgItem(IDC_SPLIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BESCHREIBUNG)->SetFocus();
*/
}

void BuchenDlg::OnRechner() 
{
	char buf[1000];
	CRechnDlg dlg;
	GetDlgItemText(IDC_BETRAG, buf, sizeof(buf));
	dlg.m_betrag = (double)currency_to_int(buf) / 100.0;
	GetDlgItemText(IDC_MWST, buf, sizeof(buf)); 
	CBetrag b;
	if (!strlen(buf) || !b.SetMWSt(buf))
		dlg.m_mwst = 0.0;
	else
		dlg.m_mwst = (double)b.MWSt / 1000.0;
	if (dlg.DoModal() == IDOK)
	{
		int_to_currency((int)((dlg.m_betrag_neu + 0.005) * 100), 4, buf);
		SetDlgItemText(IDC_BETRAG, buf);
	}

	GetDlgItem(IDC_BESCHREIBUNG)->SetFocus();
}

void BuchenDlg::OnAlt(int nCtrl, int nItem)
{
	int i;
	for (i = 0; i < ((CListCtrl *)GetDlgItem(nCtrl))->GetItemCount(); i++)
	{
		if (i == nItem)
			((CListCtrl *)GetDlgItem(nCtrl))->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
		else
			((CListCtrl *)GetDlgItem(nCtrl))->SetItemState(i, 0, LVIS_SELECTED);
	}	
}

void BuchenDlg::OnAlt1() 
{
	OnAlt(IDC_BETRIEB, 0);
}

void BuchenDlg::OnAlt2() 
{
	OnAlt(IDC_BETRIEB, 1);
}

void BuchenDlg::OnAlt3() 
{
	OnAlt(IDC_BETRIEB, 2);
}

void BuchenDlg::OnAlt4() 
{
	OnAlt(IDC_BESTANDSKONTO, 0);
}

void BuchenDlg::OnAlt5() 
{
	OnAlt(IDC_BESTANDSKONTO, 1);
}

void BuchenDlg::OnAlt6() 
{
	OnAlt(IDC_BESTANDSKONTO, 2);
}

void BuchenDlg::OnBnClickedAbgangBuchen()
{
	m_pParent->AfAAbgang(m_ppb);

	CDialog::OnOK();

	int i;
	for (i = 0; i < MAX_BUCHUNGEN; i++)
		if (m_pParent->ppPosBuchungsliste[i] && *(m_pParent->ppPosBuchungsliste[i]) == *m_ppb)
		{
			m_pParent->nSelected = i;
			break;
		}
	m_pParent->RedrawWindow();
	m_pDoc->UpdateAllViews(NULL);
}
