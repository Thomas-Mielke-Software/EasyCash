// Formularfeld.cpp : implementation file
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
#include "Formularfeld.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFormularfeld dialog


CFormularfeld::CFormularfeld(CEasyCashView* pParent)
	: CDialog(CFormularfeld::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFormularfeld)
	m_langform = _T("");
	m_name = _T("");
	m_ausrichtung = -1;
	m_seite = 0;
	m_horizontal = 0;
	m_vertikal = 0;
	m_id = 0;
	m_anteil = -1;
	m_nachkommaanteil = -1;
	m_nNullwertdarstellung = -1;
	m_veraltet = 0;
	//}}AFX_DATA_INIT

	m_pParent = pParent;
	m_bKeineFeldUeberschreibenMeldung = FALSE;
}


void CFormularfeld::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormularfeld)
	DDX_Control(pDX, IDC_TYP, m_typ);
	DDX_Text(pDX, IDC_LANGFORM, m_langform);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Radio(pDX, IDC_AUSRICHTUNG, m_ausrichtung);
	DDX_Text(pDX, IDC_SEITE, m_seite);
	DDV_MinMaxInt(pDX, m_seite, 1, 10000);
	DDX_Text(pDX, IDC_HORIZONTAL, m_horizontal);
	DDX_Text(pDX, IDC_VERTIKAL, m_vertikal);
	DDX_Text(pDX, IDC_ID, m_id);
	DDV_MinMaxInt(pDX, m_id, 0, 10000);
	DDX_Radio(pDX, IDC_ANTEIL, m_anteil);
	DDX_Radio(pDX, IDC_NACHKOMMAANTEIL, m_nachkommaanteil);
	DDX_Radio(pDX, IDC_NULLWERTE, m_nNullwertdarstellung);
	DDX_Radio(pDX, IDC_VERALTET, m_veraltet);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFormularfeld, CDialog)
	//{{AFX_MSG_MAP(CFormularfeld)
	ON_BN_CLICKED(IDOK, OnSpeichern)
	ON_NOTIFY(UDN_DELTAPOS, IDC_HORIZONTAL_SPIN, OnDeltaposHorizontalSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_VERTIKAL_SPIN, OnDeltaposVertikalSpin)
	ON_BN_CLICKED(IDC_AUSRICHTUNG, OnAusrichtung)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SEITE_SPIN, OnDeltaposSeiteSpin)
	ON_EN_CHANGE(IDC_SEITE, OnChangeSeite)
	ON_EN_CHANGE(IDC_VERTIKAL, OnChangeVertikal)
	ON_EN_CHANGE(IDC_HORIZONTAL, OnChangeHorizontal)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_AUSRICHTUNG2, OnAusrichtung)
	ON_CBN_SELCHANGE(IDC_TYP, OnSelchangeTyp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormularfeld message handlers

BOOL CFormularfeld::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_seite < 1) m_seite = 1;
	SetTimer(1, 1, NULL);

	return TRUE; 
}

void CFormularfeld::OnTimer(UINT nIDEvent) 
{
	KillTimer(1);
	RECT r, rScreen;
	GetDesktopWindow()->GetWindowRect(&rScreen);
	GetWindowRect(&r);
	r.left = rScreen.right - (r.right - r.left);
	r.right = rScreen.right;
	r.bottom -= r.top;
	r.top = 0;
	MoveWindow(&r);
	ShowWindow(SW_SHOW);
	UpdateFeldmarke();
	
	CDialog::OnTimer(nIDEvent);
}


