// Einstellungen4.cpp : implementation file
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
#include "Einstellungen1.h"
#include "Einstellungen4.h"
#include "NeuesKonto.h"
#include ".\ECTIFace\EasyCashDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen4 property page

IMPLEMENT_DYNCREATE(CEinstellungen4, CPropertyPage)

CEinstellungen4::CEinstellungen4() : CPropertyPage(CEinstellungen4::IDD)
{
	//{{AFX_DATA_INIT(CEinstellungen4)
	m_ustvst_gesondert = FALSE;
	m_nach_konten_seitenumbruch = FALSE;
	//}}AFX_DATA_INIT

	m_kontenkategorie = 0;
	m_aktFormular = NULL;

	m_ButtonsNotYetSet = TRUE;

#if (_MSC_VER >= 1600)
	EnableDynamicLayout(TRUE);
#endif
}

CEinstellungen4::~CEinstellungen4()
{
	if (m_aktFormular) { delete m_aktFormular; m_aktFormular = NULL; }
}

void CEinstellungen4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEinstellungen4)
	DDX_Control(pDX, IDC_FORMULAR, m_formular);
	DDX_Control(pDX, IDC_LISTE, m_liste);
	DDX_Check(pDX, IDC_USTVST_GESONDERT, m_ustvst_gesondert);
	DDX_Check(pDX, IDC_NACH_KONTEN_SEITENUMBRUCH, m_nach_konten_seitenumbruch);
	DDX_CBIndex(pDX, IDC_KONTENKATEGORIE, m_kontenkategorie);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_FINANZAMTSZAHLUNGEN1, m_ctrlFinanzamtszahlungen1);
	DDX_Control(pDX, IDC_FINANZAMTSZAHLUNGEN2, m_ctrlFinanzamtszahlungen2);
}


BEGIN_MESSAGE_MAP(CEinstellungen4, CPropertyPage)
	//{{AFX_MSG_MAP(CEinstellungen4)
	ON_BN_CLICKED(IDC_PLUS, OnPlus)
	ON_BN_CLICKED(IDC_MINUS, OnMinus)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_AENDERN, OnAendern)
	ON_CBN_SELCHANGE(IDC_KONTENKATEGORIE, OnSelchangeKontenkategorie)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTE, OnDblclkListe)
	ON_CBN_SELCHANGE(IDC_FORMULAR, OnSelchangeFormular)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_QUICKLIST_GETLISTITEMDATA, OnGetListItem) 
	#ifdef USEXPTHEMES
		//WM_THEMECHANGED = 0x031A (WM_THEMECHANGED may not be avalible)
		ON_MESSAGE(0x031A, OnThemeChanged)	
	#endif
		ON_CBN_SELCHANGE(IDC_FINANZAMTSZAHLUNGEN1, &CEinstellungen4::OnCbnSelchangeFinanzamtszahlungen1)
		ON_CBN_SELCHANGE(IDC_FINANZAMTSZAHLUNGEN2, &CEinstellungen4::OnCbnSelchangeFinanzamtszahlungen2)
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen4 message handlers

BOOL CEinstellungen4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// Button Bitmaps
	if (m_ButtonsNotYetSet)
	{
		PlusBmp.LoadBitmap(IDB_PLUS);
		MinusBmp.LoadBitmap(IDB_MINUS);
		UpBmp.LoadBitmap(IDB_UP);
		DownBmp.LoadBitmap(IDB_DOWN);
		AendernBmp.LoadBitmap(IDB_AENDERN);
		m_ButtonsNotYetSet = FALSE;
	}
	((CButton *)GetDlgItem(IDC_PLUS))->SetBitmap(PlusBmp);
	((CButton *)GetDlgItem(IDC_MINUS))->SetBitmap(MinusBmp);
	((CButton *)GetDlgItem(IDC_UP))->SetBitmap(UpBmp);
	((CButton *)GetDlgItem(IDC_DOWN))->SetBitmap(DownBmp);
	((CButton *)GetDlgItem(IDC_AENDERN))->SetBitmap(AendernBmp);
	
