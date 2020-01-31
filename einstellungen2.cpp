// Einstellungen2.cpp : implementation file
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
#include "Einstellungen2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen2 property page

IMPLEMENT_DYNCREATE(CEinstellungen2, CPropertyPage)

CEinstellungen2::CEinstellungen2() : CPropertyPage(CEinstellungen2::IDD)
{
	//{{AFX_DATA_INIT(CEinstellungen2)
	m_name = _T("");
	m_ort = _T("");
	m_plz = _T("");
	m_strasse = _T("");
	m_unternehmensart1 = _T("");
	m_unternehmensart2 = _T("");
	m_vorname = _T("");
	m_land = -1;
	m_vat1 = _T("");
	m_vat2 = _T("");
	m_vat3 = _T("");
	m_vat4 = _T("");
	//}}AFX_DATA_INIT
}

CEinstellungen2::~CEinstellungen2()
{
}

void CEinstellungen2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEinstellungen2)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 30);
	DDX_Text(pDX, IDC_ORT, m_ort);
	DDV_MaxChars(pDX, m_ort, 30);
	DDX_Text(pDX, IDC_PLZ, m_plz);
	DDV_MaxChars(pDX, m_plz, 5);
	DDX_Text(pDX, IDC_STRASSE, m_strasse);
	DDV_MaxChars(pDX, m_strasse, 30);
	DDX_Text(pDX, IDC_UNTERNEHMENSART1, m_unternehmensart1);
	DDV_MaxChars(pDX, m_unternehmensart1, 40);
	DDX_Text(pDX, IDC_UNTERNEHMENSART2, m_unternehmensart2);
	DDX_Text(pDX, IDC_VORNAME, m_vorname);
	DDV_MaxChars(pDX, m_vorname, 30);
	DDX_Radio(pDX, IDC_LAND1, m_land);
	DDX_Text(pDX, IDC_VAT1, m_vat1);
	DDV_MaxChars(pDX, m_vat1, 12);
	DDX_Text(pDX, IDC_VAT2, m_vat2);
	DDV_MaxChars(pDX, m_vat2, 12);
	DDX_Text(pDX, IDC_VAT3, m_vat3);
	DDV_MaxChars(pDX, m_vat3, 12);
	DDX_Text(pDX, IDC_VAT4, m_vat4);
	DDV_MaxChars(pDX, m_vat4, 12);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEinstellungen2, CPropertyPage)
	//{{AFX_MSG_MAP(CEinstellungen2)
	ON_BN_CLICKED(IDC_LAND1, OnLand1)
	ON_BN_CLICKED(IDC_LAND2, OnLand2)
	ON_BN_CLICKED(IDC_LAND3, OnLand3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen2 message handlers

void CEinstellungen2::OnLand1() 
{
	SetDlgItemText(IDC_VAT1, "19");	// D
	SetDlgItemText(IDC_VAT2, "7");
	SetDlgItemText(IDC_VAT3, "0");
	SetDlgItemText(IDC_VAT4, "0");
}

void CEinstellungen2::OnLand2() 
{
	SetDlgItemText(IDC_VAT1, "20"); // AT
	SetDlgItemText(IDC_VAT2, "13");
	SetDlgItemText(IDC_VAT3, "10");
	SetDlgItemText(IDC_VAT4, "0");
}

void CEinstellungen2::OnLand3() 
{
	SetDlgItemText(IDC_VAT1, "7,7"); // CH
	SetDlgItemText(IDC_VAT2, "2,5");
	SetDlgItemText(IDC_VAT3, "3,7");	
	SetDlgItemText(IDC_VAT4, "0");
}
