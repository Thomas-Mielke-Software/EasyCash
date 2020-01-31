// Einstellungen5.cpp : implementation file
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
#include "Einstellungen5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen5 property page

IMPLEMENT_DYNCREATE(CEinstellungen5, CPropertyPage)

CEinstellungen5::CEinstellungen5() : CPropertyPage(CEinstellungen5::IDD)
{
	//{{AFX_DATA_INIT(CEinstellungen5)
	m_art1 = _T("");
	m_art10 = _T("");
	m_art2 = _T("");
	m_art3 = _T("");
	m_art4 = _T("");
	m_art5 = _T("");
	m_art6 = _T("");
	m_art7 = _T("");
	m_art8 = _T("");
	m_art9 = _T("");
	m_satz1 = _T("");
	m_satz10 = _T("");
	m_satz2 = _T("");
	m_satz3 = _T("");
	m_satz4 = _T("");
	m_satz5 = _T("");
	m_satz6 = _T("");
	m_satz7 = _T("");
	m_satz8 = _T("");
	m_satz9 = _T("");
	m_ustsatz1 = _T("");
	m_ustsatz10 = _T("");
	m_ustsatz2 = _T("");
	m_ustsatz3 = _T("");
	m_ustsatz4 = _T("");
	m_ustsatz5 = _T("");
	m_ustsatz6 = _T("");
	m_ustsatz7 = _T("");
	m_ustsatz8 = _T("");
	m_ustsatz9 = _T("");
	//}}AFX_DATA_INIT

	m_part[0] = &m_art1;
	m_part[1] = &m_art2;
	m_part[2] = &m_art3;
	m_part[3] = &m_art4;
	m_part[4] = &m_art5;
	m_part[5] = &m_art6;
	m_part[6] = &m_art7;
	m_part[7] = &m_art8;
	m_part[8] = &m_art9;
	m_part[9] = &m_art10;

	m_psatz[0] = &m_satz1;
	m_psatz[1] = &m_satz2;
	m_psatz[2] = &m_satz3;
	m_psatz[3] = &m_satz4;
	m_psatz[4] = &m_satz5;
	m_psatz[5] = &m_satz6;
	m_psatz[6] = &m_satz7;
	m_psatz[7] = &m_satz8;
	m_psatz[8] = &m_satz9;
	m_psatz[9] = &m_satz10;

	m_pustsatz[0] = &m_ustsatz1;
	m_pustsatz[1] = &m_ustsatz2;
	m_pustsatz[2] = &m_ustsatz3;
	m_pustsatz[3] = &m_ustsatz4;
	m_pustsatz[4] = &m_ustsatz5;
	m_pustsatz[5] = &m_ustsatz6;
	m_pustsatz[6] = &m_ustsatz7;
	m_pustsatz[7] = &m_ustsatz8;
	m_pustsatz[8] = &m_ustsatz9;
	m_pustsatz[9] = &m_ustsatz10;

	m_privat_split_size = 10;
}

CEinstellungen5::~CEinstellungen5()
{
}

void CEinstellungen5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEinstellungen5)
	DDX_Text(pDX, IDC_ART1, m_art1);
	DDX_Text(pDX, IDC_ART10, m_art10);
	DDX_Text(pDX, IDC_ART2, m_art2);
	DDX_Text(pDX, IDC_ART3, m_art3);
	DDX_Text(pDX, IDC_ART4, m_art4);
	DDX_Text(pDX, IDC_ART5, m_art5);
	DDX_Text(pDX, IDC_ART6, m_art6);
	DDX_Text(pDX, IDC_ART7, m_art7);
	DDX_Text(pDX, IDC_ART8, m_art8);
	DDX_Text(pDX, IDC_ART9, m_art9);
	DDX_Text(pDX, IDC_SATZ1, m_satz1);
	DDX_Text(pDX, IDC_SATZ10, m_satz10);
	DDX_Text(pDX, IDC_SATZ2, m_satz2);
	DDX_Text(pDX, IDC_SATZ3, m_satz3);
	DDX_Text(pDX, IDC_SATZ4, m_satz4);
	DDX_Text(pDX, IDC_SATZ5, m_satz5);
	DDX_Text(pDX, IDC_SATZ6, m_satz6);
	DDX_Text(pDX, IDC_SATZ7, m_satz7);
	DDX_Text(pDX, IDC_SATZ8, m_satz8);
	DDX_Text(pDX, IDC_SATZ9, m_satz9);
	DDX_Text(pDX, IDC_USTSATZ1, m_ustsatz1);
	DDX_Text(pDX, IDC_USTSATZ10, m_ustsatz10);
	DDX_Text(pDX, IDC_USTSATZ2, m_ustsatz2);
	DDX_Text(pDX, IDC_USTSATZ3, m_ustsatz3);
	DDX_Text(pDX, IDC_USTSATZ4, m_ustsatz4);
	DDX_Text(pDX, IDC_USTSATZ5, m_ustsatz5);
	DDX_Text(pDX, IDC_USTSATZ6, m_ustsatz6);
	DDX_Text(pDX, IDC_USTSATZ7, m_ustsatz7);
	DDX_Text(pDX, IDC_USTSATZ8, m_ustsatz8);
	DDX_Text(pDX, IDC_USTSATZ9, m_ustsatz9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEinstellungen5, CPropertyPage)
	//{{AFX_MSG_MAP(CEinstellungen5)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen5 message handlers

BOOL CEinstellungen5::OnSetActive() 
{
	UpdateCombo(IDC_ART1);
	UpdateCombo(IDC_ART2);
	UpdateCombo(IDC_ART3);
	UpdateCombo(IDC_ART4);
	UpdateCombo(IDC_ART5);
	UpdateCombo(IDC_ART6);
	UpdateCombo(IDC_ART7);
	UpdateCombo(IDC_ART8);
	UpdateCombo(IDC_ART9);
	UpdateCombo(IDC_ART10);
	
	UpdateData(FALSE);
	
	return CPropertyPage::OnSetActive();
}

void CEinstellungen5::UpdateCombo(int ctrlid)
{
	int i;

	((CComboBox *)GetDlgItem(ctrlid))->ResetContent();

	for (i = 0; i < 100; i++)
	{
		if (!m_einstellungen1->EinnahmenRechnungsposten[i].IsEmpty())
			((CComboBox *)GetDlgItem(ctrlid))->AddString(m_einstellungen1->EinnahmenRechnungsposten[i]);
	}
}

BOOL CEinstellungen5::OnKillActive() 
{
	UpdateData();

	int_to_currency(currency_to_int(m_satz1.GetBuffer(20)), 4, m_satz1.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz2.GetBuffer(20)), 4, m_satz2.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz3.GetBuffer(20)), 4, m_satz3.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz4.GetBuffer(20)), 4, m_satz4.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz5.GetBuffer(20)), 4, m_satz5.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz6.GetBuffer(20)), 4, m_satz6.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz7.GetBuffer(20)), 4, m_satz7.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz8.GetBuffer(20)), 4, m_satz8.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz9.GetBuffer(20)), 4, m_satz9.GetBuffer(20));
	int_to_currency(currency_to_int(m_satz10.GetBuffer(20)), 4, m_satz10.GetBuffer(20));
	
	UpdateData(FALSE);

	return CPropertyPage::OnKillActive();
}
