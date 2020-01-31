// Konvertierung.cpp : implementation file
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
#include "Konvertierung.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKonvertierung dialog


CKonvertierung::CKonvertierung(CWnd* pParent /*=NULL*/)
	: CDialog(CKonvertierung::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKonvertierung)
	m_waehrung_auswahl = -1;
	m_waehrungskuerzel = _T("");
	m_konvertieren = -1;
	//}}AFX_DATA_INIT
}


void CKonvertierung::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKonvertierung)
	DDX_Radio(pDX, IDC_WAEHRUNG_AUSWAHL, m_waehrung_auswahl);
	DDX_Text(pDX, IDC_WAEHRUNGSKUERZEL, m_waehrungskuerzel);
	DDV_MaxChars(pDX, m_waehrungskuerzel, 3);
	DDX_Radio(pDX, IDC_KONVERTIEREN1, m_konvertieren);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKonvertierung, CDialog)
	//{{AFX_MSG_MAP(CKonvertierung)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL2, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL, OnWaehrungAuswahl2)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL12, OnWaehrungAuswahl3)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL3, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL4, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL5, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL6, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL7, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL8, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL9, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL10, OnWaehrungAuswahl)
	ON_BN_CLICKED(IDC_WAEHRUNG_AUSWAHL11, OnWaehrungAuswahl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKonvertierung message handlers

void CKonvertierung::OnWaehrungAuswahl() 
{
	GetDlgItem(IDC_KONVERTIEREN1)->EnableWindow(TRUE);
	GetDlgItem(IDC_KONVERTIEREN2)->EnableWindow(TRUE);
	GetDlgItem(IDC_WAEHRUNGSKUERZEL)->EnableWindow(FALSE);
}

void CKonvertierung::OnWaehrungAuswahl2() 
{
	GetDlgItem(IDC_KONVERTIEREN1)->EnableWindow(FALSE);
	GetDlgItem(IDC_KONVERTIEREN2)->EnableWindow(FALSE);
	GetDlgItem(IDC_WAEHRUNGSKUERZEL)->EnableWindow(FALSE);
}

void CKonvertierung::OnWaehrungAuswahl3() 
{
	GetDlgItem(IDC_KONVERTIEREN1)->EnableWindow(FALSE);
	GetDlgItem(IDC_KONVERTIEREN2)->EnableWindow(FALSE);
	GetDlgItem(IDC_WAEHRUNGSKUERZEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_WAEHRUNGSKUERZEL)->SetFocus();
}

void CKonvertierung::OnOK() 
{
	m_waehrungskuerzel.MakeUpper();
	
	CDialog::OnOK();
}

BOOL CKonvertierung::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_konvertieren == -1)
	{
		GetDlgItem(IDC_KONVERTIEREN1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_KONVERTIEREN2)->ShowWindow(SW_HIDE);
		CRect r, r2;
		GetWindowRect(&r);
		int n = (r.right-r.left)/2;
		r.right -= n;
		MoveWindow(&r);
		GetDlgItem(IDOK)->GetWindowRect(&r2);
		r2.left -= n + r.left;
		r2.right -= n + r.left;
		r2.top -= r.top;
		r2.bottom -= r.top;
		GetDlgItem(IDOK)->MoveWindow(&r2);
		SetDlgItemText(IDC_TEXT1, "Bitte wählen Sie die Währung, in der die Beträge der Buchungen ausgedrückt werden sollen:");
	}

	return TRUE;  
}