#ifdef USEXPTHEMES
	m_themeManager.Init(m_hWnd);
	m_liste.SetThemeManager(&m_themeManager);
#endif

	// Einnahmen Listbox
	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_SUBITEMIMAGES , LVS_EX_SUBITEMIMAGES );
	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES );
#define LISTE_COLUMNWIDTH 250
	m_liste.InsertColumn(0, "Konten für Betriebseinnahmen", LVCFMT_LEFT, LISTE_COLUMNWIDTH);
	m_liste.InsertColumn(1, "Formularfeld", LVCFMT_LEFT, LISTE_COLUMNWIDTH);
	m_liste.EnableToolTips(TRUE);

	// Formulare in Combo-Box laden
	LadeECFormulare(m_csaFormulare);
	m_formular.ResetContent();
	int i;
	for (i = 0; i < m_csaFormulare.GetSize(); i++)
	{
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[i]);
		LPXNode xml = xmldoc.GetRoot();
		if (xml)
		{
			LPCTSTR attr_name = xml->GetAttrValue("name");
			LPCTSTR attr_anzeigename = xml->GetAttrValue("anzeigename");
TRACE3("%d: %s  --  %s\n\r", i, attr_name, m_csaFormulare[i].GetBuffer(0));
			if (attr_name) 
			{
				// gleicher Name (= Formularkategorie) schon in der Liste? Dann bisherigen mit der aktuellen 
				// Formulardatei ersetzen und Eintrag löschen bzw. nicht in Combobox aufnehmen
// bei neuen Feldern in neuen Versionen von Formularen muss sichergestellt werden, dass auch die Möglichkeit besteht, das neue Formular auszuwählen
//				int nPosGefunden;
//				if ((nPosGefunden = m_formular.FindStringExact(0, attr_name)) != CB_ERR) 
//				{
//					m_csaFormulare[nPosGefunden] = m_csaFormulare[i];
//					m_csaFormulare.RemoveAt(i--);
//					continue;	// an gleicher Stelle fortfahren, da Element gelöscht
//				}
	
				m_formular.AddString((CString)attr_name + " \\ " + (CString)attr_anzeigename);
			}
			else
				m_formular.AddString("kein name-Attribut");
		}
		else
			m_formular.AddString("kein gültiges Formular");
	}
	if (m_formular.GetCount() > 0 ) m_formular.SetCurSel(0);
	OnSelchangeFormular();

	UpdateList();
	
	m_ctrlFinanzamtszahlungen1.ResetContent();
	m_ctrlFinanzamtszahlungen2.ResetContent();
	CString csKonto;
	for (i = 0; i < 100; i++)
	{
		csKonto = m_einstellungen1->EinnahmenRechnungsposten[i];
		if (!csKonto.IsEmpty())
			m_ctrlFinanzamtszahlungen1.AddString(csKonto);
		else 
			break;

		if (csKonto == m_finanzamtserstattungen)
			m_ctrlFinanzamtszahlungen1.SetCurSel(i);
	}
	m_ctrlFinanzamtszahlungen1.AddString("");
	if (m_finanzamtserstattungen.IsEmpty()) m_ctrlFinanzamtszahlungen1.SetCurSel(i);

	for (i = 0; i < 100; i++)
	{
		csKonto = m_einstellungen1->AusgabenRechnungsposten[i];
		if (!csKonto.IsEmpty())
			m_ctrlFinanzamtszahlungen2.AddString(csKonto);
		else
			break;

		if (csKonto == m_finanzamtszahlungen)
			m_ctrlFinanzamtszahlungen2.SetCurSel(i);
	}
	m_ctrlFinanzamtszahlungen2.AddString("");
	if (m_finanzamtszahlungen.IsEmpty()) m_ctrlFinanzamtszahlungen2.SetCurSel(i);

	return TRUE;	              
}