void CFormularfeld::OnSpeichern() 
{
	UpdateData();

	if (m_name.IsEmpty())
	{
		AfxMessageBox("Bitte Feldnamen angeben");
		GetDlgItem(IDC_NAME)->SetFocus();
		return;
	}

	if (m_typ.GetCurSel() < 0)
	{
		AfxMessageBox("Bitte Feldtyp auswählen");
		m_typ.SetFocus();
		return;
	}
	CString csTyp;
	m_typ.GetLBText(m_typ.GetCurSel(), csTyp);

	// Formulardefinitionsdatei in xmldoc laden
	XDoc xmldoc;
	xmldoc.LoadFile(m_csFormulardatei);
	LPXNode xml = xmldoc.GetRoot();
	LPXNode felder = NULL;

	if (xml && (felder = xml->Find("felder")))
	{
		LPXNode child;

		// besteht ID bereits?
		int i;
		for(i = 0; i < felder->GetChildCount(); i++)
		{
			child = felder->GetChild(i);
			if (child)
			{
				if (m_id == atoi(child->GetAttrValue("id")))
				{
					if (m_bKeineFeldUeberschreibenMeldung) 
						break;
					else
					{
						if (AfxMessageBox("Ein Feld mit dieser Feld-ID existiert bereits im Formular. Überschreiben?", MB_YESNO) == IDNO)
							return;
						else
							break;
					}
				}
			}
		}
		
		if (i < felder->GetChildCount())	// ändern
		{
			// felder->RemoveChild(child);	// Feld-Node erstmal löschen wenn schon existiert
			CString csFromInt;
			_ultoa((DWORD)m_id, csFromInt.GetBuffer(30), 10);
			child->GetAttr("id")->value = csFromInt;
			child->GetAttr("typ")->value = csTyp;
			csFromInt = _ultoa((DWORD)m_seite, csFromInt.GetBuffer(30), 10);
			child->GetAttr("seite")->value = csFromInt;
			csFromInt = _ultoa((DWORD)m_horizontal, csFromInt.GetBuffer(30), 10);
			child->GetAttr("horizontal")->value = csFromInt;
			csFromInt = _ultoa((DWORD)m_vertikal, csFromInt.GetBuffer(30), 10);
			child->GetAttr("vertikal")->value = csFromInt;
			child->GetAttr("ausrichtung")->value = m_ausrichtung == 0 ? "linksbuendig" : "rechtsbuendig";
			child->GetAttr("anteil")->value = m_anteil == 0 ? "brutto" : (m_anteil == 1 ? "netto" : "mwst");
			child->GetAttr("nachkommaanteil")->value = m_nachkommaanteil == 0 ? "mit" : "ohne";
			if (!child->GetAttr("nullwertdarstellung"))	// später dazugekommen... eigentlich sollte ich die anderen Werte auch testen...
				child->AppendAttr("nullwertdarstellung", m_nNullwertdarstellung == 0 ? "ja" : "nein");
			else
				child->GetAttr("nullwertdarstellung")->value = m_nNullwertdarstellung == 0 ? "ja" : "nein";			
			if (!child->GetAttr("veraltet"))	// später dazugekommen... eigentlich sollte ich die anderen Werte auch testen...
				child->AppendAttr("veraltet", m_veraltet == 1 ? "ja" : "nein");
			else
				child->GetAttr("veraltet")->value = m_veraltet == 1 ? "ja" : "nein";			
			child->GetChild("name")->value = m_name;
			//if (!m_langform.IsEmpty())
			{
				if (!child->GetChild("erweiterung"))
					child->AppendChild("erweiterung", m_langform);
				else
					child->GetChild("erweiterung")->value = m_langform;
			}

			DISP_OPT opt;
			opt.newline = false; // no new line
			if (!xmldoc.SaveFile(m_csFormulardatei, &opt))
			{
				AfxMessageBox("Konnte die Änderungen nicht in der Formulardatei speichern.");
				return;
			}
		}
		else	// neu
		{
			child = felder->AppendChild("feld");	// Feld-Node hinzufügen
			if (child)
			{
				CString csFromInt;
				_ultoa((DWORD)m_id, csFromInt.GetBuffer(30), 10);
				child->AppendAttr("id", csFromInt);
				child->AppendAttr("typ", csTyp);
				csFromInt = _ultoa((DWORD)m_seite, csFromInt.GetBuffer(30), 10);
				child->AppendAttr("seite", csFromInt);
				csFromInt = _ultoa((DWORD)m_horizontal, csFromInt.GetBuffer(30), 10);
				child->AppendAttr("horizontal", csFromInt);
				csFromInt = _ultoa((DWORD)m_vertikal, csFromInt.GetBuffer(30), 10);
				child->AppendAttr("vertikal", csFromInt);
				child->AppendAttr("ausrichtung", m_ausrichtung == 0 ? "linksbuendig" : "rechtsbuendig");
				child->AppendAttr("anteil", m_anteil == 0 ? "brutto" : (m_anteil == 1 ? "netto" : "mwst"));
				child->AppendAttr("nachkommaanteil", m_nachkommaanteil == 0 ? "mit" : "ohne");
				child->AppendChild("name", m_name);
				if (!m_langform.IsEmpty())
					child->AppendChild("erweiterung", m_langform);

				DISP_OPT opt;
				opt.newline = false; // no new line
				if (!xmldoc.SaveFile(m_csFormulardatei, &opt))
				{
					AfxMessageBox("Konnte das Feld nicht in die Formulardatei speichern.");
					return;
				}
			}
		}
	}

	UpdateFeldmarke();	

	CDialog::OnOK();
}