//Get item data
LRESULT CEinstellungen4::OnGetListItem(WPARAM wParam, LPARAM lParam)
{
	//wParam is a handler to the list
	//Make sure message comes from list box
	ASSERT( (HWND)wParam == m_liste.GetSafeHwnd() );

	//lParam is a pointer to the data that is needed for the element
	CQuickList::CListItemData* data = (CQuickList::CListItemData*) lParam;

	//Get which item and subitem that is asked for.
	int item = data->GetItem();
	int subItem = data->GetSubItem();

	data->m_allowEdit = FALSE;
	data->m_textStyle.m_bold = FALSE;
	data->m_textStyle.m_italic = FALSE;

	if(data->GetItem()%2 == 0)
		data->m_colors.m_backColor = RGB( 255, 255, 255);
	else	
		data->m_colors.m_backColor = RGB( 232, 232, 232);

	if (subItem == 0)
	{
		if (m_kontenkategorie == 0)
			data->m_text = m_einstellungen1->EinnahmenRechnungsposten[item];
		else if (m_kontenkategorie == 1)
			data->m_text = m_einstellungen1->AusgabenRechnungsposten[item];
		else
			data->m_text = "";
		
		data->m_tooltip = data->m_text;
	}
	else if (subItem == 1)
	{
		if (m_kontenkategorie == 0)
		{
			if (m_formular.GetCurSel() >= 0) 
			{	
				CString formularname;
				int strippos;
				m_formular.GetLBText(m_formular.GetCurSel(), formularname);
				if ((strippos = formularname.Find(" \\ ", 0)) >= 0) formularname = formularname.Left(strippos);
				
				int nID = atoi(GetErweiterungKey(m_einstellungen1->EinnahmenFeldzuweisungen[item], "ECT", formularname));

				// das Feld mit der ID aus EinnahmenFeldzuweisungen[] in der Formulardatei suchen
				LPXNode xml = m_aktFormular->GetRoot();
				if (xml)
				{
					LPXNode felder = NULL;
					felder = xml->Find("felder");

					if (felder)
					{
						LPXNode child;
						int nCount = felder->GetChildCount();
						int i;
						for (i = 0; i < nCount; i++)
						{
							child = felder->GetChild(i);
							if (atoi(child->GetAttrValue("id")) == nID)
							{
								data->m_text = child->GetChildValue("name");
								data->m_tooltip = child->GetChildValue("erweiterung");
								if (data->m_tooltip == "") data->m_tooltip = data->m_text;

								break;
							}
						}
						if (i == nCount) data->m_tooltip = "Jetzt doppelklicken, um das Konto mit einem Formularfeld zu verknüpfen!";
					}
				}
			}
		}
		else if (m_kontenkategorie == 1)
		{
			if (m_formular.GetCurSel() >= 0) 
			{	
				CString formularname;
				m_formular.GetLBText(m_formular.GetCurSel(), formularname);
				int strippos;
				if ((strippos = formularname.Find(" \\ ", 0)) >= 0) formularname = formularname.Left(strippos);
				
				int nID = atoi(GetErweiterungKey(m_einstellungen1->AusgabenFeldzuweisungen[item], "ECT", formularname));

				// das Feld mit der ID aus AusgabenFeldzuweisungen[] in der Formulardatei suchen
				LPXNode xml = m_aktFormular->GetRoot();

// CString yxc = xml->GetAttrValue("name");

				if (xml)
				{
					LPXNode felder = NULL;
					felder = xml->Find("felder");

					if (felder)
					{
						LPXNode child;
						int nCount = felder->GetChildCount();
						int i;
						for (i = 0; i < nCount; i++)
						{
							child = felder->GetChild(i);
							CString csTemp = child->GetAttrValue("id");
							if (atoi(csTemp) == nID)
							{
								data->m_text = child->GetChildValue("name");
								data->m_tooltip = child->GetChildValue("erweiterung");
								if (data->m_tooltip.IsEmpty()) data->m_tooltip = data->m_text;
								break;
							}
						}
					}
				}
			}			
		}
		else
			data->m_text = "";
	}

	// data->m_tooltip = "";

	return 0;
}