void CFormularfeld::OnCancel() 
{
	UpdateFeldmarke();
	
	CDialog::OnCancel();
}


//--- Feldposition Manipulation ---

void CFormularfeld::OnDeltaposSeiteSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	UpdateData();
	m_seite += pNMUpDown->iDelta;
	if (m_seite < 1) m_seite = 1;
	UpdateFeldmarke();
	
	*pResult = 0;
}

void CFormularfeld::OnDeltaposHorizontalSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	UpdateData();
	m_horizontal -= pNMUpDown->iDelta;
	if (m_horizontal < 0) m_horizontal = 0;
	UpdateFeldmarke();
	
	*pResult = 0;
}

void CFormularfeld::OnDeltaposVertikalSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	UpdateData();
	m_vertikal += pNMUpDown->iDelta;
	if (m_vertikal < 0) m_vertikal = 0;
	UpdateFeldmarke();
	
	*pResult = 0;
}

void CFormularfeld::OnAusrichtung() 
{
	UpdateData();
	UpdateFeldmarke();
}

void CFormularfeld::UpdateFeldmarke()
{
	m_pParent->ptFeldmarke.x = m_horizontal;
	m_pParent->ptFeldmarke.y = m_vertikal + ((m_seite-1) * 1414);
	m_pParent->m_bFeldmarkeRechtsbuendig = m_ausrichtung;
	UpdateData(FALSE);
	m_pParent->GetDocument()->UpdateAllViews(NULL);
	m_pParent->InvalidateRect(NULL, FALSE);
}

void CFormularfeld::OnChangeSeite() 
{
	UpdateData();
	UpdateFeldmarke();
}

void CFormularfeld::OnChangeVertikal() 
{
	UpdateData();
	UpdateFeldmarke();
}

void CFormularfeld::OnChangeHorizontal() 
{
	UpdateData();
	UpdateFeldmarke();
}

void CFormularfeld::OnSelchangeTyp() 
{
	if (m_typ.GetCurSel() >= 2)
	{
		GetDlgItem(IDC_ANTEIL)->EnableWindow(FALSE);
		GetDlgItem(IDC_ANTEIL2)->EnableWindow(FALSE);
		GetDlgItem(IDC_ANTEIL3)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_ANTEIL)->EnableWindow(TRUE);
		GetDlgItem(IDC_ANTEIL2)->EnableWindow(TRUE);
		GetDlgItem(IDC_ANTEIL3)->EnableWindow(TRUE);
	}

	if (m_typ.GetCurSel() >= 3)
	{
		GetDlgItem(IDC_NACHKOMMAANTEIL)->EnableWindow(FALSE);
		GetDlgItem(IDC_NACHKOMMAANTEIL2)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_NACHKOMMAANTEIL)->EnableWindow(TRUE);
		GetDlgItem(IDC_NACHKOMMAANTEIL2)->EnableWindow(TRUE);
	}

	if (m_typ.GetCurSel() == 4)		// Dokumentdaten
	{
		SetDlgItemText(IDC_HINWEIS, "Hiermit kann auf den 'Erweiterung'-Bereich des Dokument-Objekts zugegriffen werden, in etwa so: [Plugin]Schlüssel. Platzhalter: $J (Jahr 4-stellig), $j (Jahr 2-stellig), $v (Voranmeldungszeitraum)");
	}
	else if (m_typ.GetCurSel() == 5)	// Freitext
	{
		SetDlgItemText(IDC_HINWEIS, "Freitext bedeutet, dass der oben im 'Erweiterung'-Feld stehende Text 1:1 in das Formular geschrieben wird. Mögliche Platzhalter: $J (Jahr 4-stellig), $j (Jahr 2-stellig), $v (Voranmeldungszeitraum)");
	}
	else
		SetDlgItemText(IDC_HINWEIS, "");
}