void CEinstellungen4::UpdateList()
{
	UpdateData();
	int n = 0;
	if (m_kontenkategorie == 0)
	{
		int i;
		for (i = 0; i < 100; i++)
		{
			n = i;
			if (m_einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
			{
				break;
			}
		}
	}
	else if (m_kontenkategorie == 1)
	{
		int i;
		for (i = 0; i < 100; i++)
		{
			n = i;
			if (m_einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
			{
				break;
			}
		}
	}

	m_liste.LockWindowUpdate();
	m_liste.SetItemCount(n);
	m_liste.DeleteColumn(0);
	if (m_kontenkategorie == 0)
		m_liste.InsertColumn(0, "Konten für Betriebseinnahmen", LVCFMT_LEFT, LISTE_COLUMNWIDTH);
	else
		m_liste.InsertColumn(0, "Konten für Betriebsausgaben", LVCFMT_LEFT, LISTE_COLUMNWIDTH);
	m_liste.UnlockWindowUpdate();
	m_liste.RedrawItems(
			m_liste.GetTopIndex(),
			m_liste.GetTopIndex()+m_liste.GetCountPerPage());
}


BOOL CEinstellungen4::OnKillActive() 
{	
	return CPropertyPage::OnKillActive();
}


void CEinstellungen4::OnPlus() 
{
	int i;
	if (m_kontenkategorie == 0)
	{
		for (i = 0; i < 100; i++)
			if (m_einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
			{
				CNeuesKonto	dlg;
				dlg.m_konto = "";
				if (m_formular.GetCurSel() >= 0) 
				{	
					m_formular.GetLBText(m_formular.GetCurSel(), dlg.m_formularname);
					int strippos;
					if ((strippos = dlg.m_formularname.Find(" \\ ", 0)) >= 0) dlg.m_formularname = dlg.m_formularname.Left(strippos);
					dlg.m_formulardatei = m_csaFormulare[m_formular.GetCurSel()];
				}
				((CComboBox *)GetDlgItem(IDC_KONTENKATEGORIE))->GetLBText(m_kontenkategorie, dlg.m_kategorie);
				if (dlg.DoModal() == IDOK)
				{
					if (dlg.m_konto.IsEmpty()) return;

					m_einstellungen1->EinnahmenRechnungsposten[i] = dlg.m_konto;
					m_einstellungen1->EinnahmenUnterkategorien[i] = dlg.m_unterkategorie;
					char feldid_str[30];
					_ultoa(dlg.m_feldID, feldid_str, 10);
					SetErweiterungKey(m_einstellungen1->EinnahmenFeldzuweisungen[i], "ECT", dlg.m_formularname, feldid_str);
					UpdateList();
					break;
				}
				else 
					break;
			}
	}
	else if (m_kontenkategorie == 1)
	{
		for (i = 0; i < 100; i++)
			if (m_einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
			{
				CNeuesKonto	dlg;
				dlg.m_konto = "";
				if (m_formular.GetCurSel() >= 0) 
				{	
					m_formular.GetLBText(m_formular.GetCurSel(), dlg.m_formularname);
					int strippos;
					if ((strippos = dlg.m_formularname.Find(" \\ ", 0)) >= 0) dlg.m_formularname = dlg.m_formularname.Left(strippos);
					dlg.m_formulardatei = m_csaFormulare[m_formular.GetCurSel()];
				}
				((CComboBox *)GetDlgItem(IDC_KONTENKATEGORIE))->GetLBText(m_kontenkategorie, dlg.m_kategorie);
				if (dlg.DoModal() == IDOK)
				{
					if (dlg.m_konto.IsEmpty()) return;

					m_einstellungen1->AusgabenRechnungsposten[i] = dlg.m_konto;
					m_einstellungen1->AusgabenUnterkategorien[i] = dlg.m_unterkategorie;
					char feldid_str[30];
					_ultoa(dlg.m_feldID, feldid_str, 10);
					SetErweiterungKey(m_einstellungen1->AusgabenFeldzuweisungen[i], "ECT", dlg.m_formularname, feldid_str);
					UpdateList();
					break;
				}
				else 
					break;
			}
	}

	if (i >= 100) AfxMessageBox("Maximal 100 Konten pro Kategorie möglich.");
}

void CEinstellungen4::OnMinus() 
{
	int n;
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (pos)
		n = m_liste.GetNextSelectedItem(pos);
	else
	{
		AfxMessageBox("Zum Löschen bitte erst einen Eintrag auswählen!");
		return;
	}

	if (AfxMessageBox("Konto wirklich löschen?", MB_YESNO|MB_DEFBUTTON2) == IDYES)
	{
		CString cs;
		if (m_kontenkategorie == 0)
		{
			int i;
			for (i = n; i+1 < 100; i++)
			{
				m_einstellungen1->EinnahmenRechnungsposten[i] = m_einstellungen1->EinnahmenRechnungsposten[i+1];
				m_einstellungen1->EinnahmenFeldzuweisungen[i] = m_einstellungen1->EinnahmenFeldzuweisungen[i+1];
				m_einstellungen1->EinnahmenUnterkategorien[i] = m_einstellungen1->EinnahmenUnterkategorien[i+1];
			}
			m_einstellungen1->EinnahmenRechnungsposten[99] = "";
			m_einstellungen1->EinnahmenFeldzuweisungen[99] = "";
			m_einstellungen1->EinnahmenUnterkategorien[99] = "";
			UpdateList();
		}
		else if (m_kontenkategorie == 1)
		{
			int i;
			for (i = n; i+1 < 100; i++)
			{
				m_einstellungen1->AusgabenRechnungsposten[i] = m_einstellungen1->AusgabenRechnungsposten[i+1];
				m_einstellungen1->AusgabenFeldzuweisungen[i] = m_einstellungen1->AusgabenFeldzuweisungen[i+1];
				m_einstellungen1->AusgabenUnterkategorien[i] = m_einstellungen1->AusgabenUnterkategorien[i+1];
			}
			m_einstellungen1->AusgabenRechnungsposten[99] = "";
			m_einstellungen1->AusgabenFeldzuweisungen[99] = "";
			m_einstellungen1->AusgabenUnterkategorien[99] = "";
			UpdateList();
		}
		m_liste.SetItemState(n, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CEinstellungen4::OnUp() 
{
	int n;
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (pos)
		n = m_liste.GetNextSelectedItem(pos);
	else
	{
		AfxMessageBox("Zum Verschieben bitte erst einen Eintrag auswählen!");
		return;
	}

	if (n <= 0) return;

	CString cs;
	if (m_kontenkategorie == 0)
	{
		cs = m_einstellungen1->EinnahmenRechnungsposten[n];
		m_einstellungen1->EinnahmenRechnungsposten[n] = m_einstellungen1->EinnahmenRechnungsposten[n-1];
		m_einstellungen1->EinnahmenRechnungsposten[n-1] = cs;
		cs = m_einstellungen1->EinnahmenFeldzuweisungen[n];
		m_einstellungen1->EinnahmenFeldzuweisungen[n] = m_einstellungen1->EinnahmenFeldzuweisungen[n-1];
		m_einstellungen1->EinnahmenFeldzuweisungen[n-1] = cs;
		cs = m_einstellungen1->EinnahmenUnterkategorien[n];
		m_einstellungen1->EinnahmenUnterkategorien[n] = m_einstellungen1->EinnahmenUnterkategorien[n-1];
		m_einstellungen1->EinnahmenUnterkategorien[n-1] = cs;
		UpdateList();
	}
	else if (m_kontenkategorie == 1)
	{
		cs = m_einstellungen1->AusgabenRechnungsposten[n];
		m_einstellungen1->AusgabenRechnungsposten[n] = m_einstellungen1->AusgabenRechnungsposten[n-1];
		m_einstellungen1->AusgabenRechnungsposten[n-1] = cs;
		cs = m_einstellungen1->AusgabenFeldzuweisungen[n];
		m_einstellungen1->AusgabenFeldzuweisungen[n] = m_einstellungen1->AusgabenFeldzuweisungen[n-1];
		m_einstellungen1->AusgabenFeldzuweisungen[n-1] = cs;
		cs = m_einstellungen1->AusgabenUnterkategorien[n];
		m_einstellungen1->AusgabenUnterkategorien[n] = m_einstellungen1->AusgabenUnterkategorien[n-1];
		m_einstellungen1->AusgabenUnterkategorien[n-1] = cs;
		UpdateList();
	}
	m_liste.SetItemState(n, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_liste.SetItemState(n-1, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	m_liste.EnsureVisible(n-1, FALSE);
	m_liste.SetFocus();
}

void CEinstellungen4::OnDown() 
{
	int n;
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (pos)
		n = m_liste.GetNextSelectedItem(pos);
	else
	{
		AfxMessageBox("Zum Verschieben bitte erst einen Eintrag auswählen!");
		return;
	}

	if (n > m_liste.GetItemCount()-2) return;

	CString cs;
	if (m_kontenkategorie == 0)
	{
		cs = m_einstellungen1->EinnahmenRechnungsposten[n];
		m_einstellungen1->EinnahmenRechnungsposten[n] = m_einstellungen1->EinnahmenRechnungsposten[n+1];
		m_einstellungen1->EinnahmenRechnungsposten[n+1] = cs;
		cs = m_einstellungen1->EinnahmenFeldzuweisungen[n];
		m_einstellungen1->EinnahmenFeldzuweisungen[n] = m_einstellungen1->EinnahmenFeldzuweisungen[n+1];
		m_einstellungen1->EinnahmenFeldzuweisungen[n+1] = cs;
		cs = m_einstellungen1->EinnahmenUnterkategorien[n];
		m_einstellungen1->EinnahmenUnterkategorien[n] = m_einstellungen1->EinnahmenUnterkategorien[n+1];
		m_einstellungen1->EinnahmenUnterkategorien[n+1] = cs;
		UpdateList();
	}
	else if (m_kontenkategorie == 1)
	{
		cs = m_einstellungen1->AusgabenRechnungsposten[n];
		m_einstellungen1->AusgabenRechnungsposten[n] = m_einstellungen1->AusgabenRechnungsposten[n+1];
		m_einstellungen1->AusgabenRechnungsposten[n+1] = cs;
		cs = m_einstellungen1->AusgabenFeldzuweisungen[n];
		m_einstellungen1->AusgabenFeldzuweisungen[n] = m_einstellungen1->AusgabenFeldzuweisungen[n+1];
		m_einstellungen1->AusgabenFeldzuweisungen[n+1] = cs;
		cs = m_einstellungen1->AusgabenUnterkategorien[n];
		m_einstellungen1->AusgabenUnterkategorien[n] = m_einstellungen1->AusgabenUnterkategorien[n+1];
		m_einstellungen1->AusgabenUnterkategorien[n+1] = cs;
		UpdateList();
	}
	m_liste.SetItemState(n, 0, LVIS_SELECTED | LVIS_FOCUSED);
	m_liste.SetItemState(n+1, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	m_liste.EnsureVisible(n+1, FALSE);
	m_liste.SetFocus();
}

void CEinstellungen4::OnAendern() 
{
	int n;
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (pos)
		n = m_liste.GetNextSelectedItem(pos);
	else
	{
		AfxMessageBox("Zum Ändern des Namens bitte erst einen Eintrag auswählen!");
		return;
	}

	if (n < 0) return;

	CNeuesKonto	dlg;
	if (m_formular.GetCurSel() >= 0) 
	{	
		m_formular.GetLBText(m_formular.GetCurSel(), dlg.m_formularname);
		int strippos;
		if ((strippos = dlg.m_formularname.Find(" \\ ", 0)) >= 0) dlg.m_formularname = dlg.m_formularname.Left(strippos);
		dlg.m_formulardatei = m_csaFormulare[m_formular.GetCurSel()];
	}
TRACE3("%d: %s  --  %s\n\r", n, dlg.m_formularname.GetBuffer(0), m_csaFormulare[m_formular.GetCurSel()]);
	if (m_kontenkategorie == 0)
	{
		dlg.m_konto = m_einstellungen1->EinnahmenRechnungsposten[n];
		dlg.m_unterkategorie = m_einstellungen1->EinnahmenUnterkategorien[n];
		dlg.m_feldID = atoi(GetErweiterungKey(m_einstellungen1->EinnahmenFeldzuweisungen[n], "ECT", dlg.m_formularname));
	}
	else if (m_kontenkategorie == 1)
	{
		dlg.m_konto = m_einstellungen1->AusgabenRechnungsposten[n];
		dlg.m_unterkategorie = m_einstellungen1->AusgabenUnterkategorien[n];
		dlg.m_feldID = atoi(GetErweiterungKey(m_einstellungen1->AusgabenFeldzuweisungen[n], "ECT", dlg.m_formularname));
	}
	((CComboBox *)GetDlgItem(IDC_KONTENKATEGORIE))->GetLBText(m_kontenkategorie, dlg.m_kategorie);
	dlg.m_aendern = TRUE;
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_konto.IsEmpty()) return;

		if (m_kontenkategorie == 0)
		{
			m_einstellungen1->EinnahmenRechnungsposten[n] = dlg.m_konto;
			m_einstellungen1->EinnahmenUnterkategorien[n] = dlg.m_unterkategorie;
			char feldid_str[30];
			_ultoa(dlg.m_feldID, feldid_str, 10);
			SetErweiterungKey(m_einstellungen1->EinnahmenFeldzuweisungen[n], "ECT", dlg.m_formularname, feldid_str);
		}
		else if (m_kontenkategorie == 1)
		{
			m_einstellungen1->AusgabenRechnungsposten[n] = dlg.m_konto;
			m_einstellungen1->AusgabenUnterkategorien[n] = dlg.m_unterkategorie;
			char feldid_str[30];
			_ultoa(dlg.m_feldID, feldid_str, 10);
			SetErweiterungKey(m_einstellungen1->AusgabenFeldzuweisungen[n], "ECT", dlg.m_formularname, feldid_str);
		}
		UpdateList();
	}
}

void CEinstellungen4::OnDestroy() 
{
	CPropertyPage::OnDestroy();
}

void CEinstellungen4::OnSelchangeKontenkategorie() 
{
	UpdateList();	
	SetTimer(1, 1, NULL);
}

void CEinstellungen4::OnDblclkListe(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnAendern();

	*pResult = 0;
}

void CEinstellungen4::OnSelchangeFormular() 
{
	CString csTemp;
	int n = m_formular.GetCurSel();
	if (n >= 0)
	{
		int strippos;
		m_formular.GetLBText(n, csTemp);
		if ((strippos = csTemp.Find(" \\ ", 0)) >= 0) csTemp = csTemp.Left(strippos);
		m_liste.DeleteColumn(1);
		m_liste.InsertColumn(1, csTemp, LVCFMT_LEFT, LISTE_COLUMNWIDTH);
		if (n >= 0) 
		{
			if (m_aktFormular) delete m_aktFormular;
			m_aktFormular = new XDoc;
			m_aktFormular->LoadFile(m_csaFormulare[n]);
			SetTimer(1, 1, NULL);
		}
	}
}

void CEinstellungen4::OnCbnSelchangeFinanzamtszahlungen1()
{
	int nSel = m_ctrlFinanzamtszahlungen1.GetCurSel();
	if (nSel >= 0)
		m_ctrlFinanzamtszahlungen1.GetLBText(nSel, m_finanzamtserstattungen);
}

void CEinstellungen4::OnCbnSelchangeFinanzamtszahlungen2()
{
	int nSel = m_ctrlFinanzamtszahlungen2.GetCurSel();
	if (nSel >= 0)
		m_ctrlFinanzamtszahlungen2.GetLBText(nSel, m_finanzamtszahlungen);
}


void CEinstellungen4::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	SetTimer(1, 1, NULL);
}


void CEinstellungen4::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(1);

	// wieviel Platz haben wir?
	RECT r;
	m_liste.GetWindowRect(&r);
	int nListeBreite = r.right - r.left - 21;

	m_liste.SetColumnWidth(0, nListeBreite / 2);
	m_liste.SetColumnWidth(1, nListeBreite / 2);

	CPropertyPage::OnTimer(nIDEvent);
}
